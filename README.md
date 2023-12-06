###### *需先安装LIDAR_SDK  https://github.com/YDLIDAR/LIDAR_SDK

## 进入工程目录，编译

```
colcon build
source install/setup.bash
```

## 使用

```
ros2 launch lidar_ros2_driver lidar_launch.py
```

输出数据到/scan话题
