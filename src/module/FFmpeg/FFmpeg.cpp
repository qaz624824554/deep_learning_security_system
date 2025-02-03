#include "FFmpeg.hpp"
#include <iostream>
#include <libavformat/avformat.h>
#include <thread>

FFmpeg::FFmpeg()
{
    avformat_network_init();

    init_encodec();
    init_frame();
}

FFmpeg::~FFmpeg()
{
    avformat_free_context(rtsp_out_fmt_ctx_);
    avcodec_free_context(&rk_encodec_ctx_);
    av_packet_free(&packet_);
    av_frame_free(&frame_);
    av_packet_free(&hevc_pkt_);
}

std::string FFmpeg::get_mp4_path()
{
    return "/root/record/record_" + std::to_string(time(nullptr)) + ".mp4";
}

void FFmpeg::init_encodec()
{
    rk_encodec_     = avcodec_find_encoder_by_name("h264_rkmpp");
    rk_encodec_ctx_ = avcodec_alloc_context3(rk_encodec_);

    AVDictionary * rk_encodec_opts = nullptr;

    av_dict_set_int(&rk_encodec_opts, "qp_init", 25, 0);
    // av_dict_set(&rk_encodec_opts, "rc_mode", "VBR", 0);
    av_dict_set(&rk_encodec_opts, "profile", "high", 0);
    av_dict_set(&rk_encodec_opts, "level", "5.2", 0);

    rk_encodec_ctx_->width     = 3840;
    rk_encodec_ctx_->height    = 2160;
    rk_encodec_ctx_->pix_fmt   = AV_PIX_FMT_BGR24;
    rk_encodec_ctx_->time_base = (AVRational){1, 30};
    rk_encodec_ctx_->gop_size  = 30;
    rk_encodec_ctx_->max_b_frames = 1;
    // rk_encodec_ctx_->bit_rate  = 1024 * 2;

    if(avcodec_open2(rk_encodec_ctx_, rk_encodec_, &rk_encodec_opts) < 0) {
        throw std::runtime_error("avcodec_open2 failed");
    }
}

void FFmpeg::init_frame()
{
    frame_->width  = rk_encodec_ctx_->width;
    frame_->height = rk_encodec_ctx_->height;
    frame_->format = AV_PIX_FMT_BGR24;

    if(av_frame_get_buffer(frame_, 0) < 0) {
        throw std::runtime_error("av_frame_get_buffer failed");
    }

    if(av_frame_make_writable(frame_) < 0) {
        throw std::runtime_error("av_frame_make_writable failed");
    }
}

void FFmpeg::push_frame(std::shared_ptr<cv::Mat> opencv_frame)
{
    std::lock_guard<std::mutex> lock(frame_mutex_);
    frame_queue_.push(std::move(opencv_frame));
    frame_cond_.notify_one();
}

void FFmpeg::start_process_frame()
{
    is_process_frame_ = true;

    try {
        avformat_alloc_output_context2(&rtsp_out_fmt_ctx_, nullptr, "rtsp", RTSP_URL);
        if(!rtsp_out_fmt_ctx_) {
            throw std::runtime_error("avformat_alloc_output_context2 failed");
        }

        rtsp_out_stream_ = avformat_new_stream(rtsp_out_fmt_ctx_, nullptr);
        if(!rtsp_out_stream_) {
            throw std::runtime_error("avformat_new_stream failed");
        }

        if(avcodec_parameters_from_context(rtsp_out_stream_->codecpar, rk_encodec_ctx_) < 0) {
            throw std::runtime_error("Failed to copy codec parameters to output stream");
        }

        AVDictionary * rtsp_opts = nullptr;
        av_dict_set(&rtsp_opts, "rtsp_transport", "tcp", 0);

        // 写入文件头部信息
        if(avformat_write_header(rtsp_out_fmt_ctx_, &rtsp_opts) < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret_, errbuf, sizeof(errbuf));
            throw std::runtime_error(errbuf);
        }

        av_dump_format(rtsp_out_fmt_ctx_, 0, RTSP_URL, 1);

        std::cout << "init_rtsp success" << std::endl;
    } catch(std::exception & e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // 启用线程处理帧
    std::thread([this]() {
        try {
            while(is_process_frame_) {
                std::unique_lock<std::mutex> lock(frame_mutex_);
                frame_cond_.wait(lock, [this]() { return !frame_queue_.empty(); });

                auto opencv_frame = frame_queue_.front();
                frame_queue_.pop();

                if (!opencv_frame) {
                    std::cout << "opencv_frame is null" << std::endl;
                    continue;
                }

                if(av_image_fill_arrays(frame_->data, frame_->linesize, opencv_frame->data, AV_PIX_FMT_BGR24,
                                        opencv_frame->cols, opencv_frame->rows, 1) < 0) {
                    throw std::runtime_error("av_image_fill_arrays failed");
                }

                frame_->pts = origin_rtsp_pts_++;

                ret_ = avcodec_send_frame(rk_encodec_ctx_, frame_);
                if(ret_ < 0) {
                    throw std::runtime_error("Error sending a frame for encoding");
                }

                while(ret_ >= 0) {
                    ret_ = avcodec_receive_packet(rk_encodec_ctx_, hevc_pkt_);
                    if(ret_ == AVERROR(EAGAIN) || ret_ == AVERROR_EOF)
                        break;
                    else if(ret_ < 0) {
                        throw std::runtime_error("Error during encoding");
                    }

                    if(is_mp4_recording_) {
                        auto cloned_packet = av_packet_clone(hevc_pkt_);
                        if(!cloned_packet) {
                            throw std::runtime_error("av_packet_clone failed");
                        }

                        cloned_packet->pts = origin_mp4_pts_;
                        cloned_packet->dts = origin_mp4_pts_++;

                        cloned_packet->stream_index = mp4_out_stream_->index;

                        cloned_packet->pts =
                            av_rescale_q_rnd(cloned_packet->pts, rk_encodec_ctx_->time_base, mp4_out_stream_->time_base,
                                             AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                        cloned_packet->dts =
                            av_rescale_q_rnd(cloned_packet->dts, rk_encodec_ctx_->time_base, mp4_out_stream_->time_base,
                                             AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

                        // std::cout << "cloned_packet->pts: " << cloned_packet->pts
                        //           << ", cloned_packet->dts: " << cloned_packet->dts << std::endl;

                        ret_ = av_interleaved_write_frame(mp4_out_fmt_ctx_, cloned_packet);
                        if(ret_ < 0) {
                            char errbuf[AV_ERROR_MAX_STRING_SIZE];
                            av_strerror(ret_, errbuf, sizeof(errbuf));
                            std::cerr << "Error writing hevc_pkt_ to mp4: " << errbuf << std::endl;
                        }

                        av_packet_unref(cloned_packet);
                    }

                    hevc_pkt_->stream_index = rtsp_out_stream_->index;

                    // 将编码器输出的数据包时间戳值重新调整为流的时间基准。
                    hevc_pkt_->pts =
                        av_rescale_q_rnd(hevc_pkt_->pts, rk_encodec_ctx_->time_base, rtsp_out_stream_->time_base,
                                         AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                    hevc_pkt_->dts =
                        av_rescale_q_rnd(hevc_pkt_->dts, rk_encodec_ctx_->time_base, rtsp_out_stream_->time_base,
                                         AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

                    // std::cout << "hevc_pkt->pts: " << hevc_pkt_->pts << ", hevc_pkt->dts: " << hevc_pkt_->dts
                    //           << std::endl;

                    ret_ = av_interleaved_write_frame(rtsp_out_fmt_ctx_, hevc_pkt_);

                    if(ret_ < 0) {
                        char errbuf[AV_ERROR_MAX_STRING_SIZE];
                        av_strerror(ret_, errbuf, sizeof(errbuf));
                        std::cerr << "Error writing hevc_pkt_ to rtsp: " << errbuf << std::endl;
                        break;
                    }

                    av_packet_unref(hevc_pkt_);
                }
            }

            if(av_write_trailer(rtsp_out_fmt_ctx_) < 0) {
                throw std::runtime_error("av_write_trailer failed");
            }

            avformat_free_context(rtsp_out_fmt_ctx_);

        } catch(std::exception & e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }).detach();
}

void FFmpeg::stop_process_frame()
{
    is_process_frame_ = false;
}

void FFmpeg::start_record(std::string path)
{
    try {

        if(avformat_alloc_output_context2(&mp4_out_fmt_ctx_, nullptr, "mp4", path.empty() ? get_mp4_path().c_str() : path.c_str()) < 0) {
            throw std::runtime_error("avformat_alloc_output_context2 failed");
        }

        mp4_out_stream_ = avformat_new_stream(mp4_out_fmt_ctx_, nullptr);
        if(!mp4_out_stream_) {
            throw std::runtime_error("avformat_new_stream failed");
        }

        mp4_out_stream_->start_time = 0;

        if(avcodec_parameters_from_context(mp4_out_stream_->codecpar, rk_encodec_ctx_) < 0) {
            throw std::runtime_error("Failed to copy codec parameters to output stream");
        }

        if(!(mp4_out_fmt_ctx_->oformat->flags & AVFMT_NOFILE)) {
            std::cout << "mp4_out_fmt_ctx_->url: " << mp4_out_fmt_ctx_->url << std::endl;
            if((ret_ = avio_open2(&mp4_out_fmt_ctx_->pb, mp4_out_fmt_ctx_->url, AVIO_FLAG_WRITE, nullptr, nullptr)) <
               0) {
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret_, errbuf, sizeof(errbuf));
                throw std::runtime_error(errbuf);
            }
        }

        if(avformat_write_header(mp4_out_fmt_ctx_, nullptr) < 0) {
            throw std::runtime_error("avformat_write_header failed");
        }

        std::cout << "init_mp4 success" << std::endl;

        origin_mp4_pts_ = 0;

        is_mp4_recording_ = true;
    } catch(std::exception & e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void FFmpeg::stop_record()
{
    if(mp4_out_fmt_ctx_ && av_write_trailer(mp4_out_fmt_ctx_) < 0) {
        throw std::runtime_error("av_write_trailer failed");
    }

    is_mp4_recording_ = false;

    if(mp4_out_fmt_ctx_) {
        avformat_free_context(mp4_out_fmt_ctx_);
    }
}
