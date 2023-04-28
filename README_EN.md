English | [简体中文](README.md)

# **AdaFlow: Pipeline Frameworks for Deep-Learning Applications**

![Docker CI Tests](https://github.com/modelscope/AdaFlow/actions/workflows/dev.yml/badge.svg) ![Platforms](https://anaconda.org/conda-forge/adaflow/badges/platforms.svg) ![License](https://anaconda.org/conda-forge/adaflow/badges/license.svg)

# 📘Introduction
AdaFlow is cross-modal, cross-platform pipeline frameworks, which provides a unified solution for 
NN model inference, construction, and deployment.

## ✨Highlight Features:
- flexible plug-in design
  * users can easily arrange custom multiple neural network models pipelines by combining different plug-in components
- highly customizable audio and video processing plug-ins
  * hundreds other [GStreamer* plugins](https://gstreamer.freedesktop.org/documentation/plugins_doc.html) built on various open-source libraries for media input and output, muxing and demuxing, decode and encode
- ModelScope support
  * first-class model hub support, set the model ID to pull and run the modelscope model in the pipeline
- easy to use
  * low-code pipeline definitions and utilities, ready-to-use pipeline server and Docker images

# ⚡️Quick Start
## 🛠️ Environment Setup
AdaFlow supports multiple ways of environment setup, and developers can choose any installation method according to their needs.

### 1.Docker Images 
AdaFlow provides an official docker image, easy to get started.  
[Docker image version and address](./docs/user_guide/docker_images.md)

### 2.Install AdaFlow Package
Install full AdaFlow package using [Conda](https://conda.io/).
```
Step 1: conda package installation
conda install adaflow
Step 2: python dependency installation
python3 -m pip install adaflow-python
```
**Current packages are only built for linux-x86_64 platform.**

To see more about installation, please refer to [Installation](./docs/user_guide/installation.md) in user guide.

### 3.Build From Source
Developers can install AdaFlow through source code compilation.  
[Build from source](./docs/contribution_guide/build_from_source.md)

## ⏩Main Functions
Run [Object detection with visualization](./modules/adaflow-python/test/detection_repo/pipelines/real_detector/pipeline.json)

```shell
adaflow launch ./modules/adaflow-python/test/detection_repo real_detector --task_path ./modules/adaflow-python/test/detection_repo/task/real_detector/task.json 
```
<div align="center"><img src="./docs/user_guide/images/output.gif" width=900/></div>


## 📖Developer Guide

- Basic Tutorials
  * [Basic Tutorial 1:compose and run the first pipeline](docs/user_guide/tutorials/basic_tutorial_1_EN.md)
  * [Basic Tutorial 2:compose a simple single model pipeline](docs/user_guide/tutorials/basic_tutorial_2_EN.md)
  * [Basic Tutorial 3:compose a multi-model parallel pipeline](docs/user_guide/tutorials/basic_tutorial_3_EN.md)
  * [Basic Tutorial 4:model deployment based on TensorRT](docs/user_guide/tutorials/basic_tutorial_4_EN.md)
- Advanced Tutorials
  * [Built-in elements](./docs/user_guide/built_in_elements.md)
  * [Composing a Pipeline](./docs/user_guide/composing_a_pipeline.md)
  * [Using command line interface](./docs/user_guide/cli.md)
  * [Python Extension](./docs/user_guide/python_extension.md)
  * [Pipeline and other concepts](./docs/user_guide/concept.md)
  * [Serve pipelines as REST services](./docs/user_guide/pipeline_server.md)


## 🙌Contribution Guide

### Roadmap

| Release train | Feature                                                      | Status       |
|---------------|--------------------------------------------------------------|--------------|
| 2023-04       | Native tensor support, TensorRT integration, Pipeline server | WIP          |
| 2023-05       | MNN integration and Android support                          | WIP          |
| 2023-06       | OpenXLA and large model deployment support                   | Under Review |

To request a feature or submit a bug, please use GitHub Issues.

### Contribute to AdaFlow

All contributions are welcome to improve AdaFlow. Please read following guidelines before submitting a PR:

* [Building from source](docs/contribution_guide/build_from_source.md)
* [Pacakge releasing](./docs/contribution_guide/releasing.md)
* [Coding Guidelines](./docs/contribution_guide/coding_guidelines.md)

## 📄License
This project is licensed under the Apache License (Version 2.0).