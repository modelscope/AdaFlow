# Installation {#install}

## Using package manager

For Python and C++ developers, please install system packages for AdaFlow:

```shell
# TODO
```

and then install pip package:

```shell
# TODO
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