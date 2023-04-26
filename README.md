[English](README_EN.md) | 简体中文

# **AdaFlow: Pipeline Frameworks for Deep-Learning Applications**


# 📘简介

AdaFlow是一个跨模态、跨平台的流式计算框架，它为NN模型推理、构建、部署提供了统一的解决方案

## ✨主要特性

- 灵活地插件化设计
  * 通过组合不同插件组件，用户可以便捷地编排自定义多模型推理pipeline
- 强大的音视频处理插件
  * 数百个可跨平台使用的[GStreamer* plugins](https://gstreamer.freedesktop.org/documentation/plugins_doc.html)，包含音视频编解码、各种分流合流等常用功能插件
- 丰富的模型 
  * 基于第一大模型库ModelScope，设置模型ID即可在pipeline里拉取、运行modelscope模型
- 简单易用
  * 开箱即用的Docker镜像和pipeline服务化，支持低代码JSON描述及运行pipeline



# ⚡️快速开始
## 🛠️环境配置
AdaFlow支持多种方式的环境配置，开发者可根据自己需求选择任意安装方式

### 1.docker镜像
AdaFlow提供了官方镜像，无需配置环境，轻松上手  
[docker镜像版本及地址](./docs/user_guide/docker_images.md)

### 2.安装AdaFlow包
使用[Conda](https://conda.io/)可安装完整的AdaFlow包

```
第一步：conda包安装
conda install adaflow
第二步：python依赖安装
python3 -m pip install adaflow-python
```
**当前的包只适合linux-x86_64平台.**

更详细的安装信息可参考[Installation](./docs/user_guide/installation.md)

### 3.源码安装
开发者通过源码编译安装AdaFlow及配置其运行环境  
[源码编译及环境配置](./docs/contribution_guide/build_from_source.md)

## ⏩主要功能快速体验
[通用目标检测模型及其结果可视化](./modules/adaflow-python/test/detection_repo/pipelines/real_detector/pipeline.json)

```shell
adaflow launch ./modules/adaflow-python/test/detection_repo real_detector --task_path ./modules/adaflow-python/test/detection_repo/task/real_detector/task.json 
```
<div align="center"><img src="./docs/user_guide/images/output.gif" width=900/></div>

## 📖开发指南

- 入门教程
  * [基础教程1:创建和运行第一个pipeline](docs/user_guide/tutorials/basic_tutorial_1.md)
  * [基础教程2:单模型pipeline搭建](docs/user_guide/tutorials/basic_tutorial_2.md)
  * [基础教程3:多模型并联pipeline搭建](docs/user_guide/tutorials/basic_tutorial_3.md)
  * [基础教程4:TensorRT模型部署](docs/user_guide/tutorials/basic_tutorial_4.md)
- 进阶教程
  * [AdaFlow插件详解](./docs/user_guide/built_in_elements.md)
  * [pipeline的构建](./docs/user_guide/composing_a_pipeline.md)
  * [CLI工具](./docs/user_guide/cli.md)
  * [编写扩展代码](./docs/user_guide/python_extension.md)
  * [pipeline和其他概念](./docs/user_guide/concept.md)
  * [pipeline服务化](./docs/user_guide/pipeline_server.md)


## 🙌贡献指南

### 技术路线

| Release train | Feature                                                      | Status       |
|---------------|--------------------------------------------------------------|--------------|
| 2023-04       | Native tensor support, TensorRT integration, Pipeline server | WIP          |
| 2023-05       | MNN integration and Android support                          | WIP          |
| 2023-06       | OpenXLA and large model deployment support                   | Under Review |

建议使用GitHub Issues来提出建议或者指出问题。

### 成为AdaFlow贡献者

我们感谢所有为了改进AdaFlow而做的贡献，也欢迎社区用户积极参与到本项目中来。请在提交PR之前阅读以下指南:

* [源码编译](docs/contribution_guide/build_from_source.md)
* [包发行](./docs/contribution_guide/releasing.md)
* [编码准则](./docs/contribution_guide/coding_guidelines.md)

## 📄License
本项目的发布受Apache 2.0 license许可认证。
