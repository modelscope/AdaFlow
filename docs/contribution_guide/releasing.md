# Releasing

Main targets:

* AdaFlow Docker images, for online serving
* AdaFlow Conda packages, for binary release
* `adaflow-python` PIP packages, for Python binding

## Docker images

Supported Build args for CPU-only and CUDA image:

| Name               | Description                  | Default value    | CPU | CUDA |
|--------------------|------------------------------|------------------|-----|------|
| ADAFLOW_PREFIX     | Install prefix for AdaFlow   | /adaflow-install | Y   | Y    |
| ADAFLOW_BUILD_TYPE | Cmake build type for AdaFlow | RELEASE          | Y   | Y    |
| PYTHON_VERSION     | Python version to install    | 3.7.14           | Y   | Y    |
| TRT_VERSION        | TensorRT version to install  | 8.4.3.1          | N   | Y    |
| CUDA_VERSION       | CUDA version to install      | 11.6.2           | N   | Y    |
| GST_VERSION        | GStreamer version to install | 1.21.1           | Y   | Y    |


### CPU-only image

Build the latest devel image for architecture of current host: 

```shell
docker buildx build --push -t adaflow/adaflow-devel-cpu:$(arch)-latest -f ./docker/adaflow-devel-cpu.dockerfile .
```

Build the latest runtime image for architecture of current host:

```shell
docker buildx build --pull --build-arg DEVEL_TAG=$(arch)-latest --push -t adaflow/adaflow-runtime-cpu:$(arch)-latest -f ./docker/adaflow-runtime-cpu.dockerfile .
```


### CUDA-enabled image

Build development image for CUDA-enabled X86_64 machines:

```shell
docker buildx build --push -t adaflow/adaflow-devel-cuda:latest -f ./docker/adaflow-devel-cuda.dockerfile .
```

Build runtime image for CUDA-enabled X86_64 machines:

```shell
docker buildx build --push -t adaflow/adaflow-runtime-cuda:latest -f ./docker/adaflow-runtime-cuda.dockerfile .
```


### Python wheel

First of all, install toolchain for building Python wheel:

```shell
pip3 install -U setuptools build
```

To build a Python wheel

```shell
cd modules/adaflow-python
python -m build .
# `dist` folder should contain wheel like `adaflow-python-0.0.1-py3-none-any.whl`
```

To test install a wheel package

```shell
pip3 install --force-reinstal dist/adaflow-python-0.0.1-py3-none-any.whl
```

To uninstall the test wheel

```shell
pip3 uninstall adaflow-python
```

### Conda packages

TODO move to conda-forge recipe after this project is open-sourced. 

```shell
# use conda-forge
conda config --append channels conda-forge
```

To build a conda package from source root

```shell
# local build
conda build . -c conda-forge --no-include-recipe
```

To test with a local build:

```shell
conda install adaflow -c $HOME/miniconda3/envs/${ENV_NAME}/conda-bld
```
