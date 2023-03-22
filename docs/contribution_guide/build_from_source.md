# Envrionment setup

As major dependencies of `AdaFlow` contain GStreamer and its toolchains, it's recommended developers to employ `Conda` to ease the process by using pre-built packages on workstation setup like local desktops, Jupyter Notebook instances. Please refer to [conda.io](https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html) for installation guide for Conda.

For linux-like environment, Docker images for development are available for testing and production purpose.

## Pre-requisites

* Python >=3.8, <=3.10
* CMake >=3.25.2
* GCC 9+
* Operating System
   * Linux-like: CentOS 7, Ubuntu 20.04
   * MacOS 13.2.1+
   * Windows is not supported. However, running in WSL is possible. 
   * Android and iOS support is coming soon.

## GStreamer setup

* For Linux-like systems and Intel Macs, prebuilt packages are recommended.
* For Apple-silicon macs, at the moment of writing, you have to build GStreamer from source due to upstream issues ([conda-forge/gst-plugins-bad-feedstock](https://github.com/conda-forge/gst-plugins-bad-feedstock/issues/9)). 

### Install prebuilt GStreamer packages

```shell
conda create -n adaflow-dev -c conda-forge adaflow-dev python=3.10 gstreamer gst-plugins-base gst-plugins-good gst-plugins-ugly gst-plugins-bad
```


Remember to activate newly created conda env:

```shell
conda activate adaflow-dev
```

### Build GStreamer from source

For `Apple sillicon` macs, `gst-plugins-bad` is not available on .  We have to build GStreamer from source.

```shell
# if cmake is already installed, you can remove cmake in the following line
conda create -y -n adaflow-dev -c conda-forge python=3.10 gobject-introspection pkg-config cmake
# install python toolchains
pip3 install pycairo pygobject meson ninja
# checkout source code
git clone -b 1.22.0 git@github.com:GStreamer/gstreamer.git && cd gstreamer
# configure, compile and install. (this will take a while)
meson setup builddir -Dgpl=enabled -Dexamples=disabled -Dtests=disabled --prefix=$CONDA_PREFIX && \
    meson compile -C builddir && \
    meson install -C builddir
```

## Build AdaFlow from source

Assuming you are on project root and `adaflow-dev` env is activated, you can make a debug configuration as following: 

```shell
# Configure as debug build
mkdir build && cd build
cmake \
      -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX \
      -DCMAKE_BUILD_TYPE=Debug

# make and install
make -j${nproc}
make install
```

## Install an editable copy of `adaflow-python`

Assuming you are on project root, you can install `adaflow-python` package as an editable copy.

```shell
cd modules/adaflow-python
pip install -e .
```