#include <iostream>
#include <stdio.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <algorithm> // std::remove_if
#include <stdexcept>
#include <cmath>
#include "../include/cv.hpp"

cv::RNG rng(12345);

AxisVision::AxisVision(void) { std::cout << "AxisVision created" << std::endl; }

void AxisVision::StartCapture() {
  if (!capture_.isOpened()) {
    std::cout << "AxisVision::StartCapture - Capture is not open, opening..."
              << std::endl;
    if (capture_.open("http://ptz:ptz@192.168.0.120/mjpg/video.mjpg")) {
      std::cout << "AxisVision::StartCapture - Opened device successfully."
                << std::endl;
    } else {
      std::cout << "AxisVision::StartCapture - Device opening FAILED!"
                << std::endl;
    }
  } else {
    std::cout << "AxisVision::StartCapture - Device already open" << std::endl;
  }
}

void AxisVision::ReadFrame() {
  if (!capture_.isOpened()) {
    std::cout << "AxisVision::ReadFrame - Device is not open. FAILED!"
              << std::endl;
  } else {
    std::cout << "AxisVision::ReadFrame - Frame reading now..." << std::endl;
    capture_ >> grab_picture_;
    producer_consumer_counter_++;
    std::cout << "producer_consumer_counter_=" << producer_consumer_counter_
              << std::endl;
  }
}

cv::UMat AxisVision::GetFrame() {
  std::cout << "AxisVision::GetFrame returning class stored frame" << std::endl;
  producer_consumer_counter_--;
  std::cout << "producer_consumer_counter_=" << producer_consumer_counter_
            << std::endl;

  return grab_picture_;
}

void AxisVision::GrabFrame() {
  if (!capture_.grab()) {
    std::cout << "Camera->GrabFrame->  Can not grab images." << std::endl;
  } else {
    if (!capture_.retrieve(grab_picture_)) {
      std::cout << "Webcam->RetrieveFrame->  Can not retrieve image."
                << std::endl;
    } else {
      cv::namedWindow("Named Window", cv::WINDOW_AUTOSIZE);
      cv::imshow("Named Window", grab_picture_);
      cv::waitKey(1);
    }
    // std::cout << "Camera->GrabFrame->GRAB OK" << std::endl;
  }
}

void AxisVision::FindObject() {
  ar_overlay_ = cv::Mat::zeros(grab_picture_.size(), CV_8UC3);

  cv::UMat hsv;
  cv::cvtColor(grab_picture_, hsv, cv::COLOR_BGR2HSV);
  cv::UMat lower_red_hue_range;
  cv::UMat upper_red_hue_range;
  cv::inRange(hsv, cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255),
              lower_red_hue_range);
  cv::inRange(hsv, cv::Scalar(160, 100, 100), cv::Scalar(179, 255, 255),
              upper_red_hue_range);
  cv::UMat red_hue_image;
  // cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0,
  // red_hue_image);
  cv::bitwise_or(lower_red_hue_range, upper_red_hue_range, red_hue_image);

  int morph_size = 1;
  cv::Mat selement = getStructuringElement(
      cv::MORPH_ELLIPSE, cv::Size(4 * morph_size + 1, 2 * morph_size + 1),
      cv::Point(morph_size, morph_size));

  cv::erode(red_hue_image, red_hue_image, selement, cv::Point(-1, -1), 2);
  cv::dilate(red_hue_image, red_hue_image, selement, cv::Point(-1, -1), 2);

  cv::GaussianBlur(red_hue_image, red_hue_image, cv::Size(9, 9), 2, 2);

  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;

  cv::findContours(red_hue_image, contours, hierarchy, CV_RETR_TREE,
                   CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

  cv::Mat drawing = cv::Mat::zeros(red_hue_image.size(), CV_8UC3);

  cv::Rect bounding_rect;
  cv::Moments moment_of_largest;
  cv::Point2f centroid_of_largest;
  double largest_area = 0;
  int largest_contour_index = 0;
  for (int i = 0; i < contours.size(); i++) {
    double a = cv::contourArea(contours[i], false);

    if (a > largest_area) {
      largest_area = a;
      // Store the index of largest contour
      largest_contour_index = i;
      // Find the bounding rectangle for biggest contour
      bounding_rect = cv::boundingRect(contours[i]);
    }

    cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
                                  rng.uniform(0, 255));
    cv::drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0,
                     cv::Point());
  }

  if (largest_area > 100) {
    moment_of_largest = cv::moments(contours[largest_contour_index], false);
    centroid_of_largest = cv::Point2f(
        static_cast<float>(moment_of_largest.m10 / moment_of_largest.m00),
        static_cast<float>(moment_of_largest.m01 / moment_of_largest.m00));

    std::cout << "Centroid of largest = " << centroid_of_largest << std::endl;

    cv::circle(ar_overlay_, centroid_of_largest, 4, cv::Scalar(255, 0, 0), -1, 8,
               0);
    last_known_position_ = centroid_of_largest;
  }

  cv::rectangle(ar_overlay_, bounding_rect, cv::Scalar(255, 255, 0), 2, 8, 0);

  /*
  cv::namedWindow("Named Windowx", cv::WINDOW_AUTOSIZE);
  cv::imshow("Named Windowx", drawing);
  */
  cv::namedWindow("Overlay", cv::WINDOW_AUTOSIZE);
  cv::imshow("Overlay", ar_overlay_);
  cv::waitKey(1);
}

cv::Mat AxisVision::GetOverlay(){
  return ar_overlay_;
}

cv::Point2f AxisVision::GetLastKnownPosition(){
  return last_known_position_;
}

AxisVision::~AxisVision(void) {
  std::cout << "AxisVision destroyed" << std::endl;
}