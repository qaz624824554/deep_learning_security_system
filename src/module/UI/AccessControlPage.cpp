#include "PageManager.hpp"
#include "Sensor.hpp"
#include "UI.hpp"
#include "Camera.hpp"
#include "Font.hpp"
#include "RknnPool.hpp"
#include "Lvgl.hpp"
#include <cstdint>
#include <exception>
#include <iostream>
#include <src/core/lv_obj.h>
#include <src/display/lv_display.h>
#include <src/misc/lv_area.h>
#include <src/misc/lv_async.h>
#include <src/misc/lv_event.h>
#include <thread>

extern "C" {
LV_IMAGE_DECLARE(bg);
}

static LvImageDsc * image_dsc_ = new LvImageDsc;
static LvImage * image_;
static LvLabel * face_num_label_;

AccessControlPage::AccessControlPage(Camera & camera, FaceRknnPool & face_rknn_pool, ImageProcess & image_process)
    : camera_(camera), face_rknn_pool_(face_rknn_pool), image_process_(image_process)
{
    main_screen  = new LvObject(nullptr);
    black_screen = new LvObject(nullptr);

    main_screen->set_style_bg_image_src(&bg, 0).set_style_text_font(Font16::get_font(), 0);
    black_screen->set_style_bg_color(lv_color_black(), 0);

    image_ = new LvImage(main_screen->raw());

    image_->align(LV_ALIGN_CENTER, 0, -50);

    LvButton back_button(main_screen->raw(), "返回");
    back_button.set_pos(10, 10).add_event_cb(
        [&](lv_event_t * e, void * data) { PageManager::getInstance().switchToPage(PageManager::PageType::MAIN_PAGE); },
        LV_EVENT_CLICKED, nullptr);

    LvObject switch_container(main_screen->raw());
    switch_container.set_size(LV_SIZE_CONTENT, LV_SIZE_CONTENT)
        .align(LV_ALIGN_BOTTOM_MID, 200, 0)
        .set_style_bg_opa(LV_OPA_TRANSP, 0)
        .set_style_border_width(0, 0)
        .set_flex_flow(LV_FLEX_FLOW_COLUMN)
        .set_flex_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    LvSwitch switch_button(switch_container.raw());
    switch_button.set_size(80, 40);

    LvLabel label(switch_container.raw(), "人脸检测", lv_color_white());
    switch_button.add_event_cb(
        [&](lv_event_t * e, void * data) {
            // 判断状态
            auto target = (lv_obj_t *)lv_event_get_target(e);
            if(lv_obj_has_state(target, LV_STATE_CHECKED)) {
                face_rknn_pool_.change_face_recognition_status(true);
            } else {
                face_rknn_pool_.change_face_recognition_status(false);
            }
        },
        LV_EVENT_VALUE_CHANGED, nullptr);

    LvObject face_num_container(main_screen->raw());
    face_num_container.set_size(LV_SIZE_CONTENT, LV_SIZE_CONTENT)
        .align(LV_ALIGN_BOTTOM_MID, -30, -33)
        .set_style_bg_opa(LV_OPA_TRANSP, 0)
        .set_style_border_width(0, 0)
        .set_flex_flow(LV_FLEX_FLOW_ROW)
        .set_style_text_font(Font24::get_font(), 0);

    LvLabel face_label(face_num_container.raw(), "已录入人脸数量:", lv_color_white());

    face_num_label_ = new LvLabel(face_num_container.raw(), "0", lv_color_white());

    LvButton record_face_button(main_screen->raw(), "录入人脸");
    record_face_button.align(LV_ALIGN_BOTTOM_MID, -200, -45);
    record_face_button.add_event_cb(
        [&](lv_event_t * e, void * data) {
            std::thread(
                [&](LvLabel * label) {
                    std::unique_lock<std::mutex> lock(get_frame_mutex_);

                    auto frame = camera_.get_frame();

                    lock.unlock();

                    face_rknn_pool_.add_inference_task(std::move(frame), image_process_, true);
                },
                face_num_label_)
                .detach();

            LvAsync::call([&]() {
                face_num_label_->set_text(
                    std::to_string(face_rknn_pool_.get_facenet_feature_vector_size() + 1).c_str());
            });
        },
        LV_EVENT_CLICKED, nullptr);

    timer = new LvTimer(
        [&](lv_timer_t * t, void * data) {

            std::shared_ptr<cv::Mat> frame_res_ptr = face_rknn_pool_.get_image_result_from_queue();

            if(frame_res_ptr) {

                cv::resize(*frame_res_ptr, *frame_res_ptr, cv::Size(800, 450));

                memset(image_dsc_, 0, sizeof(LvImageDsc));

                image_dsc_->raw()->data      = frame_res_ptr->data;
                image_dsc_->raw()->data_size = frame_res_ptr->total() * frame_res_ptr->elemSize();

                image_dsc_->raw()->header.w  = frame_res_ptr->cols;
                image_dsc_->raw()->header.h  = frame_res_ptr->rows;
                image_dsc_->raw()->header.cf = LV_COLOR_FORMAT_RGB888;

                image_->set_src(image_dsc_->raw());
            }

        },
        10, nullptr);

    timer->pause();

    load_black_screen_fn = [this]() { lv_screen_load(black_screen->raw()); };
}

void AccessControlPage::show()
{
    show_normal_screen();

    SR501::listen_state([&](bool has_person) {
        try {
            std::cout << "has_person: " << has_person << std::endl;

            // 如果当前有人，且屏幕处于黑屏状态，则记录当前时间戳，并显示正常屏幕
            // 如果当前有人，但屏幕处于显示状态，则更新当前时间戳
            // 如果当前没人，且屏幕处于显示状态，对比当前时间和之前记录的时间搓，如果超过2秒，则黑屏
            // 如果当前没人，且屏幕处于黑屏状态，不做任何操作
            if(has_person) {
                pre_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::system_clock::now().time_since_epoch())
                                    .count();
                if(is_black_screen) {
                    is_black_screen = false;
                    show_normal_screen();
                }
            } else {
                uint64_t current_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                 std::chrono::system_clock::now().time_since_epoch())
                                                 .count();
                if(!is_black_screen && current_timestamp - pre_timestamp > ACCESS_CONTROL_PAGE_DELAY_TIME) {
                    is_black_screen = true;
                    show_black_screen();
                }
            }
        } catch(std::exception & e) {
            std::cout << "AccessControlPage---show: " << e.what() << std::endl;
        }
    });
}

void AccessControlPage::hide()
{
    image_->add_flag(LV_OBJ_FLAG_HIDDEN);
    timer->pause();
    is_running = false;
    SR501::stop_listen_state();
}

void AccessControlPage::show_normal_screen()
{
    lv_screen_load(main_screen->raw());

    is_running = true;

    timer->resume();

    image_->remove_flag(LV_OBJ_FLAG_HIDDEN);

    std::thread([&]() {
        try {
            camera_.start();

            while(is_running) {
                std::unique_lock<std::mutex> lock(get_frame_mutex_);

                auto frame = camera_.get_frame();

                lock.unlock();

                face_rknn_pool_.add_inference_task(std::move(frame), image_process_);
            }

            face_rknn_pool_.clean_image_results();
            camera_.stop();

        } catch(std::exception & e) {
            std::cout << "AccessControlPage---show_normal_screen: " << e.what() << std::endl;
        }
    }).detach();
}

void AccessControlPage::show_black_screen()
{

    timer->pause();
    is_running = false;

    lv_async_call(
        [](void * data) {
            auto cb = static_cast<std::function<void()> *>(data);
            (*cb)();
        },
        &load_black_screen_fn);
}
