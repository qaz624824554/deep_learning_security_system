#pragma once

#include "ImageProcess.hpp"
#include "ThreadPool.hpp"
#include "Model.hpp"
#include <cstdint>
#include <memory>
#include <opencv2/opencv.hpp>

#define RKNN_POOL_SIZE 10

class FaceRknnPool {
private:
  int thread_num_{RKNN_POOL_SIZE};
  std::unique_ptr<ThreadPool> thread_pool_;
  std::queue<std::shared_ptr<cv::Mat>> image_results_;
  std::vector<std::shared_ptr<Retinaface>> retinaface_models_;
  std::vector<std::shared_ptr<Facenet>> facenet_models_;
  std::mutex id_mutex_;
  std::mutex image_results_mutex_;
  std::condition_variable image_results_cv_;
  int retinaface_model_size_;
  int facenet_model_size_;
  uint32_t id_{0};

  std::atomic_bool is_face_recognition_{false};

  std::vector<std::vector<float>> facenet_feature_vector_;

  bool face_recognition(int mode_id, cv::Mat &image, retinaface_result &results, bool is_generate_face_feature = false);

  uint64_t pre_show_oled_timestamp_{0};

public:
  FaceRknnPool();
  ~FaceRknnPool();
  void add_inference_task(std::shared_ptr<cv::Mat> src,
                          ImageProcess &retinaface_image_process, bool is_generate_face_feature = false);
  std::shared_ptr<cv::Mat> get_image_result_from_queue();
  int get_model_id();
  int get_retinaface_model_size();
  int get_facenet_model_size();
  int get_facenet_feature_vector_size();
  void clean_image_results();
  void change_face_recognition_status(bool status);
};
