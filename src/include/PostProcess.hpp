#pragma once
#include <stdint.h>
#include <string>

#include "Common.hpp"
#include "rknn_api.h"

int retinaface_post_process(rknn_app_context_t *app_ctx, rknn_output *outputs,
                 letterbox_t *letter_box, retinaface_result *results);

const char *coco_cls_to_name(int cls_id);

int init_yolo_post_process(const char *label_path);

void deinit_yolo_post_process();

int yolo_post_process(rknn_app_context_t *app_ctx, rknn_output *outputs,
                 letterbox_t *letter_box, float conf_threshold,
                 float nms_threshold, yolo_result_list *results);
                      
