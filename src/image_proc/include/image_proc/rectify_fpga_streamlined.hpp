/*
      ____  ____
     /   /\/   /
    /___/  \  /   Copyright (c) 2021, Xilinx®.
    \   \   \/    Author: Víctor Mayoral Vilches <victorma@xilinx.com>
     \   \
     /   /        Licensed under the Apache License, Version 2.0 (the "License");
    /___/   /\    you may not use this file except in compliance with the License.
    \   \  /  \   You may obtain a copy of the License at
     \___\/\___\            http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    Inspired by rectify.hpp authored by Willow Garage, Inc., Andreas Klintberg,
      Joshua Whitley
*/

#ifndef IMAGE_PROC__RECTIFY_FPGA_STREAMLINED_HPP_
#define IMAGE_PROC__RECTIFY_FPGA_STREAMLINED_HPP_

#include <rclcpp/rclcpp.hpp>
#include <image_transport/image_transport.hpp>
#include <image_geometry/pinhole_camera_model.h>
#include <cv_bridge/cv_bridge.h>

#include <thread>
#include <memory>
#include <vector>
#include <string>

#include <vitis_common/common/ros_opencl_120.hpp>
#include <vitis_common/common/utilities.hpp>

namespace image_geometry
{

class PinholeCameraModelFPGAStreamlined: public image_geometry::PinholeCameraModel
{
public:

  /* \brief Constructor
   */
  PinholeCameraModelFPGAStreamlined();

  /**
   * \brief Rectify a raw camera image offloading the remapping to the FPGA.
   *
   *  TODO: Consider pushing OpenCV cv::initRectificationMaps also to the FPGA
   *  by using Vitis Vision Library xf::cv::InitUndistortRectifyMapInverse
   */
  void rectifyImageFPGA(const cv::Mat& raw, cv::Mat& rectified, bool gray) const;

  /**
   * \brief Auxiliary method to debug rectification across CPU and FPGA
   */
  void rectifyImageFPGA_debug(const cv::Mat& raw, cv::Mat& rectified, bool gray) const;

private:
  cl::Kernel* krnl_;
  cl::Context* context_;
  cl::CommandQueue* queue_;
};

} //namespace image_geometry

namespace image_proc
{

class RectifyNodeFPGAStreamlined
  : public rclcpp::Node
{
public:
  explicit RectifyNodeFPGAStreamlined(const rclcpp::NodeOptions &);

private:
  image_transport::CameraSubscriber sub_camera_;

  int queue_size_;
  int interpolation;
  std::mutex connect_mutex_;
  image_transport::Publisher pub_rect_;

  // Processing state (note: only safe because we're using single-threaded NodeHandle!)
  image_geometry::PinholeCameraModelFPGAStreamlined model_;

  void subscribeToCamera();
  void imageCb(
    const sensor_msgs::msg::Image::ConstSharedPtr & image_msg,
    const sensor_msgs::msg::CameraInfo::ConstSharedPtr & info_msg);
};

}  // namespace image_proc

#endif  // IMAGE_PROC__RECTIFY_FPGA_STREAMLINED_HPP_
