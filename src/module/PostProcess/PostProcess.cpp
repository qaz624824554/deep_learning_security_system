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

static int clamp(int x, int min, int max)
{
    if(x > max) return max;
    if(x < min) return min;
    return x;
}

// 计算重叠区域
static float CalculateOverlap(float xmin0, float ymin0, float xmax0, float ymax0, float xmin1, float ymin1, float xmax1,
                              float ymax1)
{
    float w = fmax(0.f, fmin(xmax0, xmax1) - fmax(xmin0, xmin1) + 1);
    float h = fmax(0.f, fmin(ymax0, ymax1) - fmax(ymin0, ymin1) + 1);
    float i = w * h;
    float u = (xmax0 - xmin0 + 1) * (ymax0 - ymin0 + 1) + (xmax1 - xmin1 + 1) * (ymax1 - ymin1 + 1) - i;
    return u <= 0.f ? 0.f : (i / u);
}

// 非极大值抑制
static int nms(int validCount, float * outputLocations, int order[], float threshold)
{
    for(int i = 0; i < validCount; ++i) {
        if(order[i] == -1) {
            continue;
        }
        int n = order[i];
        for(int j = i + 1; j < validCount; ++j) {
            int m = order[j];
            if(m == -1) {
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

            float iou = CalculateOverlap(xmin0, ymin0, xmax0, ymax0, xmin1, ymin1, xmax1, ymax1);

            if(iou > threshold) {
                order[j] = -1;
            }
        }
    }
    return 0;
}

static int retinaface_quick_sort_indice_inverse(float * input, int left, int right, int * indices)
{
    float key;
    int key_index;
    int low  = left;
    int high = right;
    if(left < right) {
        key_index = indices[left];
        key       = input[left];
        while(low < high) {
            while(low < high && input[high] <= key) {
                high--;
            }
            input[low]   = input[high];
            indices[low] = indices[high];
            while(low < high && input[low] >= key) {
                low++;
            }
            input[high]   = input[low];
            indices[high] = indices[low];
        }
        input[low]   = key;
        indices[low] = key_index;
        retinaface_quick_sort_indice_inverse(input, left, low - 1, indices);
        retinaface_quick_sort_indice_inverse(input, low + 1, right, indices);
    }
    return low;
}

// 筛选有效结果
static int filterValidResult(float * scores, float * loc, float * landms, const float boxPriors[][4], int model_in_h,
                             int model_in_w, int filter_indice[], float * props, float threshold, const int num_results)
{
    int validCount           = 0;
    const float VARIANCES[2] = {0.1, 0.2};
    // Scale them back to the input size.
    for(int i = 0; i < num_results; ++i) {
        float face_score = scores[i * 2 + 1];
        if(face_score > threshold) {
            filter_indice[validCount] = i;
            props[validCount]         = face_score;
            // decode location to origin position
            float xcenter = loc[i * 4 + 0] * VARIANCES[0] * boxPriors[i][2] + boxPriors[i][0];
            float ycenter = loc[i * 4 + 1] * VARIANCES[0] * boxPriors[i][3] + boxPriors[i][1];
            float w       = (float)expf(loc[i * 4 + 2] * VARIANCES[1]) * boxPriors[i][2];
            float h       = (float)expf(loc[i * 4 + 3] * VARIANCES[1]) * boxPriors[i][3];

            float xmin = xcenter - w * 0.5f;
            float ymin = ycenter - h * 0.5f;
            float xmax = xmin + w;
            float ymax = ymin + h;

            loc[i * 4 + 0] = xmin;
            loc[i * 4 + 1] = ymin;
            loc[i * 4 + 2] = xmax;
            loc[i * 4 + 3] = ymax;
            for(int j = 0; j < 5; ++j) {
                landms[i * 10 + 2 * j] = landms[i * 10 + 2 * j] * VARIANCES[0] * boxPriors[i][2] + boxPriors[i][0];
                landms[i * 10 + 2 * j + 1] =
                    landms[i * 10 + 2 * j + 1] * VARIANCES[0] * boxPriors[i][3] + boxPriors[i][1];
            }
            ++validCount;
        }
    }

    return validCount;
}

int retinaface_post_process(rknn_app_context_t * app_ctx, rknn_output * outputs, letterbox_t * letter_box,
                            retinaface_result * result)
{
    float * location = (float *)outputs[0].buf;
    float * scores   = (float *)outputs[1].buf;
    float * landms   = (float *)outputs[2].buf;
    const float(*prior_ptr)[4];
    int num_priors = 0;
    if(app_ctx->model_height == 320) {
        num_priors = 4200; // anchors box number
        prior_ptr  = BOX_PRIORS_320;
    } else if(app_ctx->model_height == 640) {
        num_priors = 16800; // anchors box number
        prior_ptr  = BOX_PRIORS_640;
    } else {
        printf("model_shape error!!!\n");
        return -1;
    }

    int filter_indices[num_priors];
    float props[num_priors];

    memset(filter_indices, 0, sizeof(int) * num_priors);
    memset(props, 0, sizeof(float) * num_priors);

    int validCount = filterValidResult(scores, location, landms, prior_ptr, app_ctx->model_height, app_ctx->model_width,
                                       filter_indices, props, CONF_THRESHOLD, num_priors);

    retinaface_quick_sort_indice_inverse(props, 0, validCount - 1, filter_indices);
    nms(validCount, location, filter_indices, NMS_THRESHOLD);

    int last_count = 0;
    result->count  = 0;
    for(int i = 0; i < validCount; ++i) {
        if(last_count >= 128) {
            printf("Warning: detected more than 128 faces, can not handle that");
            break;
        }
        if(filter_indices[i] == -1 || props[i] < VIS_THRESHOLD) {
            continue;
        }

        int n = filter_indices[i];

        float x1                              = location[n * 4 + 0] * app_ctx->model_width - letter_box->x_pad;
        float y1                              = location[n * 4 + 1] * app_ctx->model_height - letter_box->y_pad;
        float x2                              = location[n * 4 + 2] * app_ctx->model_width - letter_box->x_pad;
        float y2                              = location[n * 4 + 3] * app_ctx->model_height - letter_box->y_pad;
        int model_in_w                        = app_ctx->model_width;
        int model_in_h                        = app_ctx->model_height;
        result->object[last_count].box.left   = (int)(clamp(x1, 0, model_in_w) / letter_box->scale); // Face box
        result->object[last_count].box.top    = (int)(clamp(y1, 0, model_in_h) / letter_box->scale);
        result->object[last_count].box.right  = (int)(clamp(x2, 0, model_in_w) / letter_box->scale);
        result->object[last_count].box.bottom = (int)(clamp(y2, 0, model_in_h) / letter_box->scale);
        result->object[last_count].score      = props[i]; // Confidence

        for(int j = 0; j < 5; ++j) { // Facial feature points
            float ponit_x = landms[n * 10 + 2 * j] * app_ctx->model_width - letter_box->x_pad;
            float ponit_y = landms[n * 10 + 2 * j + 1] * app_ctx->model_height - letter_box->y_pad;
            result->object[last_count].ponit[j].x = (int)(clamp(ponit_x, 0, model_in_w) / letter_box->scale);
            result->object[last_count].ponit[j].y = (int)(clamp(ponit_y, 0, model_in_h) / letter_box->scale);
        }
        last_count++;
    }

    result->count = last_count;
    return 0;
}

// ============================ yolo post process ============================

static char * labels[OBJ_CLASS_NUM];
static int num_labels = 0;

static char * readLine(FILE * fp, char * buffer, int * len)
{
    int ch;
    int i           = 0;
    size_t buff_len = 0;

    buffer = (char *)malloc(buff_len + 1);
    if(!buffer) return NULL; // Out of memory

    while((ch = fgetc(fp)) != '\n' && ch != EOF) {
        buff_len++;
        void * tmp = realloc(buffer, buff_len + 1);
        if(tmp == NULL) {
            free(buffer);
            return NULL; // Out of memory
        }
        buffer = (char *)tmp;

        buffer[i] = (char)ch;
        i++;
    }
    buffer[i] = '\0';

    *len = buff_len;

    // Detect end
    if(ch == EOF && (i == 0 || ferror(fp))) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

static int readLines(const char * fileName, char * lines[], int max_line)
{
    FILE * file = fopen(fileName, "r");
    char * s;
    int i = 0;
    int n = 0;

    if(file == NULL) {
        std::cout << "Open " << fileName << " fail!" << std::endl;
        return -1;
    }

    while((s = readLine(file, s, &n)) != NULL) {
        lines[i++] = s;
        if(i >= max_line) break;
    }
    fclose(file);
    std::cout << "There are " << i << " lines" << std::endl;
    return i;
}

static int loadLabelName(const char * locationFilename, char * label[])
{
    std::cout << "load lable " << locationFilename << std::endl;
    num_labels = readLines(locationFilename, label, OBJ_CLASS_NUM);
    return 0;
}

const char * coco_cls_to_name(int cls_id)
{
    if(cls_id >= num_labels) {
        return "null";
    }
    if(labels[cls_id]) {
        return labels[cls_id];
    }
    return "null";
}

int init_yolo_post_process(const char * label_path)
{
    int ret = 0;
    ret     = loadLabelName(label_path, labels);
    if(ret < 0) {
        std::cout << "Load " << label_path << " failed!" << std::endl;
        return -1;
    }
    return 0;
}

void deinit_yolo_post_process()
{
    for(int i = 0; i < num_labels; i++) {
        if(labels[i] != nullptr) {
            free(labels[i]);
            labels[i] = nullptr;
        }
    }
}

inline static int32_t __clip(float val, float min, float max)
{
    float f = val <= min ? min : (val >= max ? max : val);
    return f;
}

static int8_t qnt_f32_to_affine(float f32, int32_t zp, float scale)
{
    float dst_val = (f32 / scale) + zp;
    int8_t res    = (int8_t)__clip(dst_val, -128, 127);
    return res;
}

static float deqnt_affine_to_f32(int8_t qnt, int32_t zp, float scale)
{
    return ((float)qnt - (float)zp) * scale;
}

void compute_dfl(float * tensor, int dfl_len, float * box)
{
    for(int b = 0; b < 4; b++) {
        float exp_t[dfl_len];
        float exp_sum = 0;
        float acc_sum = 0;
        for(int i = 0; i < dfl_len; i++) {
            exp_t[i] = exp(tensor[i + b * dfl_len]);
            exp_sum += exp_t[i];
        }

        for(int i = 0; i < dfl_len; i++) {
            acc_sum += exp_t[i] / exp_sum * i;
        }
        box[b] = acc_sum;
    }
}

static int process_i8(int8_t * box_tensor, int32_t box_zp, float box_scale, int8_t * score_tensor, int32_t score_zp,
                      float score_scale, int8_t * score_sum_tensor, int32_t score_sum_zp, float score_sum_scale,
                      int grid_h, int grid_w, int stride, int dfl_len, std::vector<float> & boxes,
                      std::vector<float> & objProbs, std::vector<int> & classId, float threshold)
{
    int validCount            = 0;
    int grid_len              = grid_h * grid_w;
    int8_t score_thres_i8     = qnt_f32_to_affine(threshold, score_zp, score_scale);
    int8_t score_sum_thres_i8 = qnt_f32_to_affine(threshold, score_sum_zp, score_sum_scale);

    for(int i = 0; i < grid_h; i++) {
        for(int j = 0; j < grid_w; j++) {
            int offset       = i * grid_w + j;
            int max_class_id = -1;

            // 通过 score sum 起到快速过滤的作用
            if(score_sum_tensor != nullptr) {
                if(score_sum_tensor[offset] < score_sum_thres_i8) {
                    continue;
                }
            }

            int8_t max_score = -score_zp;
            for(int c = 0; c < num_labels; c++) {
                if((score_tensor[offset] > score_thres_i8) && (score_tensor[offset] > max_score)) {
                    max_score    = score_tensor[offset];
                    max_class_id = c;
                }
                offset += grid_len;
            }

            // compute box
            if(max_score > score_thres_i8) {
                offset = i * grid_w + j;
                float box[4];
                float before_dfl[dfl_len * 4];
                for(int k = 0; k < dfl_len * 4; k++) {
                    before_dfl[k] = deqnt_affine_to_f32(box_tensor[offset], box_zp, box_scale);
                    offset += grid_len;
                }
                compute_dfl(before_dfl, dfl_len, box);

                float x1, y1, x2, y2, w, h;
                x1 = (-box[0] + j + 0.5) * stride;
                y1 = (-box[1] + i + 0.5) * stride;
                x2 = (box[2] + j + 0.5) * stride;
                y2 = (box[3] + i + 0.5) * stride;
                w  = x2 - x1;
                h  = y2 - y1;
                boxes.push_back(x1);
                boxes.push_back(y1);
                boxes.push_back(w);
                boxes.push_back(h);

                objProbs.push_back(deqnt_affine_to_f32(max_score, score_zp, score_scale));
                classId.push_back(max_class_id);
                validCount++;
            }
        }
    }
    return validCount;
}

static int process_fp32(float * box_tensor, float * score_tensor, float * score_sum_tensor, int grid_h, int grid_w,
                        int stride, int dfl_len, std::vector<float> & boxes, std::vector<float> & objProbs,
                        std::vector<int> & classId, float threshold)
{
    int validCount = 0;
    int grid_len   = grid_h * grid_w;
    for(int i = 0; i < grid_h; i++) {
        for(int j = 0; j < grid_w; j++) {
            int offset       = i * grid_w + j;
            int max_class_id = -1;

            // 通过 score sum 起到快速过滤的作用
            if(score_sum_tensor != nullptr) {
                if(score_sum_tensor[offset] < threshold) {
                    continue;
                }
            }

            float max_score = 0;
            for(int c = 0; c < num_labels; c++) {
                if((score_tensor[offset] > threshold) && (score_tensor[offset] > max_score)) {
                    max_score    = score_tensor[offset];
                    max_class_id = c;
                }
                offset += grid_len;
            }

            // compute box
            if(max_score > threshold) {
                offset = i * grid_w + j;
                float box[4];
                float before_dfl[dfl_len * 4];
                for(int k = 0; k < dfl_len * 4; k++) {
                    before_dfl[k] = box_tensor[offset];
                    offset += grid_len;
                }
                compute_dfl(before_dfl, dfl_len, box);

                float x1, y1, x2, y2, w, h;
                x1 = (-box[0] + j + 0.5) * stride;
                y1 = (-box[1] + i + 0.5) * stride;
                x2 = (box[2] + j + 0.5) * stride;
                y2 = (box[3] + i + 0.5) * stride;
                w  = x2 - x1;
                h  = y2 - y1;
                boxes.push_back(x1);
                boxes.push_back(y1);
                boxes.push_back(w);
                boxes.push_back(h);

                objProbs.push_back(max_score);
                classId.push_back(max_class_id);
                validCount++;
            }
        }
    }
    return validCount;
}

static int yolo_quick_sort_indice_inverse(std::vector<float> & input, int left, int right, std::vector<int> & indices)
{
    float key;
    int key_index;
    int low  = left;
    int high = right;
    if(left < right) {
        key_index = indices[left];
        key       = input[left];
        while(low < high) {
            while(low < high && input[high] <= key) {
                high--;
            }
            input[low]   = input[high];
            indices[low] = indices[high];
            while(low < high && input[low] >= key) {
                low++;
            }
            input[high]   = input[low];
            indices[high] = indices[low];
        }
        input[low]   = key;
        indices[low] = key_index;
        yolo_quick_sort_indice_inverse(input, left, low - 1, indices);
        yolo_quick_sort_indice_inverse(input, low + 1, right, indices);
    }
    return low;
}

double rotatedRectIoU(const cv::RotatedRect & rect1, const cv::RotatedRect & rect2)
{
    std::vector<cv::Point2f> intersectingRegion;
    cv::rotatedRectangleIntersection(rect1, rect2, intersectingRegion);
    if(intersectingRegion.empty()) {
        return 0;
    }

    double intersectionArea = cv::contourArea(intersectingRegion);

    double area1 = rect1.size.width * rect1.size.height;
    double area2 = rect2.size.width * rect2.size.height;

    double unionArea = area1 + area2 - intersectionArea;
    return intersectionArea / unionArea;
}

static int yolo_nms(int validCount, std::vector<float> & outputLocations, std::vector<int> classIds,
                    std::vector<int> & order, int filterId, float threshold)
{
    for(int i = 0; i < validCount; ++i) {
        if(order[i] == -1 || classIds[order[i]] != filterId) {
            continue;
        }
        int n = order[i];
        for(int j = i + 1; j < validCount; ++j) {
            int m = order[j];
            if(m == -1 || classIds[order[j]] != filterId) {
                continue;
            }
            float xmin0 = outputLocations[n * 4 + 0];
            float ymin0 = outputLocations[n * 4 + 1];
            float xmax0 = outputLocations[n * 4 + 0] + outputLocations[n * 4 + 2];
            float ymax0 = outputLocations[n * 4 + 1] + outputLocations[n * 4 + 3];

            float xmin1 = outputLocations[m * 4 + 0];
            float ymin1 = outputLocations[m * 4 + 1];
            float xmax1 = outputLocations[m * 4 + 0] + outputLocations[m * 4 + 2];
            float ymax1 = outputLocations[m * 4 + 1] + outputLocations[m * 4 + 3];

            float iou = CalculateOverlap(xmin0, ymin0, xmax0, ymax0, xmin1, ymin1, xmax1, ymax1);

            if(iou > threshold) {
                order[j] = -1;
            }
        }
    }
    return 0;
}

int yolo_post_process(rknn_app_context_t * app_ctx, rknn_output * outputs, letterbox_t * letter_box,
                      float conf_threshold, float nms_threshold, yolo_result_list * results)
{
    std::vector<float> filterBoxes;
    std::vector<float> objProbs;
    std::vector<int> classId;
    int validCount = 0;
    int stride     = 0;
    int grid_h     = 0;
    int grid_w     = 0;
    int model_in_w = app_ctx->model_width;
    int model_in_h = app_ctx->model_height;

    memset(results, 0, sizeof(yolo_result_list));

    // default 3 branch
    int dfl_len           = app_ctx->output_attrs[0].dims[1] / 4;
    int output_per_branch = app_ctx->io_num.n_output / 3;
    for(int i = 0; i < 3; i++) {
        void * score_sum      = nullptr;
        int32_t score_sum_zp  = 0;
        float score_sum_scale = 1.0;
        if(output_per_branch == 3) {
            score_sum       = outputs[i * output_per_branch + 2].buf;
            score_sum_zp    = app_ctx->output_attrs[i * output_per_branch + 2].zp;
            score_sum_scale = app_ctx->output_attrs[i * output_per_branch + 2].scale;
        }
        int box_idx   = i * output_per_branch;
        int score_idx = i * output_per_branch + 1;

        grid_h = app_ctx->output_attrs[box_idx].dims[2];
        grid_w = app_ctx->output_attrs[box_idx].dims[3];
        stride = model_in_h / grid_h;

        if(app_ctx->is_quant) {
            validCount += process_i8((int8_t *)outputs[box_idx].buf, app_ctx->output_attrs[box_idx].zp,
                                     app_ctx->output_attrs[box_idx].scale, (int8_t *)outputs[score_idx].buf,
                                     app_ctx->output_attrs[score_idx].zp, app_ctx->output_attrs[score_idx].scale,
                                     (int8_t *)score_sum, score_sum_zp, score_sum_scale, grid_h, grid_w, stride,
                                     dfl_len, filterBoxes, objProbs, classId, conf_threshold);
        } else {
            validCount +=
                process_fp32((float *)outputs[box_idx].buf, (float *)outputs[score_idx].buf, (float *)score_sum, grid_h,
                             grid_w, stride, dfl_len, filterBoxes, objProbs, classId, conf_threshold);
        }
    }

    if(validCount <= 0) {
        return 0;
    }
    std::vector<int> indexArray;

    for(int i = 0; i < validCount; ++i) {
        indexArray.push_back(i);
    }
    yolo_quick_sort_indice_inverse(objProbs, 0, validCount - 1, indexArray);

    std::set<int> class_set(std::begin(classId), std::end(classId));

    for(auto c : class_set) {
        yolo_nms(validCount, filterBoxes, classId, indexArray, c, nms_threshold);
    }

    int last_count = 0;
    results->count = 0;

    /* box valid detect target */
    for(int i = 0; i < validCount; ++i) {

        if(indexArray[i] == -1 || last_count >= OBJ_NUMB_MAX_SIZE) {
            continue;
        }
        int n = indexArray[i];

        float x1       = filterBoxes[n * 4 + 0] - letter_box->x_pad;
        float y1       = filterBoxes[n * 4 + 1] - letter_box->y_pad;
        float x2       = x1 + filterBoxes[n * 4 + 2];
        float y2       = y1 + filterBoxes[n * 4 + 3];
        int id         = classId[n];
        float obj_conf = objProbs[i];

        results->results[last_count].box.left   = (int)(clamp(x1, 0, model_in_w) / letter_box->scale);
        results->results[last_count].box.top    = (int)(clamp(y1, 0, model_in_h) / letter_box->scale);
        results->results[last_count].box.right  = (int)(clamp(x2, 0, model_in_w) / letter_box->scale);
        results->results[last_count].box.bottom = (int)(clamp(y2, 0, model_in_h) / letter_box->scale);
        results->results[last_count].prop       = obj_conf;
        results->results[last_count].cls_id     = id;
        last_count++;
    }
    results->count = last_count;
    return 0;
}
