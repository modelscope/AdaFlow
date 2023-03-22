# AdaFlow

Pipeline frameworks for deep-learning applications.

Highlight features:

* Low-code pipeline definitions and utilities
* First-class model hub support
* Highly customizable audio and video processing components
* Ready-to-use pipeline server and Docker images


## Quick start
 
Install full AdaFlow package using [Conda](https://conda.io/).

```
conda install 
# TODO: replace with official conda-forge package
```

To see more about installation, please refer to [Installation](./docs/user_guide/installation.md) in user guide. 

Run video segmentation task:

```shell
# TODO 
```


## Developer Guide

* Get started
  * [Installation](./docs/user_guide/installation.md)
  * [Pipeline and other concepts](./docs/user_guide/concept.md)
  * [Official Docker images](./docs/user_guide/docker_images.md)
* To write a re-usable pipeline
  * [Composing a Pipeline](./docs/user_guide/composing_a_pipeline.md)
  * [Built-in elements](./docs/user_guide/built_in_elements.md)
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