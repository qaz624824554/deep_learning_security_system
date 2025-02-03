#pragma once

#include "rknn_api.h"
#include <opencv2/core/mat.hpp>

#define OBJ_NAME_MAX_SIZE 64
#define OBJ_NUMB_MAX_SIZE 128
#define OBJ_CLASS_NUM 80
#define NMS_THRESH 0.8
#define BOX_THRESH 0.5
#define PROTO_CHANNEL (32)
#define PROTO_HEIGHT (160)
#define PROTO_WEIGHT (160)

/**
 * @brief LetterBox
 *
 */
typedef struct {
  int x_pad;
  int y_pad;
  float scale;
} letterbox_t;

/**
 * @brief Image rectangle
 *
 */

typedef struct box_rect_t {
  int left;   ///< Most left coordinate
  int top;    ///< Most top coordinate
  int right;  ///< Most right coordinate
  int bottom; ///< Most bottom coordinate
} box_rect_t;

typedef struct ponit_t {
  int x;
  int y;
} ponit_t;

typedef struct {
  int cls;
  box_rect_t box;
  float score;
  ponit_t ponit[5];
} retinaface_object;

typedef struct {
  int id;
  int count;
  retinaface_object object[OBJ_NUMB_MAX_SIZE];
} retinaface_result;

typedef struct {
  rknn_context rknn_ctx;
  rknn_input_output_num io_num;
  rknn_tensor_attr *input_attrs;
  rknn_tensor_attr *output_attrs;
  int model_channel;
  int model_width;
  int model_height;
  bool is_quant;
} rknn_app_context_t;

typedef struct {
  box_rect_t box;
  float prop;
  int cls_id;
} yolo_result;

typedef struct {
  int id;
  int count;
  yolo_result results[OBJ_NUMB_MAX_SIZE];
} yolo_result_list;
