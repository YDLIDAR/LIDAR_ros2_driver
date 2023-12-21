###### *需先安装LIDAR_SDK

## 进入工程目录，编译

```
colcon build
source install/setup.bash
```

## 使用

```
ros2 launch lidar_ros2_driver lidar_launch.py
或
ros2 launch lidar_ros2_driver lidar_launch_view.py
```

输出数据如下表:

| 话题           | 类型                     |
| ------------ | ---------------------- |
| /scan        | sensor_msgs/LaserScan  |
| /point_cloud | sensor_msgs/PointCloud |

launch中主要参数含义如下表:

| 参数名称        | 参数含义                              |
| ----------- | --------------------------------- |
| ip          | 雷达的ip地址                           |
| port        | 雷达的tcp通信端口                        |
| lidar_type  | 雷达的类型, 目前默认为0                     |
| angle_min   | 最小扫描角度(单位°), 目前支持最小到-150          |
| angle_max   | 最大扫描角度(单位°), 目前支持最大到150           |
| range_min   | 最小扫描距离(单位米), 目前支持最小到0.1米          |
| range_max   | 最大扫描距离(单位米), 目前支持最大到25米           |
| frequency   | 雷达扫描转速(单位Hz),目前支持10~40Hz,默认20Hz   |
| sample_rate | 雷达采样频率(单位K，即每秒雷达输出的点数),目前支持20~40K |

##### 

##### 常见错误

一.  /scan或者/point_cloud不显示，且终端提示类似如下错误：

New publisher discovered on topic '/point_cloud', offering incompatible QoS. No messages will be sent to it. Last incompatible policy: RELIABILITY_QOS_POLICY

解决办法：找到rviz软件中对应话题>Topic>Reliability Policy属性，将其改成SystemDefault或者Best Effort即可。
