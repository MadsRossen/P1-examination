
#include <cv_bridge/cv_bridge.h>
#include <cstdlib>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include "ros/ros.h"
#include "opencv2/opencv.hpp"
#include "ros/console.h"
#include <color/colordetect.h>


void LineDetect::imageCallback(const sensor_msgs::ImageConstPtr& msg) {
  cv_bridge::CvImagePtr cv_ptr;
  try {
    cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    img = cv_ptr->image;
    cv::waitKey(30);
  }
  catch (cv_bridge::Exception& e) {
    ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
  }
}

cv::Mat LineDetect::Gauss(cv::Mat input) {
  cv::Mat output;
// Applying Gaussian Filter
  cv::GaussianBlur(input, output, cv::Size(3, 3), 0.1, 0.1);
  return output;
}

void LineDetect::colorthresh(cv::Mat input) {
  // Initializaing variables
  cv::Size s = input.size();
  std::vector<std::vector<cv::Point> > v;
  auto w = s.width;
  auto h = s.height;
  auto c_x = 0.0;
  // Detect all objects within the HSV range
  cv::cvtColor(input, LineDetect::img_hsv, CV_BGR2HSV);
  LineDetect::LowerYellow = {70, 160, 61};
  LineDetect::UpperYellow = {102, 240, 102};
  cv::inRange(LineDetect::img_hsv, LowerYellow,
   UpperYellow, LineDetect::img_mask);
  img_mask(cv::Rect(0, 0, w, 0.8*h)) = 0;
  // Find contours for better visualization
  cv::findContours(LineDetect::img_mask, v, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
  // If contours exist add a bounding
  // Choosing contours with maximum area
  if (v.size() != 0) {
  auto area = 0;
  auto idx = 0;
  auto count = 0;
  while (count < v.size()) {
    if (area < v[count].size()) {
       idx = count;
       area = v[count].size();
    }
    count++;
  }
  cv::Rect rect = boundingRect(v[idx]);
  cv::Point pt1, pt2, pt3;
  pt1.x = rect.x;
  pt1.y = rect.y;
  pt2.x = rect.x + rect.width;
  pt2.y = rect.y + rect.height;
  pt3.x = pt1.x+5;
  pt3.y = pt1.y-5;
  // Drawing the rectangle using points obtained
  rectangle(input, pt1, pt2, CV_RGB(255, 0, 0), 2);
  // Inserting text box
  cv::putText(input, "Line Detected", pt3,
    CV_FONT_HERSHEY_COMPLEX, 1, CV_RGB(255, 0, 0));
  }
  // Mask image to limit the future turns affecting the output
  img_mask(cv::Rect(0.7*w, 0, 0.3*w, h)) = 0;
  img_mask(cv::Rect(0, 0, 0.3*w, h)) = 0;
  // Perform centroid detection of line
  cv::Moments M = cv::moments(LineDetect::img_mask);
  if (M.m00 > 0) {
    cv::Point p1(M.m10/M.m00, M.m01/M.m00);
    cv::circle(LineDetect::img_mask, p1, 5, cv::Scalar(155, 200, 0), -1);
  }
  // Tolerance to chooise direction
 
  // Turn left if centroid is to the left of the image center minus tolerance
  // Output images viewed by the turtlebot
  //cv::namedWindow(OPENCV_WINDOW,600);
  cv::imshow(OPENCV_WINDOW_ORIGINAL, input);
  //return LineDetect::dir;
}

int main(int argc, char **argv) {
    // Initializing node and object
    ros::init(argc, argv, "detection");
    //ros::NodeHandle n;
    LineDetect det;
    // Creating Publisher and subscriber
    /*ros::Subscriber sub = n.subscribe("/camera/rgb/image_raw",
        1, &LineDetect::imageCallback, &det);
    */
    while (ros::ok()) {
        if (!det.img.empty()) {
            // Perform image processing
            det.img_filt = det.Gauss(det.img);
            det.colorthresh(det.img_filt);
            }
        ros::spinOnce();
    }
    // Closing image viewer
    cv::destroyWindow("Turtlebot View");
}
