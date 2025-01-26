#pragma once

#include "opencv2/opencv.hpp"
#include <condition_variable>
#include <memory>
#include <thread>

#define CAMERA_WIDTH 3840
#define CAMERA_HEIGHT 2160

class Camera {
  private:
    cv::VideoCapture capture_;

    std::queue<std::shared_ptr<cv::Mat>> frame_queue_;

    std::mutex frame_queue_mutex_;
    std::condition_variable frame_queue_cond_;

    bool is_running_;
    std::thread capture_thread_;
    std::function<void()> capture_thread_fn_;

  public:
    Camera();

    void start();
    void stop();

    std::shared_ptr<cv::Mat> get_frame();
};