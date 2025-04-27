# 基于深度学习的人脸门禁+ IPC 智能安防监控系统

本项目基于 yolo11、retinaface、facenet 深度学习模型，GUI 为 LVGL，实现人脸检测、人脸识别、物体检测的 IPC 智能安防监控系统，满帧运行。

英文版请见 [README_en.md](README_en.md)

## 克隆项目

```sh
git clone https://github.com/qaz624824554/deep_learning_security_system.git
cd deep_learning_security_system
```

LVGL 是本项目的一个子模块，使用以下命令获取它，将会下载到 lvgl/ 目录

```sh
git submodule update --init --recursive
```

## 前置条件

### 依赖库

本项目依赖多个第三方库，请确保你的开发板已经安装以下库：

- [Opencv4.9](https://github.com/opencv/opencv/tree/4.9.0)
- [RKNN](https://github.com/airockchip/rknn-toolkit2)
- [FFmpeg-rockchip](https://github.com/nyanmisaka/ffmpeg-rockchip)
- [MPP](https://github.com/rockchip-linux/mpp)
- [BlueAlsa](https://github.com/arkq/bluez-alsa)

### 硬件

<img src="./assets/hardware.jpeg" alt="硬件" width="50%">

- 野火鲁班猫4 RK3588S2
- IMX415 800W 4k 摄像头
- RTL8822CE Wifi+BT
- mipi LCD RGB 7寸 1024×600 触摸屏
- 人体红外检测 SR501：通过 GPIO 读取相应数值，用于判断是否有人靠近
- 128×32 OLED屏：通过 I2C 通信显示画面内容，用于显示人脸检测结果
- 蓝牙音箱

## 开始使用

编译

```sh
cmake -B build .
cmake --build build -j8
```

运行 `/bin` 目录下的 `lvglsim` 文件即可。

## 线程图

<img src="./assets/process.png" alt="处理流程图" width="50%">

## 演示视频

