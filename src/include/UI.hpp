#pragma once

#include "Camera.hpp"
#include "FFmpeg.hpp"
#include "Lvgl.hpp"
#include "RknnPool.hpp"
#include "PageManager.hpp"

#include <atomic>
#include <cstdint>

#define ACCESS_CONTROL_PAGE_DELAY_TIME 7000
#define SECURITY_CAMERA_PAGE_AUTO_RECORD_DELAY_TIME 2

class MainPage : public BasePage {
  private:
    LvObject * main_screen;

  public:
    MainPage();
    void show() override;
    void hide() override;
};

// 加载门禁页面
class AccessControlPage : public BasePage {
  private:

    Camera & camera_;
    FaceRknnPool & face_rknn_pool_;
    ImageProcess & image_process_;
    LvTimer * timer;
    std::atomic_bool is_running = false;
    std::mutex get_frame_mutex_;

    LvObject * main_screen;
    LvObject * black_screen;

    std::atomic_bool is_black_screen = false;

    // 记录当前时间戳
    uint64_t pre_timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    std::function<void()> load_black_screen_fn;

  public:
    AccessControlPage(Camera & camera, FaceRknnPool & face_rknn_pool, ImageProcess & image_process);

    void show() override;
    void hide() override;

    void show_black_screen();
    void show_normal_screen();
};

// 加载安防摄像头页面
class SecurityCameraPage : public BasePage {
  private:

    Camera & camera_;
    ImageProcess & image_process_;
    SecurityRknnPool & security_rknn_pool_;
    FFmpeg & ffmpeg_;

    std::atomic_bool is_running_     = false;
    std::atomic_bool is_mp4_running_ = false;
    std::atomic_bool is_auto_record_ = false;
    std::atomic_bool is_auto_record_start_ = false;
    std::atomic_bool is_alarm_ = false;
    std::atomic_bool is_alarm_start_ = false;
    uint64_t auto_record_start_time_ = 0;

    std::mutex frame_mutex_;
    std::condition_variable frame_cv_;
    bool is_rtsp_turn_ = false; // true表示轮到RTSP线程取帧

    void inner_start_record();
    void inner_stop_record();

  public:
    SecurityCameraPage(Camera & camera, ImageProcess & image_process, SecurityRknnPool & security_rknn_pool,
                       FFmpeg & ffmpeg);
    void show() override;
    void hide() override;
};

// 加载文件列表页面
class FileListPage : public BasePage {
  private:

  public:
    FileListPage();
    void show() override;
    void hide() override;
};
