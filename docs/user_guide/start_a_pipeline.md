[English](start_a_pipeline_EN.md) | 简体中文
# 创建和运行第一个pipeline

创建一个新的pipeline通常只需要三步，运行这个pipeline只需要一行命令

## 创建pipeline
### 1.创建本地pipeline仓库
AdaFlow提供命令行工具`adaflow init`简单快捷地初始化pipeline仓库

```shell
# 此命令可以创建名为`my_pipelines`的仓库，同时仓库中pipelines文件夹下有包含`pipline.json`名为`foobar`任务的文件夹
# `my_pipelines`为仓库名，`foobar`为某个具体任务名
adaflow init my_pipelines foobar
```

### 2.创建pipeline描述文件`pipeline.json`

根据实际pipeline的任务填写`my_pipelines/foobar/pipeline.json`文件，本例子实现一个mp4视频的先解码再编码成新的mp4视频的流程，具体定义如下：

```
{
  "name": "foobar",
  "backend": "GStreamer",
  "dialect": [
    "{{F.source('src1')}}",
    "x264enc",
    "mp4mux",
    "{{F.sink('sink1')}}"
  ]
}
```

* `name` 这个pipeline具体实现的task名称
* `backend` 是不同处理后端的标识，这里选择`GStreamer`
* `dialect` 是pipeline处理的具体描述
    * ``{{`` 和 ```}}```包含的是运行时解析的插件，它会根据任务请求动态编译。`F.source` 和 `F.sink`分别是输入和输出插件的占位符函数
    * `x264enc`X264视频编码插件
    * `mp4mux`重新封装视频帧为MP4格式的插件

### 3.(*可选)编写扩展代码完成任务的其他操作
本例子不需要在pipeline中做其他操作，所以此步骤不需要，就完成了创建仓库的全部操作。  
这部分内容会在进阶教程中详细讲解，感兴趣的可以参考进阶教程。

## 运行pipeline

AdaFlow提供命令行工具`adaflow launch`启动pipeline

```shell
adaflow launch my_pipelines foobar --task `{"sources": [{"name": "src1", "type": "file", "location": "file.mp4"}], "sinks": [{"name": "sink1", "type": "gst", "element": "filesink", "properties": {"location": "output.mp4"}}]}`
```

* `--task` 参数接收JSON字符串来设置任务实际的输入输出视频文件信息
* 本例子中:
    * 处理`file.mp4`的视频源，可以是视频的路径
    * pipeline的输出结果保存为`output.mp4`