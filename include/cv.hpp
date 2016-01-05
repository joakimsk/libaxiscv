#ifndef __CV_HPP_INCLUDED__
#define __CV_HPP_INCLUDED__

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>


class AxisVision {
public:
	AxisVision();
	void StartCapture();
	void ReadFrame();
	cv::UMat GetFrame();
	cv::Mat GetOverlay();
	cv::Point2f GetLastKnownPosition();
	void GrabFrame();
	void FindObject();
	~AxisVision();
protected:
	cv::VideoCapture capture_;
	cv::UMat grab_picture_;
	cv::Mat ar_overlay_;

private:
	int producer_consumer_counter_ = 0;
	cv::Point2f last_known_position_;
};
#endif