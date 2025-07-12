# 26835_NearLink_WS63-based-Smart-Home-Monitoring-System

##### 作品简介

hisilicon赛道基于NearLink_WS63的智能居家检测系统与边缘计算架构

该项目实现佩戴端采集佩戴者的三轴加速度值以及心率血氧值，通过SLE通信传输至固定端，固定端采集温湿度数据并将数据通过串口传输至树莓派进行算法检测后上云。

实现通过数据大屏实时监测老人状态，监测到跌倒事件及时告警。

##### 代码介绍

sensor_sle 为运行于WS63E开发板的程序源代码。

以传感器名字命名的文件夹实现了对应传感器数据采集功能。

主程序位于sensor_sle.c中。

![](C:\Users\Administrator\AppData\Roaming\marktext\images\2025-07-12-16-27-44-image.png)
