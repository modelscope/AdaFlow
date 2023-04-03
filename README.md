English | [简体中文](README_zh.md)

# **AdaFlow: Pipeline Frameworks for Deep-Learning Applications**


# Introduction
AdaFlow is pipeline frameworks for deep-learning applications.
Highlight features:
* Low-code pipeline definitions and utilities
* First-class model hub support
* Highly customizable audio and video processing components
* Ready-to-use pipeline server and Docker images

## Quick start

Run [Object detection with visualization](./modules/adaflow-python/test/detection_repo/pipelines/real_detector/pipeline.json) using [docker image](./docs/user_guide/docker_images.md):

```shell
docker run -it --rm -v $PWD/modules/adaflow-python/test/detection_repo:/detection_repo ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cpu:$(arch)-latest \
  adaflow launch /detection_repo real_detector --task_path /detection_repo/task/real_detector/task.json
```

As requested in [task.json](./modules/adaflow-python/test/detection_repo/task/real_detector/task.json), detection results are visualized with colored bounding boxes drawn on original frames and encoded as a new MP4 file at `modules/adaflow-python/test/detection_repo/resource/data/MOT17-03-partial_detector_vis.mp4`.

Install full AdaFlow package using [Conda](https://conda.io/).

```
# TODO: replace with conda-forge after open-sourced 
conda install adaflow -c https://viapi-test-bj.oss-accelerate.aliyuncs.com/conda/adaflow
python3 -m pip install adaflow
```

**Current packages are only built for linux-x86_64 platform.**

To see more about installation, please refer to [Installation](./docs/user_guide/installation.md) in user guide. 


## Developer Guide

* Get started
  * [Installation](./docs/user_guide/installation.md)
  * [Composing a Pipeline](./docs/user_guide/composing_a_pipeline.md)
  * [Official Docker images](./docs/user_guide/docker_images.md)
* To write a re-usable pipeline
  * [Built-in elements](./docs/user_guide/built_in_elements.md)
  * [Pipeline and other concepts](./docs/user_guide/concept.md)
* To run pipelines in production
  * [Serve pipelines as REST services](./docs/user_guide/pipeline_server.md)
  * [Using command line interface](./docs/user_guide/cli.md)
* Extend framework capabilities
  * [Python Extension](./docs/user_guide/python_extension.md)


## Contribution Guide

### Roadmap

| Release train | Feature                                                      | Status       |
|---------------|--------------------------------------------------------------|--------------|
| 2023-04       | Native tensor support, TensorRT integration, Pipeline server | WIP          |
| 2023-05       | MNN integration and Android support                          | WIP          |
| 2023-06       | OpenXLA and large model deployment support                   | Under Review |

To request a feature or submit a bug, please use GitHub Issues.

### Contribute to AdaFlow

Please read following guidelines before submitting a PR:

* [Building from source](docs/contribution_guide/build_from_source.md)
* [Pacakge releasing](./docs/contribution_guide/releasing.md)
* [Coding Guidelines](./docs/contribution_guide/coding_guidelines.md)