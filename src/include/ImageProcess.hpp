//
// Created by kaylor on 3/4/24.
//

#pragma once

#include "mutex"
#include "PostProcess.hpp"
#include <opencv2/opencv.hpp>

class ImageProcess {
public:
  ImageProcess(int width, int height, int target_size);
  std::unique_ptr<cv::Mat> convert(const cv::Mat &src);
  const letterbox_t &get_letter_box();
  void image_post_process(cv::Mat &image, retinaface_result &results, cv::Scalar &color);

private:
  double scale_;
  int padding_x_;
  int padding_y_;
  cv::Size new_size_;
  int target_size_;
  letterbox_t letterbox_;
};
