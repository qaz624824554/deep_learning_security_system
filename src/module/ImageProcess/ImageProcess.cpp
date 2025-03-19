#include "ImageProcess.hpp"
#include <string>

// 定义最大类别数
#define N_CLASS_COLORS (20)

// 人脸框长度
#define FACE_BOX_LENGTH 60

// 计算缩放比例和填充大小的构造函数
ImageProcess::ImageProcess(int width, int height, int target_size)
{
    // 根据目标大小计算缩放比例
    scale_ = static_cast<double>(target_size) / std::max(height, width);

    // 根据缩放比例计算填充的大小
    padding_x_ = target_size - static_cast<int>(width * scale_);
    padding_y_ = target_size - static_cast<int>(height * scale_);

    // 计算新的尺寸
    new_size_ = cv::Size(static_cast<int>(width * scale_), static_cast<int>(height * scale_));

    target_size_ = target_size;

    // 设置填充信息
    letterbox_.scale = scale_;
    letterbox_.x_pad = padding_x_ / 2;
    letterbox_.y_pad = padding_y_ / 2;
}

// 将图像转换为目标大小，填充并返回
std::unique_ptr<cv::Mat> ImageProcess::convert(const cv::Mat & src)
{
    if(src.empty()) {
        return nullptr;
    }

    // 调整图像大小
    cv::Mat resize_img;
    cv::resize(src, resize_img, new_size_);

    // 创建一个新的目标大小的空白图像，使用填充颜色
    auto square_img = std::make_unique<cv::Mat>(target_size_, target_size_, src.type(), cv::Scalar(114, 114, 114));

    // 计算填充位置
    cv::Point position(padding_x_ / 2, padding_y_ / 2);

    // 将调整大小后的图像复制到目标图像
    resize_img.copyTo((*square_img)(cv::Rect(position.x, position.y, resize_img.cols, resize_img.rows)));

    return square_img;
}

// 获取 letterbox 配置，用于图像填充
const letterbox_t & ImageProcess::get_letter_box()
{
    return letterbox_;
}

// 图像后处理，进行物体检测和后续处理
void ImageProcess::image_post_process(cv::Mat & image, retinaface_result & results, cv::Scalar & color)
{

    for(int i = 0; i < results.count; ++i) {

        retinaface_object * detect_result = &(results.object[i]);

        // 绘制检测框
        // 左上角
        cv::line(image, cv::Point(detect_result->box.left, detect_result->box.top),
                 cv::Point(detect_result->box.left + FACE_BOX_LENGTH, detect_result->box.top), color, 5);
        cv::line(image, cv::Point(detect_result->box.left, detect_result->box.top),
                 cv::Point(detect_result->box.left, detect_result->box.top + FACE_BOX_LENGTH), color, 5);

        // 右上角
        cv::line(image, cv::Point(detect_result->box.right - FACE_BOX_LENGTH, detect_result->box.top),
                 cv::Point(detect_result->box.right, detect_result->box.top), color, 5);
        cv::line(image, cv::Point(detect_result->box.right, detect_result->box.top),
                 cv::Point(detect_result->box.right, detect_result->box.top + FACE_BOX_LENGTH), color, 5);

        // 左下角
        cv::line(image, cv::Point(detect_result->box.left, detect_result->box.bottom - FACE_BOX_LENGTH),
                 cv::Point(detect_result->box.left, detect_result->box.bottom), color, 5);
        cv::line(image, cv::Point(detect_result->box.left, detect_result->box.bottom),
                 cv::Point(detect_result->box.left + FACE_BOX_LENGTH, detect_result->box.bottom), color, 5);

        // 右下角
        cv::line(image, cv::Point(detect_result->box.right - FACE_BOX_LENGTH, detect_result->box.bottom),
                 cv::Point(detect_result->box.right, detect_result->box.bottom), color, 5);
        cv::line(image, cv::Point(detect_result->box.right, detect_result->box.bottom - FACE_BOX_LENGTH),
                 cv::Point(detect_result->box.right, detect_result->box.bottom), color, 5);
    }
}

void ImageProcess::image_post_process(cv::Mat & image, yolo_result_list & results, cv::Scalar & color)
{
    for(int i = 0; i < results.count; ++i) {
        yolo_result * detect_result = &(results.results[i]);

        std::string name = std::string(coco_cls_to_name(detect_result->cls_id));

        cv::rectangle(image, cv::Point(detect_result->box.left, detect_result->box.top),
                      cv::Point(detect_result->box.right, detect_result->box.bottom), color, 5);

        cv::rectangle(image, cv::Point(detect_result->box.left, detect_result->box.top - 100),
                      cv::Point(detect_result->box.left + 380, detect_result->box.top), color, cv::FILLED);

        char text[256];
        sprintf(text, "%s", name.c_str());
        cv::putText(image, text, cv::Point(detect_result->box.left, detect_result->box.top - 30),
                    cv::FONT_HERSHEY_COMPLEX, 3, cv::Scalar(0, 0, 0), 5,
                    cv::LINE_8); // 绘制类别标签
    }
}