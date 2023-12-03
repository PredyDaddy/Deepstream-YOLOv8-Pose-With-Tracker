# Deepstream YOLOv8-Pose With Tracker
This is an implementation of a video analysis framework based on Deepstream YOLOv8-Pose with ByteTrack, utilizing NVIDIA Deepstream 6.2. It integrates seven different trackers, effectively compensating for scenarios where detection results are suboptimal. The framework offers two types of pipelines: one is a file sink, and the other is an RTSP sink. These two sinks are provided to facilitate visual development. This project can be deployed both on a server and on a Jetson NX device.

![在这里插入图片描述](https://img-blog.csdnimg.cn/direct/916ad9c098944687aeae2ad1e3c8acba.png)

# 2. How to Start

```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

```bash
# streming
./build/v8_pose_bytetrack_RTSPfile file:///your_vedio
# filesink
./build/v8_pose_bytetrack_sinkfile file:///your_vedio
```

![在这里插入图片描述](https://img-blog.csdnimg.cn/direct/57242095029940ab97cc05ced7acdd75.png)
