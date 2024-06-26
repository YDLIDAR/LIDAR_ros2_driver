﻿/*
 *  LIDAR SYSTEM
 *  LIDAR ROS 2 Node
 *
 *  Copyright 2017 - 2020 EAI TEAM
 *  http://www.eaibot.com
 *
 */

#ifdef _MSC_VER
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#endif

#include "src/CLidar.h"
#include <math.h>
#include <chrono>
#include <iostream>
#include "core/math/angles.h"
#include <memory>
#include "sensor_msgs/msg/point_cloud.hpp"
#include "rclcpp/clock.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/time_source.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "std_srvs/srv/empty.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <signal.h>

#define ROS2Verision "1.0.1"


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

  auto node = rclcpp::Node::make_shared("lidar_ros2_driver_node");

  RCLCPP_INFO(node->get_logger(), "[LIDAR INFO] Current ROS Driver Version: %s\n", ((std::string)ROS2Verision).c_str());

  CLidar laser;
  std::string str_optvalue = "192.168.0.11";
  node->declare_parameter("ip", str_optvalue);
  node->get_parameter("ip", str_optvalue);
  ///lidar port
  laser.setlidaropt(LidarPropSerialPort, str_optvalue.c_str(), str_optvalue.size());

  ///ignore array
  str_optvalue = "";
  node->declare_parameter("ignore_array", str_optvalue);
  node->get_parameter("ignore_array", str_optvalue);
  laser.setlidaropt(LidarPropIgnoreArray, str_optvalue.c_str(), str_optvalue.size());

  std::string frame_id = "laser_frame";
  node->declare_parameter("frame_id", frame_id);
  node->get_parameter("frame_id", frame_id);

  //////////////////////int property/////////////////
  /// lidar  port
  int optval = 8090;
  node->declare_parameter("port", optval);
  node->get_parameter("port", optval);
  laser.setlidaropt(LidarPropSerialBaudrate, &optval, sizeof(int));
  /// lidar type
  optval = TYPE_LIDAR;
  node->declare_parameter("lidar_type", optval);
  node->get_parameter("lidar_type", optval);
  laser.setlidaropt(LidarPropLidarType, &optval, sizeof(int));
  bool b_optvalue;
  node->declare_parameter("auto_reconnect", b_optvalue);
  node->get_parameter("auto_reconnect", b_optvalue);
  laser.setlidaropt(LidarPropAutoReconnect, &b_optvalue, sizeof(bool));
  /// sample rate
  optval = 9;
  node->declare_parameter("sample_rate", optval);
  node->get_parameter("sample_rate", optval);
  laser.setlidaropt(LidarPropSampleRate, &optval, sizeof(int));

  //////////////////////float property/////////////////
  /// unit: °
  float f_optvalue = 180.0f;
  node->declare_parameter("angle_max", f_optvalue);
  node->get_parameter("angle_max", f_optvalue);
  laser.setlidaropt(LidarPropMaxAngle, &f_optvalue, sizeof(float));
  f_optvalue = -180.0f;
  node->declare_parameter("angle_min", f_optvalue);
  node->get_parameter("angle_min", f_optvalue);
  laser.setlidaropt(LidarPropMinAngle, &f_optvalue, sizeof(float));
  /// unit: m
  f_optvalue = 64.f;
  node->declare_parameter("range_max", f_optvalue);
  node->get_parameter("range_max", f_optvalue);
  laser.setlidaropt(LidarPropMaxRange, &f_optvalue, sizeof(float));
  f_optvalue = 0.1f;
  node->declare_parameter("range_min", f_optvalue);
  node->get_parameter("range_min", f_optvalue);
  laser.setlidaropt(LidarPropMinRange, &f_optvalue, sizeof(float));
  /// unit: Hz
  f_optvalue = 20.f;
  node->declare_parameter("frequency", f_optvalue);
  node->get_parameter("frequency", f_optvalue);
  laser.setlidaropt(LidarPropScanFrequency, &f_optvalue, sizeof(float));

  node->declare_parameter("reversion", b_optvalue);
  node->get_parameter("reversion", b_optvalue);
  laser.setlidaropt(LidarPropReversion, &b_optvalue, sizeof(bool));

  bool ret = laser.initialize();
  if (ret) {
    ret = laser.turnOn();
  } else {
    RCLCPP_ERROR(node->get_logger(), "%s\n", laser.DescribeError());
  }
  
  auto laser_pub = node->create_publisher<sensor_msgs::msg::LaserScan>("scan", rclcpp::SensorDataQoS());
  auto pc_pub = node->create_publisher<sensor_msgs::msg::PointCloud>("point_cloud", rclcpp::SensorDataQoS());
  
  auto stop_scan_service =
    [&laser](const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<std_srvs::srv::Empty::Request> req,
  std::shared_ptr<std_srvs::srv::Empty::Response> response) -> bool
  {
    return laser.turnOff();
  };

  auto stop_service = node->create_service<std_srvs::srv::Empty>("stop_scan",stop_scan_service);

  auto start_scan_service =
    [&laser](const std::shared_ptr<rmw_request_id_t> request_header,
  const std::shared_ptr<std_srvs::srv::Empty::Request> req,
  std::shared_ptr<std_srvs::srv::Empty::Response> response) -> bool
  {
    return laser.turnOn();
  };

  auto start_service = node->create_service<std_srvs::srv::Empty>("start_scan",start_scan_service);

  rclcpp::WallRate loop_rate(20);

  while (ret && rclcpp::ok()) {

    LaserScan scan;//

    if (laser.doProcessSimple(scan)) {
      auto scan_msg = std::make_shared<sensor_msgs::msg::LaserScan>();
      auto pc_msg = std::make_shared<sensor_msgs::msg::PointCloud>();

      scan_msg->header.stamp.sec = RCL_NS_TO_S(scan.stamp);
      scan_msg->header.stamp.nanosec =  scan.stamp - RCL_S_TO_NS(scan_msg->header.stamp.sec);
      scan_msg->header.frame_id = frame_id;
      pc_msg->header = scan_msg->header;
      scan_msg->angle_min = scan.config.min_angle;
      scan_msg->angle_max = scan.config.max_angle;
      scan.config.angle_increment = 300.0 / scan.points.size() / 180 * M_PI;
      scan_msg->angle_increment = scan.config.angle_increment;
      scan_msg->scan_time = scan.config.scan_time;
      scan_msg->time_increment = scan.config.time_increment;
      scan_msg->range_min = scan.config.min_range;
      scan_msg->range_max = scan.config.max_range;
      int size = (scan.config.max_angle - scan.config.min_angle) / scan.config.angle_increment + 1;
      scan_msg->ranges.resize(size);
      scan_msg->intensities.resize(size);
      pc_msg->channels.resize(2);
      int idx_intensity = 0;
      pc_msg->channels[idx_intensity].name = "intensities";
      int idx_timestamp = 1;
      pc_msg->channels[idx_timestamp].name = "stamps";
      for(size_t i=0; i < scan.points.size(); i++) {
        int index = std::ceil((M_PI - scan.points[i].angle / 180.f * M_PI - scan.config.min_angle) 
          / scan.config.angle_increment);
        if(index >= 0 && index < size) {
	  if (scan.points[i].range >= scan.config.min_range && scan.points[i].range <= scan.config.max_range) {
            scan_msg->ranges[index] = scan.points[i].range;
            scan_msg->intensities[index] = scan.points[i].intensity;
	  }
        }
        float curAngle = scan.points[i].angle / 180.f * M_PI - M_PI;
	if (curAngle >= scan.config.min_angle && curAngle <= scan.config.max_angle
	    && scan.points[i].range >= scan.config.min_range && scan.points[i].range <= scan.config.max_range) {
          geometry_msgs::msg::Point32 point;
          point.x = scan.points[i].range * cos(curAngle);
          point.y = scan.points[i].range * sin(curAngle);
          point.z = 0.0;
          pc_msg->points.push_back(point);
          pc_msg->channels[idx_intensity].values.push_back(scan.points[i].intensity);
          pc_msg->channels[idx_timestamp].values.push_back(i * scan.config.time_increment);
        }
      }
      laser_pub->publish(*scan_msg);
      pc_pub->publish(*pc_msg);
    } else {
      RCLCPP_ERROR(node->get_logger(), "Failed to get scan");
    }
    if(!rclcpp::ok()) {
      break;
    }
    rclcpp::spin_some(node);
    loop_rate.sleep();
  }


  RCLCPP_INFO(node->get_logger(), "[LIDAR INFO] Now LIDAR is stopping .......");
  laser.turnOff();
  laser.disconnecting();
  rclcpp::shutdown();

  return 0;
}
