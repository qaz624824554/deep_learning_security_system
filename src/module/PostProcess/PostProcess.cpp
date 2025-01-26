//
// Created by kaylor on 3/5/24.
//

#include "PostProcess.hpp"
#include "rknn_box_priors.hpp"

#include "Float16.h"
#include "filesystem"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "rknn_matmul_api.h"
#include <iostream>
#include <set>

// 非极大值抑制阈值
#define NMS_THRESHOLD 0.4
// 置信度阈值
#define CONF_THRESHOLD 0.5
// 视觉阈值
#define VIS_THRESHOLD 0.4

#define WIDTH 3840
#define HEIGHT 2160

static int clamp(int x, int min, int max) {
  if (x > max)
    return max;
  if (x < min)
    return min;
  return x;
}

// 计算重叠区域
static float CalculateOverlap(float xmin0, float ymin0, float xmax0,
                              float ymax0, float xmin1, float ymin1,
                              float xmax1, float ymax1) {
  float w = fmax(0.f, fmin(xmax0, xmax1) - fmax(xmin0, xmin1) + 1);
  float h = fmax(0.f, fmin(ymax0, ymax1) - fmax(ymin0, ymin1) + 1);
  float i = w * h;
  float u = (xmax0 - xmin0 + 1) * (ymax0 - ymin0 + 1) +
            (xmax1 - xmin1 + 1) * (ymax1 - ymin1 + 1) - i;
  return u <= 0.f ? 0.f : (i / u);
}

// 非极大值抑制
static int nms(int validCount, float *outputLocations, int order[],
               float threshold) {
  for (int i = 0; i < validCount; ++i) {
    if (order[i] == -1) {
      continue;
    }
    int n = order[i];
    for (int j = i + 1; j < validCount; ++j) {
      int m = order[j];
      if (m == -1) {
        continue;
      }
      float xmin0 = outputLocations[n * 4 + 0] * WIDTH;
      float ymin0 = outputLocations[n * 4 + 1] * HEIGHT;
      float xmax0 = outputLocations[n * 4 + 2] * WIDTH;
      float ymax0 = outputLocations[n * 4 + 3] * HEIGHT;

      float xmin1 = outputLocations[m * 4 + 0] * WIDTH;
      float ymin1 = outputLocations[m * 4 + 1] * HEIGHT;
      float xmax1 = outputLocations[m * 4 + 2] * WIDTH;
      float ymax1 = outputLocations[m * 4 + 3] * HEIGHT;

      float iou = CalculateOverlap(xmin0, ymin0, xmax0, ymax0, xmin1, ymin1,
                                   xmax1, ymax1);

      if (iou > threshold) {
        order[j] = -1;
      }
    }
  }
  return 0;
}

static int quick_sort_indice_inverse(float *input, int left, int right,
                                     int *indices) {
  float key;
  int key_index;
  int low = left;
  int high = right;
  if (left < right) {
    key_index = indices[left];
    key = input[left];
    while (low < high) {
      while (low < high && input[high] <= key) {
        high--;
      }
      input[low] = input[high];
      indices[low] = indices[high];
      while (low < high && input[low] >= key) {
        low++;
      }
      input[high] = input[low];
      indices[high] = indices[low];
    }
    input[low] = key;
    indices[low] = key_index;
    quick_sort_indice_inverse(input, left, low - 1, indices);
    quick_sort_indice_inverse(input, low + 1, right, indices);
  }
  return low;
}

// 筛选有效结果
static int filterValidResult(float *scores, float *loc, float *landms,
                             const float boxPriors[][4], int model_in_h,
                             int model_in_w, int filter_indice[], float *props,
                             float threshold, const int num_results) {
  int validCount = 0;
  const float VARIANCES[2] = {0.1, 0.2};
  // Scale them back to the input size.
  for (int i = 0; i < num_results; ++i) {
    float face_score = scores[i * 2 + 1];
    if (face_score > threshold) {
      filter_indice[validCount] = i;
      props[validCount] = face_score;
      // decode location to origin position
      float xcenter =
          loc[i * 4 + 0] * VARIANCES[0] * boxPriors[i][2] + boxPriors[i][0];
      float ycenter =
          loc[i * 4 + 1] * VARIANCES[0] * boxPriors[i][3] + boxPriors[i][1];
      float w = (float)expf(loc[i * 4 + 2] * VARIANCES[1]) * boxPriors[i][2];
      float h = (float)expf(loc[i * 4 + 3] * VARIANCES[1]) * boxPriors[i][3];

      float xmin = xcenter - w * 0.5f;
      float ymin = ycenter - h * 0.5f;
      float xmax = xmin + w;
      float ymax = ymin + h;

      loc[i * 4 + 0] = xmin;
      loc[i * 4 + 1] = ymin;
      loc[i * 4 + 2] = xmax;
      loc[i * 4 + 3] = ymax;
      for (int j = 0; j < 5; ++j) {
        landms[i * 10 + 2 * j] =
            landms[i * 10 + 2 * j] * VARIANCES[0] * boxPriors[i][2] +
            boxPriors[i][0];
        landms[i * 10 + 2 * j + 1] =
            landms[i * 10 + 2 * j + 1] * VARIANCES[0] * boxPriors[i][3] +
            boxPriors[i][1];
      }
      ++validCount;
    }
  }

  return validCount;
}

int retinaface_post_process(rknn_app_context_t *app_ctx, rknn_output *outputs,
                 letterbox_t *letter_box, retinaface_result *result) {
  float *location = (float *)outputs[0].buf;
  float *scores = (float *)outputs[1].buf;
  float *landms = (float *)outputs[2].buf;
  const float(*prior_ptr)[4];
  int num_priors = 0;
  if (app_ctx->model_height == 320) {
    num_priors = 4200; // anchors box number
    prior_ptr = BOX_PRIORS_320;
  } else if (app_ctx->model_height == 640) {
    num_priors = 16800; // anchors box number
    prior_ptr = BOX_PRIORS_640;
  } else {
    printf("model_shape error!!!\n");
    return -1;
  }

  int filter_indices[num_priors];
  float props[num_priors];

  memset(filter_indices, 0, sizeof(int) * num_priors);
  memset(props, 0, sizeof(float) * num_priors);

  int validCount = filterValidResult(
      scores, location, landms, prior_ptr, app_ctx->model_height,
      app_ctx->model_width, filter_indices, props, CONF_THRESHOLD, num_priors);

  quick_sort_indice_inverse(props, 0, validCount - 1, filter_indices);
  nms(validCount, location, filter_indices, NMS_THRESHOLD);

  int last_count = 0;
  result->count = 0;
  for (int i = 0; i < validCount; ++i) {
    if (last_count >= 128) {
      printf("Warning: detected more than 128 faces, can not handle that");
      break;
    }
    if (filter_indices[i] == -1 || props[i] < VIS_THRESHOLD) {
      continue;
    }

    int n = filter_indices[i];

    float x1 = location[n * 4 + 0] * app_ctx->model_width - letter_box->x_pad;
    float y1 = location[n * 4 + 1] * app_ctx->model_height - letter_box->y_pad;
    float x2 = location[n * 4 + 2] * app_ctx->model_width - letter_box->x_pad;
    float y2 = location[n * 4 + 3] * app_ctx->model_height - letter_box->y_pad;
    int model_in_w = app_ctx->model_width;
    int model_in_h = app_ctx->model_height;
    result->object[last_count].box.left =
        (int)(clamp(x1, 0, model_in_w) / letter_box->scale); // Face box
    result->object[last_count].box.top =
        (int)(clamp(y1, 0, model_in_h) / letter_box->scale);
    result->object[last_count].box.right =
        (int)(clamp(x2, 0, model_in_w) / letter_box->scale);
    result->object[last_count].box.bottom =
        (int)(clamp(y2, 0, model_in_h) / letter_box->scale);
    result->object[last_count].score = props[i]; // Confidence

    for (int j = 0; j < 5; ++j) { // Facial feature points
      float ponit_x =
          landms[n * 10 + 2 * j] * app_ctx->model_width - letter_box->x_pad;
      float ponit_y = landms[n * 10 + 2 * j + 1] * app_ctx->model_height -
                      letter_box->y_pad;
      result->object[last_count].ponit[j].x =
          (int)(clamp(ponit_x, 0, model_in_w) / letter_box->scale);
      result->object[last_count].ponit[j].y =
          (int)(clamp(ponit_y, 0, model_in_h) / letter_box->scale);
    }
    last_count++;
  }

  result->count = last_count;
  return 0;
}
