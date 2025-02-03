#include "ImageProcess.hpp"
#include <string>

// 定义最大类别数
#define N_CLASS_COLORS (20)

// 人脸框长度
#define FACE_BOX_LENGTH 60

// 为不同的类别定义颜色，每个类别对应一个 RGB 色值
static unsigned char class_colors[][3] = {
    {255, 56, 56},   // 'FF3838'
    {255, 157, 151}, // 'FF9D97'
    {255, 112, 31},  // 'FF701F'
    {255, 178, 29},  // 'FFB21D'
    {207, 210, 49},  // 'CFD231'
    {72, 249, 10},   // '48F90A'
    {146, 204, 23},  // '92CC17'
    {61, 219, 134},  // '3DDB86'
    {26, 147, 52},   // '1A9334'
    {0, 212, 187},   // '00D4BB'
    {44, 153, 168},  // '2C99A8'
    {0, 194, 255},   // '00C2FF'
    {52, 69, 147},   // '344593'
    {100, 115, 255}, // '6473FF'
    {0, 24, 236},    // '0018EC'
    {132, 56, 255},  // '8438FF'
    {82, 0, 133},    // '520085'
    {203, 56, 255},  // 'CB38FF'
    {255, 149, 200}, // 'FF95C8'
    {255, 55, 199}   // 'FF37C7'
};

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

        if(name == "person") {
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
}