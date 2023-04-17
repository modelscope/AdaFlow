[English](basic_tutorial_2_EN.md) | 简体中文
# 基础教程2:单模型pipeline搭建
本教程主要以检测套件为例讲解如何搭建单模型pipeline, 具体任务是实现视频的通用目标检测，要求在原视频上显示结果。

## 创建pipeline
### 1.创建本地pipeline仓库
创建`detection_repo`仓库，任务的文件夹为`real_detector`
```shell
cd ./modules/adaflow-python/test/

adaflow init detection_repo --pipeline real_detector
```

### 2.创建pipeline描述文件`pipeline.json`

根据通用目标检测的pipeline的任务填写`detection_repo/pipelines/real_detector/pipeline.json`文件：

![pipeline结构图](./images/tu2_dsl.jpg)

```
{
    "name": "real_detector",
    "description": "visualization after realtime detector",
    "backend": "GStreamer",
    "dialect": [
      "{{F.source('src1')}}",
      "videoconvert",
      "videoscale",
      "video/x-raw,format=RGB",
      "flow_modelscope_pipeline task=image-object-detection id = damo/cv_cspnet_image-object-detection_yolox meta-key=detection",
      "flow_python_extension input=./detection_repo/resource/config/real_detector.yaml module=./detection_repo/extension/real_detector_vis.py class= RealDetectorPost function= postprocess",
      "videoconvert",
      "videoscale",
      "video/x-raw,format=I420",
      "x264enc qp-max=15",
      "mp4mux",
      "{{F.sink('sink1')}}"
    ]
  }
```

* `name` 这个pipeline具体实现的task名称
* `backend` 是不同处理后端的标识，这里选择`GStreamer`
* `dialect` 是pipeline处理的具体描述
    * ``{{`` 和 ```}}```包含的是运行时解析的插件，它会根据任务请求动态编译。`F.source` 和 `F.sink`分别是输入和输出插件的占位符函数
    * `videoconvert`在多种视频格式之间转换视频帧，通常搭配`videoscale`使用
    * `videoscale` `video/x-raw,format=RGB`视频尺寸、颜色空间等转换插件，`format=RGB`将视频帧转换为RGB颜色空间传给模型推理
    * `flow_modelscope_pipeline`运行modelscope上的模型，`task`和`id`指定模型，`meta-key=detection`设置该模型的检测结果的传递即下游节点获取该结果的关键词为`detection`
    * `flow_python_extension`对modelscope模型的推理结果做自定义的后处理，`input`设置后处理的外置参数文件，`module`自定义python后处理
函数py文件所在的位置，`class`函数类名，`function`具体函数实现名，具体`real_detector_vis.py`内容会在下一小节介绍
    * `videoconvert` `videoscale` `video/x-raw,format=I420`将视频转换为`format=I420`传输保存
    * `x264enc`X264视频编码插件，参数`qp-max=15`指定编码过程中最大的`qp`为15,`qp-max`越小，压缩越小，视频质量越好
    * `mp4mux`重新封装视频帧为MP4格式的插件

### 3.编写扩展代码完成任务的其他操作
AdaFlow提供插件`flow_python_extension`调用用户自定义的后处理函数，以检测显示为例讲解具体使用方法，先创建自定义后处理的python文件
detection_repo/extension/real_detector_vis.py：

```bash
from adaflow.av.data.av_data_packet import AVDataPacket
```
adaflow-python api `AVDataPacket`，这个是视频帧类，包含了视频帧的一些基本信息，例如视频帧的像素值、长宽高等

```bash
import cv2
...
```
引入自定义函数需要的第三方库

```bash
class RealDetectorPost:
    def postprocess(self, frames: AVDataPacket, kwargs):
```
声明Python的类名和函数名，函数参数是`frames`和`kwargs`，其中`kwargs`是`flow_python_extension`通过`input`传入的yaml文件而解析的用户自定义
函数的外置参数，本例子的yaml文件为detection_repo/resource/config/real_detector.yaml，具体内容如下：
```bash
color:
  [255, 0, 0]
```
函数中获取该参数的代码：
```bash
self.color = kwargs['color']
```
即检测结果显示的矩形框的颜色为红色

```bash
for frame in frames:
```
逐帧处理，默认情况下是单帧处理，也支持模型多帧输出

```bash
self.meta_data = frame.get_json_meta('detection')
self.image = frame.data()
scores = self.meta_data['scores']
boxes = self.meta_data['boxes']
labels = self.meta_data['labels']
```
获取推理结果和帧像素值，`detection`是在`flow_modelscope_pipeline`插件的参数 `meta-key=detection`决定的，用户可根据自己需求设定，保持前后一致即可

```bash
for idx in range(len(scores)):
    x1, y1, x2, y2 = boxes[idx]
    score = str(scores[idx])
    label = str(labels[idx])
    cv2.rectangle(self.image, (int(x1), int(y1)), (int(x2), int(y2)), self.color, 2)
    cv2.putText(self.image, label, (int(x1), int(y1) - 10),
    cv2.FONT_HERSHEY_PLAIN, 1, self.color)
    cv2.putText(self.image, score, (int(x1), int(y2) + 10),
    cv2.FONT_HERSHEY_PLAIN, 1, self.color)
```
该函数具体实现的函数，将检测的结果在原始视频帧上画框和标注对应的标签

## 运行pipeline

最后使用AdaFlow的命令行工具`adaflow launch`启动pipeline，具体处理的视频源的输入和输出由用户指定。
- 方式1： 可通过detection_repo/task/real_detector/task.json设置
```bash
{
  "sources": [{"name": "src1", "type": "file", "location": "./detection_repo/resource/data/MOT17-03-partial.mp4"}],
  "sinks": [{ "name": "sink1", "type": "file", "location": "./detection_repo/resource/data/MOT17-03-partial_detector_vis.mp4"}]
}
```
输入视频：./detection_repo/resource/data/MOT17-03-partial.mp4  
输出视频：./detection_repo/resource/data/MOT17-03-partial_detector_vis.mp4

运行整个pipeline:

```bash
adaflow launch detection_repo real_detector --task_path ./detection_repo/task/real_detector/task.json 
```

- 方式2：JSON字符串来设置任务实际的输入输出视频文件信息
```bash
adaflow launch detection_repo real_detector --task '{"sources": [{"name": "src1", "type": "file", "location": "./detection_repo/resource/data/MOT17-03-partial.mp4"}], "sinks": [{ "name": "sink1", "type": "file", "location": "./detection_repo/resource/data/MOT17-03-partial_detector_vis.mp4"}]}' 
```

> **本章结束，感谢阅览**




