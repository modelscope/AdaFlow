[English](README.md) | 简体中文

# **AdaFlow: Pipeline Frameworks for Deep-Learning Applications**


# 简介
AdaFlow是为深度学习应用而设计的流式框架。

亮点:
* 支持低代码描述及运行pipeline
* 基于第一大模型库
* 高度定制化的音视频处理组件
* 即用的pipeline服务化和Docker镜像

## 快速开始

使用[docker镜像](./docs/user_guide/docker_images.md)运行[通用目标检测模型及其结果可视化](./modules/adaflow-python/test/detection_repo/pipelines/real_detector/pipeline.json)：

```shell
docker run -it --rm -v $PWD/modules/adaflow-python/test/detection_repo:/detection_repo ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cpu:$(arch)-latest \
  adaflow launch /detection_repo real_detector --task_path /detection_repo/task/real_detector/task.json
```

根据[task.json](./modules/adaflow-python/test/detection_repo/task/real_detector/task.json)里参数设置，检测结果在原始视频上以彩色框的形式可视化，并且最终结果会编码为新的mp4视频文件，视频位置`modules/adaflow-python/test/detection_repo/resource/data/MOT17-03-partial_detector_vis.mp4`。

使用[Conda](https://conda.io/)可安装完整的AdaFlow包。

```
# TODO: replace with conda-forge and pip package after open-sourced 
conda install adaflow -c https://viapi-test-bj.oss-accelerate.aliyuncs.com/conda/adaflow
pip3 install https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/pip/adaflow/adaflow-0.0.1-py3-none-any.whl
```

**当前的包只适合linux-x86_64平台.**

更详细的安装信息可参考[Installation](./docs/user_guide/installation.md)。

## 开发者指南

* 快速开始
    * [Installation](./docs/user_guide/installation.md)
    * [Composing a Pipeline](./docs/user_guide/composing_a_pipeline.md)
    * [Official Docker images](./docs/user_guide/docker_images.md)
* 编写可复用的pipeline
    * [Built-in elements](./docs/user_guide/built_in_elements.md)
    * [Pipeline and other concepts](./docs/user_guide/concept.md)
* 生产场景下的pipelines运行
    * [Serve pipelines as REST services](./docs/user_guide/pipeline_server.md)
    * [Using command line interface](./docs/user_guide/cli.md)
* 扩展框架功能
    * [Python Extension](./docs/user_guide/python_extension.md)


## 贡献指南

### 技术路线

| Release train | Feature                                                      | Status       |
|---------------|--------------------------------------------------------------|--------------|
| 2023-04       | Native tensor support, TensorRT integration, Pipeline server | WIP          |
| 2023-05       | MNN integration and Android support                          | WIP          |
| 2023-06       | OpenXLA and large model deployment support                   | Under Review |

建议使用GitHub Issues来提出建议或者指出问题。

### 成为AdaFlow贡献者

请在提交PR之前阅读以下指南:

* [Building from source](docs/contribution_guide/build_from_source.md)
* [Pacakge releasing](./docs/contribution_guide/releasing.md)
* [Coding Guidelines](./docs/contribution_guide/coding_guidelines.md)