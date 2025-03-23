#include "Font.hpp"
#include "Lvgl.hpp"
#include "UI.hpp"
#include "Util.hpp"
#include <iostream>
#include <mutex>
#include <src/core/lv_obj.h>
#include <src/layouts/flex/lv_flex.h>
#include <src/misc/lv_area.h>
#include <src/misc/lv_color.h>
#include <src/misc/lv_palette.h>
#include <thread>

extern "C" {
LV_IMAGE_DECLARE(bg);
}

static LvObject * main_screen;
static LvImageDsc * image_dsc_ = new LvImageDsc;
static LvLabel * alarm_label;
static LvImage * image_;
static LvTimer * timer;
static LvObject * record_icon;
static LvAnimation * record_icon_anim;

static void anim_opacity_cb_(void * var, int32_t value)
{
    lv_obj_set_style_bg_opa((lv_obj_t *)var, value, 0);
}

SecurityCameraPage::SecurityCameraPage(Camera & camera, ImageProcess & image_process,
                                       SecurityRknnPool & security_rknn_pool, FFmpeg & ffmpeg)
    : camera_(camera), image_process_(image_process), security_rknn_pool_(security_rknn_pool), ffmpeg_(ffmpeg)
{
    main_screen = new LvObject(nullptr);
    main_screen->set_style_bg_image_src(&bg, 0).set_style_text_font(Font16::get_font(), 0);

    image_ = new LvImage(main_screen->raw());
    image_->align(LV_ALIGN_CENTER, 0, -50);

    alarm_label = new LvLabel(main_screen->raw(), "检测到人体！", lv_palette_main(LV_PALETTE_RED));
    alarm_label->align(LV_ALIGN_CENTER, 0, 100).set_style_text_font(Font24::get_font(), 0).add_flag(LV_OBJ_FLAG_HIDDEN);

    record_icon = new LvObject(image_->raw());

    record_icon->add_flag(LV_OBJ_FLAG_HIDDEN)
        .align(LV_ALIGN_TOP_RIGHT, -10, 10)
        .set_size(100, 40)
        .set_flex_flow(LV_FLEX_FLOW_ROW)
        .set_flex_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER)
        .set_style_border_color(lv_color_hex(0xff0000), 0)
        .set_style_bg_opa(LV_OPA_30, 0)
        .remove_flag(LV_OBJ_FLAG_SCROLLABLE);

    LvObject record_icon_dot(record_icon->raw());

    record_icon_dot.set_size(20, 20)
        .set_style_radius(20, 0)
        .set_style_border_width(0, 0)
        .set_style_bg_color(lv_color_hex(0xff0000), 0)
        .remove_flag(LV_OBJ_FLAG_SCROLLABLE);

    LvLabel record_icon_label(record_icon->raw(), "REC", lv_color_black());

    record_icon_anim = new LvAnimation();
    record_icon_anim->set_var(record_icon_dot.raw())
        .set_exec_cb(anim_opacity_cb_)
        .set_duration(1000)
        .set_delay(0)
        .set_values(0, 255)
        .set_path_cb(lv_anim_path_ease_in_out)
        .set_repeat_count(LV_ANIM_REPEAT_INFINITE);

    LvButton back_button(main_screen->raw(), "返回");
    back_button.set_pos(10, 10).add_event_cb(
        [&](lv_event_t * e, void * data) { PageManager::getInstance().switchToPage(PageManager::PageType::MAIN_PAGE); },
        LV_EVENT_CLICKED, nullptr);

    LvObject record_container(main_screen->raw());
    record_container.set_size(LV_SIZE_CONTENT, LV_SIZE_CONTENT)
        .align(LV_ALIGN_BOTTOM_MID, -250, 0)
        .set_style_bg_opa(LV_OPA_TRANSP, 0)
        .set_style_border_width(0, 0)
        .set_flex_flow(LV_FLEX_FLOW_COLUMN)
        .set_flex_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    LvSwitch record_switch(record_container.raw());
    record_switch.set_size(80, 40).add_event_cb(
        [&](lv_event_t * e, void * data) {
            auto target = (lv_obj_t *)lv_event_get_target(e);
            if(lv_obj_has_state(target, LV_STATE_CHECKED)) {
                this->inner_start_record();
            } else {
                this->inner_stop_record();
            }
        },
        LV_EVENT_VALUE_CHANGED, nullptr);
    LvLabel label(record_container.raw(), "录像", lv_color_white());

    LvObject auto_record_container(main_screen->raw());
    auto_record_container.set_size(LV_SIZE_CONTENT, LV_SIZE_CONTENT)
        .align(LV_ALIGN_BOTTOM_MID, 0, 0)
        .set_style_bg_opa(LV_OPA_TRANSP, 0)
        .set_style_border_width(0, 0)
        .set_flex_flow(LV_FLEX_FLOW_COLUMN)
        .set_flex_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    LvSwitch auto_record_switch(auto_record_container.raw());
    auto_record_switch.set_size(80, 40).add_event_cb(
        [&](lv_event_t * e, void * data) {
            auto target = (lv_obj_t *)lv_event_get_target(e);
            if(lv_obj_has_state(target, LV_STATE_CHECKED)) {
                is_auto_record_ = true;
            } else {
                is_auto_record_ = false;
            }
        },
        LV_EVENT_VALUE_CHANGED, nullptr);
    LvLabel auto_record_label(auto_record_container.raw(), "检测到人时自动录像", lv_color_white());

    LvObject alarm_container(main_screen->raw());
    alarm_container.set_size(LV_SIZE_CONTENT, LV_SIZE_CONTENT)
        .align(LV_ALIGN_BOTTOM_MID, 250, 0)
        .set_style_bg_opa(LV_OPA_TRANSP, 0)
        .set_style_border_width(0, 0)
        .set_flex_flow(LV_FLEX_FLOW_COLUMN)
        .set_flex_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    LvSwitch alarm_switch(alarm_container.raw());
    alarm_switch.set_size(80, 40).add_event_cb(
        [&](lv_event_t * e, void * data) {
            auto target = (lv_obj_t *)lv_event_get_target(e);
            if(lv_obj_has_state(target, LV_STATE_CHECKED)) {
                is_alarm_ = true;
            } else {
                is_alarm_ = false;
            }
        },
        LV_EVENT_VALUE_CHANGED, nullptr);
    LvLabel alarm_label(alarm_container.raw(), "检测到有人时报警", lv_color_white());

    timer = new LvTimer(
        [&](lv_timer_t * t, void * data) {
            std::unique_lock<std::mutex> lock(frame_mutex_);
            frame_cv_.wait(lock, [this]() { return !is_rtsp_turn_ || !is_running_; });

            if(!is_running_) {
                return;
            }

            auto image_result = security_rknn_pool_.get_image_result_from_queue();

            is_rtsp_turn_ = true;
            frame_cv_.notify_one();

            if(image_result) {

                cv::resize(*image_result, *image_result, cv::Size(800, 450));

                memset(image_dsc_, 0, sizeof(LvImageDsc));

                image_dsc_->raw()->data      = image_result->data;
                image_dsc_->raw()->data_size = image_result->total() * image_result->elemSize();
                image_dsc_->raw()->header.w  = image_result->cols;
                image_dsc_->raw()->header.h  = image_result->rows;
                image_dsc_->raw()->header.cf = LV_COLOR_FORMAT_RGB888;

                image_->set_src(image_dsc_->raw());
            }
        },
        10, nullptr);

    timer->pause();
}

void SecurityCameraPage::show()
{
    lv_screen_load(main_screen->raw());

    is_running_ = true;

    timer->resume();

    ffmpeg_.start_process_frame();

    std::thread([this]() {
        try {
            camera_.start();
            while(is_running_) {
                auto frame = camera_.get_frame();
                security_rknn_pool_.add_inference_task(std::move(frame), image_process_);
                {
                    std::unique_lock<std::mutex> lock(frame_mutex_);
                    // 等待直到轮到RTSP线程取帧
                    frame_cv_.wait(lock, [this]() { return is_rtsp_turn_; });
                    auto image_result = security_rknn_pool_.get_image_result_from_queue(true);

                    is_rtsp_turn_ = false;
                    frame_cv_.notify_one();

                    if(image_result) {
                        ffmpeg_.push_frame(std::move(image_result));

                        // 如果自动录像开启，并且检测到人，并且is_auto_record_start为false，记录is_auto_record_start_time，开始录像
                        // 如果自动录像开启，并且检测到人，并且is_auto_record_start为true，记录is_auto_record_start_time，继续录像
                        // 如果自动录像开启，没有检测到人，并且is_auto_record_start为true，且is_auto_record_start_time超过当前时间3秒，则停止录像
                        if(is_auto_record_) {
                            if(security_rknn_pool_.is_person) {
                                if(!is_auto_record_start_) {
                                    is_auto_record_start_   = true;
                                    auto_record_start_time_ = std::time(nullptr);
                                    this->inner_start_record();
                                    std::cout << "开始录像" << std::endl;
                                } else {
                                    auto_record_start_time_ = std::time(nullptr);
                                }
                            } else {
                                if(is_auto_record_start_ && std::time(nullptr) - auto_record_start_time_ >
                                                                SECURITY_CAMERA_PAGE_AUTO_RECORD_DELAY_TIME) {
                                    is_auto_record_start_ = false;
                                    this->inner_stop_record();

                                    std::cout << "停止录像1" << std::endl;
                                }
                            }
                        } else {
                            if(is_auto_record_start_) {
                                is_auto_record_start_ = false;
                                this->inner_stop_record();

                                std::cout << "停止录像2" << std::endl;
                            }
                        }

                        if(is_alarm_ && security_rknn_pool_.is_person && !is_alarm_start_) {
                            is_alarm_start_ = true;
                            std::thread([this]() {
                                lv_async_call([](void *) { alarm_label->remove_flag(LV_OBJ_FLAG_HIDDEN); }, nullptr);
                                // execute_command("aplay -D bluealsa:DEV=D0:6A:81:16:56:27,PROFILE=a2dp /root/alert.wav");
                                std::this_thread::sleep_for(std::chrono::seconds(2));
                                is_alarm_start_ = false;
                                lv_async_call([](void *) { alarm_label->add_flag(LV_OBJ_FLAG_HIDDEN); }, nullptr);
                            }).detach();
                        }
                    }
                }
            }
            camera_.stop();
        } catch(std::exception & e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }).detach();
}

void SecurityCameraPage::hide()
{
    ffmpeg_.stop_process_frame();

    is_running_ = false;

    timer->pause();
}

void SecurityCameraPage::inner_start_record()
{
    ffmpeg_.start_record();

    lv_async_call(
        [](void *) {
            record_icon->remove_flag(LV_OBJ_FLAG_HIDDEN);
            record_icon_anim->start();
        },
        nullptr);
}

void SecurityCameraPage::inner_stop_record()
{
    ffmpeg_.stop_record();

    lv_async_call(
        [](void *) {
            record_icon->add_flag(LV_OBJ_FLAG_HIDDEN);
            record_icon_anim->stop();
        },
        nullptr);
}
