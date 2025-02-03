#include "Camera.hpp"
#include <iostream>
#include <thread>

Camera::Camera() : is_running_(false)
{
    capture_.open(11, cv::CAP_V4L2);

    capture_.set(cv::CAP_PROP_FPS, 30);
    capture_.set(cv::CAP_PROP_FRAME_WIDTH, CAMERA_WIDTH);
    capture_.set(cv::CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT);

    capture_thread_fn_ = [this]() {
        try {
            while(is_running_) {
                std::unique_lock<std::mutex> lock(frame_queue_mutex_);
                frame_queue_cond_.wait(lock, [this]() { return frame_queue_.empty() && is_running_; });

                if(!is_running_) {
                    break;
                }

                std::unique_ptr<cv::Mat> frame = std::make_unique<cv::Mat>();
                capture_ >> *frame;

                if(frame->empty()) {
                    break;
                }

                frame_queue_.push(std::move(frame));
                frame_queue_cond_.notify_one();
            }
        } catch(std::exception & e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    };
}

void Camera::start()
{
    is_running_     = true;
    capture_thread_ = std::thread(capture_thread_fn_);
}

void Camera::stop()
{
    is_running_ = false;
    capture_thread_.join();
    while(!frame_queue_.empty()) {
        frame_queue_.pop();
    }
}

std::shared_ptr<cv::Mat> Camera::get_frame()
{
    std::unique_lock<std::mutex> lock(frame_queue_mutex_);
    frame_queue_cond_.wait(lock, [this]() { return !frame_queue_.empty(); });
    auto frame = frame_queue_.front();
    frame_queue_.pop();

    frame_queue_cond_.notify_one();

    return std::move(frame);
}