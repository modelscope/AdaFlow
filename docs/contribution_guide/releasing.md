# Releasing {#dev_guide_release}

Main targets:

* AdaFlow Docker images, for online serving
* AdaFlow Conda packages, for binary release
* `adaflow-python` PIP packages, for Python binding

## Docker images

Supported Build args for CPU-only and CUDA image:

| Name           | Description                    | Default value | CPU | CUDA |
|----------------|--------------------------------|---------------|-----|------|
| XST_PREFIX     | Install prefix for XStreamer   | /usr/local    | Y   | Y    |
| XST_BUILD_TYPE | Cmake build type for XStreamer | DEBUG         | Y   | Y    |
| PYTHON_VERSION | Python version to install      | 3.8.14        | Y   | Y    |
| TRT_VERSION    | TensorRT version to install    | 8.4.3.1       | N   | Y    |
| CUDA_VERSION   | CUDA version to install        | 11.6.2        | N   | Y    |
| GST_TAG        | GStreamer version to install   | 1.20.3        | Y   | Y    |


### CPU-only image

Build the latest devel image for architecture of current host: 

```shell
docker buildx build --push -t ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-cpu-devel:$(arch)-latest -f ./docker/adaflow-cpu-devel.dockerfile .
```

### CUDA development image

Development image containing nVidia toolchains:

```shell
docker buildx build --push -t ivpd-registry.cn-hangzhou.cr.aliyuncs.com/xstreamer-dev/xstreamer-devel-cuda:latest -f ./docker/xstreamer-devel-cuda.dockerfile .
```

### CUDA runtime runtime image

```shell

```


### Python wheel

First of all, install toolchain for building Python wheel:

```shell
pip3 install setuptools build
```

To build a Python wheel

```shell
cd modules/xst-python/xstreamer-api
python -m build .
# `dist` folder should contain wheel like `xstreamer_api-0.0.1-py3-none-any.whl`
```

To test install a wheel package

```shell
pip3 install --force-reinstal dist/xstreamer_api-0.0.1-py3-none-any.whl
```

To uninstall the test wheel

```shell
pip3 uninstall xstreamer-api
```

### Conda packages

```shell
# use conda-forge
conda config --append channels conda-forge
```

To build a conda package from source root

```shell
# local build
conda build . -c conda-forge --no-include-recipe

# upload to remote
ossutil sync /root/miniconda3/envs/gst/conda-bld oss://viapi-test-bj/conda/xstreamer/
```


To test with a local build:

```shell
conda install xstreamer -c $HOME/miniconda3/envs/${ENV_NAME}/conda-bld
```

or

```shell
conda install xstreamer -c http://viapi-test-bj.oss-accelerate.aliyuncs.com/conda/xstreamer
```
