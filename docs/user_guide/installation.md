# Installation

Compatibility matrix for workstations and servers

| Platform     | Docker image                 | Conda  | Python binding |
|--------------|------------------------------|--------|----------------|
| Linux X86_64 | Y                            | Y      | Y              |
| Linux arm64  | NA                           | NA     | NA             |
| OSX X86_64   | WIP                          | WIP    | WIP            |
| OXS arm64    | WIP                          | WIP    | Y              |
| Windows WSL  | WIP                          | WIP    | WIP            |

* Y: supported right now
* NA: not available in near future
* WIP: working in progress

Support for device environment is possible but still under discussion.

## Using package manager

For Python and C++ developers, please install system packages for AdaFlow:

```shell
# TODO replace with conda-forge package after open-sourced
conda install adaflow -c https://viapi-test-bj.oss-accelerate.aliyuncs.com/conda/adaflow
```

and then install pip package:

```shell
python3 -m pip install adaflow
```

## Using Dokcer

Docker repositories are hosted on Dockerhub:  

* [adaflow/adaflow-runtime-cuda](https://hub.docker.com/repository/docker/adaflow/adaflow-runtime-cuda/general)
* [adaflow/adaflow-devel-cuda](https://hub.docker.com/repository/docker/adaflow/adaflow-devel-cuda/general)
* [adaflow/adaflow-runtime-cpu](https://hub.docker.com/repository/docker/adaflow/adaflow-runtime-cpu/general)
* [adaflow/adaflow-devel-cpu](https://hub.docker.com/repository/docker/adaflow/adaflow-devel-cpu/general)


Images repos and their tags can be found at [Docker images](./docker_images.md) page. For instance, if we are launching an interactive shell on a CUDA platform:

```shell
docker run -it --rm --gpus all adaflow/adaflow-devel-cuda /bin/bash
```