//
// Created by kaylor on 3/6/24.
//

#include "Model.hpp"
#include "Sensor.hpp"
#include "RknnPool.hpp"
#include <future>
#include <iostream>

// 获取欧几里得距离
static float get_duclidean_distance(float * output1, float * output2)
{
    float sum = 0;
    for(int i = 0; i < 128; i++) {
        sum += (output1[i] - output2[i]) * (output1[i] - output2[i]);
    }
    float norm = std::sqrt(sum);
    return norm;
}

// 构造函数，初始化线程池和模型
FaceRknnPool::FaceRknnPool()
{
    try {
        // 配置线程池，使用指定数量的线程
        thread_pool_ = std::make_unique<ThreadPool>(thread_num_);

        // 每个线程加载一个模型
        for(int i = 0; i < this->thread_num_; ++i) {
            retinaface_models_.push_back(std::make_shared<Retinaface>()); // 使用模型路径初始化模型

            facenet_models_.push_back(std::make_shared<Facenet>()); // 使用模型路径初始化模型
        }

    } catch(const std::bad_alloc & e) {
        std::cout << "Out of memory: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    // 初始化每个模型
    for(int i = 0; i < this->thread_num_; ++i) {

        auto ret = retinaface_models_[i]->init(retinaface_models_[0]->get_rknn_context(), i != 0);
        if(ret != 0) {
            std::cout << "Init rknn model failed!" << std::endl;
            exit(EXIT_FAILURE);
        }

        ret = facenet_models_[i]->init(facenet_models_[0]->get_rknn_context(), i != 0);
        if(ret != 0) {
            std::cout << "Init rknn model failed!" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    this->retinaface_model_size_ = this->retinaface_models_[0]->get_model_width();
    this->facenet_model_size_    = this->facenet_models_[0]->get_model_width();
}

FaceRknnPool::~FaceRknnPool()
{}

// 向线程池添加推理任务
void FaceRknnPool::add_inference_task(std::shared_ptr<cv::Mat> src, ImageProcess & retinaface_image_process,
                                      bool is_generate_face_feature)
{

    // 将任务添加到线程池
    thread_pool_->enqueue(
        [&](std::shared_ptr<cv::Mat> original_img, bool is_generate_face_feature) { // 线程池执行的任务
            try {
                // 处理输入图像
                auto convert_img = retinaface_image_process.convert(*original_img);

                // 获取模型ID
                auto mode_id = get_model_id();

                // 创建一个新的图像来适配模型输入大小
                cv::Mat rgb_img =
                    cv::Mat::zeros(this->retinaface_models_[mode_id]->get_model_width(),
                                   this->retinaface_models_[mode_id]->get_model_height(), convert_img->type());

                // 将图像从BGR转换为RGB
                cv::cvtColor(*convert_img, rgb_img, cv::COLOR_BGR2RGB);

                retinaface_result results; // 存放推理结果
                // 使用模型进行推理
                this->retinaface_models_[mode_id]->inference(rgb_img.ptr(), &results,
                                                             retinaface_image_process.get_letter_box());

                // 是否是同一人脸
                bool is_check = false;

                if(results.count > 0 && is_face_recognition_) {
                    is_check = this->face_recognition(mode_id, *original_img, results, is_generate_face_feature);

                    uint64_t current_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                     std::chrono::system_clock::now().time_since_epoch())
                                                     .count();

                    if(this->pre_show_oled_timestamp_ == 0 ||
                       current_timestamp - this->pre_show_oled_timestamp_ > 500) {
                        this->pre_show_oled_timestamp_ = current_timestamp;

                        [[maybe_unused]] auto future = std::async(std::launch::async, [&]() { OLED::show(is_check); });
                    }
                } else if(results.count > 0 && is_generate_face_feature) {
                    is_check = this->face_recognition(mode_id, *original_img, results, is_generate_face_feature);
                    std::cout << "录入人脸成功" << std::endl;
                    return;
                }

                cv::Scalar recognition_color{0, 255, 0};
                cv::Scalar color{255, 255, 255};

                // 进行图像后处理
                retinaface_image_process.image_post_process(*original_img, results,
                                                            is_check ? recognition_color : color);

                // 锁住结果队列，将推理结果加入队列
                std::lock_guard<std::mutex> lock_guard(this->image_results_mutex_);
                this->image_results_.push(std::move(original_img));
                this->image_results_cv_.notify_one();
            } catch(std::exception & e) {
                std::cout << "FaceRknnPool---add_inference_task: " << e.what() << std::endl;
            }

        },
        std::move(src), is_generate_face_feature); // 向线程池添加任务
}

// 获取当前应该使用的模型ID
int FaceRknnPool::get_model_id()
{
    std::lock_guard<std::mutex> lock(id_mutex_); // 锁住ID生成过程
    int mode_id = id_;                           // 获取当前模型ID
    id_++;                                       // 自增ID
    if(id_ == thread_num_) {                     // 如果ID达到线程数限制，重置为0
        id_ = 0;
    }
    return mode_id; // 返回模型ID
}

// 从结果队列中获取图像结果
std::shared_ptr<cv::Mat> FaceRknnPool::get_image_result_from_queue()
{
    std::unique_lock<std::mutex> lock(this->image_results_mutex_);
    this->image_results_cv_.wait(lock, [this]() { return !this->image_results_.empty(); });

    // 否则，获取队列中的第一个图像并移除它
    auto res = this->image_results_.front();
    this->image_results_.pop();
    return std::move(res); // 返回结果图像
}

int FaceRknnPool::get_retinaface_model_size()
{
    return this->retinaface_model_size_;
}

int FaceRknnPool::get_facenet_model_size()
{
    return this->facenet_model_size_;
}

int FaceRknnPool::get_facenet_feature_vector_size()
{
    return this->facenet_feature_vector_.size();
}

void FaceRknnPool::clean_image_results()
{
    std::lock_guard<std::mutex> lock(this->image_results_mutex_);
    while(!this->image_results_.empty()) {
        this->image_results_.pop();
    }
}

bool FaceRknnPool::face_recognition(int mode_id, cv::Mat & image, retinaface_result & results,
                                    bool is_generate_face_feature)
{

    retinaface_object * detect_result = &(results.object[0]);
    cv::Mat crop_img                  = image(cv::Rect(detect_result->box.left, detect_result->box.top,
                                                       detect_result->box.right - detect_result->box.left,
                                                       detect_result->box.bottom - detect_result->box.top));

    auto facenet_image_process =
        std::make_unique<ImageProcess>(crop_img.cols, crop_img.rows, this->get_facenet_model_size());

    auto convert_img = facenet_image_process->convert(crop_img);

    cv::Mat rgb_img = cv::Mat::zeros(this->facenet_models_[mode_id]->get_model_width(),
                                     this->facenet_models_[mode_id]->get_model_height(), convert_img->type());

    // 将图像从BGR转换为RGB
    cv::cvtColor(*convert_img, rgb_img, cv::COLOR_BGR2RGB);

    std::vector<float> out_fp32(128);

    this->facenet_models_[mode_id]->inference(rgb_img.ptr(), out_fp32, facenet_image_process->get_letter_box());

    if(is_generate_face_feature) {
        this->facenet_feature_vector_.push_back(std::move(out_fp32));
        return false;
    } else {
        if(this->is_face_recognition_) {
            for(auto out : facenet_feature_vector_) {
                float distance = get_duclidean_distance(out.data(), out_fp32.data());
                if(distance < 0.6) {
                    return true;
                }
            }
        }
    }
    return false;
}

void FaceRknnPool::change_face_recognition_status(bool status)
{
    this->is_face_recognition_ = status;
}
