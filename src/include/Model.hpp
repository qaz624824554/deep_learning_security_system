#pragma once

#include "Common.hpp"
#include "rknn_api.h"
#include <vector>
#include <memory>
#include <mutex>

#define RETINA_FACE_MODEL_PATH "src/assets/model/retina_face.rknn"
#define FACENET_MODEL_PATH "src/assets/model/facenet.rknn"
#define YOLO11_MODEL_PATH "src/assets/model/yolo11s.rknn"
#define YOLO11_LABEL_PATH "src/assets/model/coco_80_labels_list.txt"

#define NPU_CORE_NUM 3

class Facenet {
  public:
    Facenet();
    ~Facenet();
    int inference(void * image_buf, std::vector<float> & out_fp32, letterbox_t letter_box);
    rknn_context * get_rknn_context();
    int init(rknn_context * ctx_in, bool is_copy);
    int deinit();
    int get_model_width();  // 获取模型宽度
    int get_model_height(); // 获取模型高度

  private:
    rknn_app_context_t app_ctx_;
    rknn_context ctx_{0};
    std::unique_ptr<rknn_input[]> inputs_;
    std::unique_ptr<rknn_output[]> outputs_;
    std::mutex outputs_lock_;
};

class Retinaface {
  public:
    Retinaface();
    ~Retinaface();
    int inference(void * image_buf, retinaface_result * results, letterbox_t letter_box);
    rknn_context * get_rknn_context();
    int init(rknn_context * ctx_in, bool is_copy);
    int deinit();
    int get_model_width();  // 获取模型宽度
    int get_model_height(); // 获取模型高度

  private:
    rknn_app_context_t app_ctx_;
    rknn_context ctx_{0};
    std::unique_ptr<rknn_input[]> inputs_;
    std::unique_ptr<rknn_output[]> outputs_;
    std::mutex outputs_lock_;
};

class Yolo11 {
 public:
  Yolo11();
  ~Yolo11();
  int inference(void *image_buf, yolo_result_list *results,
                letterbox_t letter_box);
  rknn_context *get_rknn_context();
  int init(rknn_context *ctx_in, bool is_copy);
  int deinit();
  int get_model_width(); // 获取模型宽度
  int get_model_height(); // 获取模型高度

 private:
  rknn_app_context_t app_ctx_;
  rknn_context ctx_{0};
  std::unique_ptr<rknn_input[]> inputs_;
  std::unique_ptr<rknn_output[]> outputs_;
  std::mutex outputs_lock_;
};