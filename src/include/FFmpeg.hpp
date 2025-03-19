#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <opencv2/opencv.hpp>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/frame.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
}

#define RTSP_URL "rtsp://localhost:8554/live/stream"
#define MP4_DIR_PATH "/root/nfs_folder/record/"

class FFmpeg {
  private:
    AVFormatContext * rtsp_out_fmt_ctx_ = nullptr;
    AVFormatContext * mp4_out_fmt_ctx_  = nullptr;
    AVStream * rtsp_out_stream_         = nullptr;
    AVStream * mp4_out_stream_          = nullptr;
    int64_t last_pts_                   = AV_NOPTS_VALUE;
    const AVCodec * rk_encodec_         = nullptr;
    AVCodecContext * rk_encodec_ctx_    = nullptr;
    int origin_rtsp_pts_                = 0;
    int origin_mp4_pts_                 = 0;
    std::mutex frame_mutex_;
    std::condition_variable frame_cond_;
    std::queue<std::shared_ptr<cv::Mat>> frame_queue_;

    int ret_;
    AVPacket * packet_   = av_packet_alloc();
    AVFrame * frame_     = av_frame_alloc();
    AVPacket * hevc_pkt_ = av_packet_alloc();

    std::atomic_bool is_process_frame_ = false;
    std::atomic_bool is_mp4_recording_  = false;

    void init_encodec();
    void init_frame();

    std::string get_mp4_path();

  public:
    FFmpeg();
    ~FFmpeg();

    void start_process_frame();
    void stop_process_frame();
    void start_record(std::string path = "");
    void stop_record();
    void push_frame(std::shared_ptr<cv::Mat> frame);
};
