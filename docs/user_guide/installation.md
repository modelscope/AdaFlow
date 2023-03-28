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
# TODO replace with pypi package after open-sourced
pip3 install https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/pip/adaflow/adaflow-0.0.1-py3-none-any.whl
```

## Using Dokcer

Base images are available for cloud environment. Login credentials are required for image registry.

### Credentials for internal registry

TODO: submit to docker hub and remove this section

**Password will be reset every month. Please do not use this credential in production!**

Read-only account for our registry:

* Username: `ali_cr_internal_readonly@aliyun-dha`
* Password: `gWXPChfZWqVZ64`
* Endpoint: ivpd-registry-vpc.cn-hangzhou.cr.aliyuncs.com

Please log in first using following command.

```shell
docker login --username=ali_cr_internal_readonly@aliyun-dha ivpd-registry-vpc.cn-hangzhou.cr.aliyuncs.com
```

Images repos and their tags can be found at [Docker images](./docker_images.md) page. For instance, if we are launching an interactive shell on a CUDA platform:


```shell
docker run -it --rm --gpus all ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-devel-cuda /bin/bash
```