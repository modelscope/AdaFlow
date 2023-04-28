ARG CUDA_VERSION=11.6.2
ARG OS_VERSION=7
FROM nvidia/cuda:${CUDA_VERSION}-cudnn8-devel-centos${OS_VERSION}

ARG GST_TAG
ARG ADAFLOW_PREFIX
ARG ADAFLOW_BUILD_TYPE=Release
ARG TRT_VERSION
ARG PYTHON_VERSION
ENV ADAFLOW_PREFIX=${ADAFLOW_PREFIX:-/adaflow-install}
ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES video,compute,utility


# Install basic packages
RUN yum install -y centos-release-scl && \
    yum install -y devtoolset-9 && \
    echo "source /opt/rh/devtoolset-9/enable" >> /etc/bashrc && \
    yum install -y epel-release && \
    yum -y install openssl-devel bzip2-devel libffi-devel zlib zlib-devel wget git

WORKDIR /build

ENV BASH_ENV="/etc/bashrc"
ENV PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:/usr/lib64/pkgconfig:/usr/lib/pkgconfig:${ADAFLOW_PREFIX}/lib/pkgconfig:${ADAFLOW_PREFIX}/lib64/pkgconfig"
ENV GI_TYPELIB_PATH="${ADAFLOW_PREFIX}/lib64/girepository-1.0"
ENV LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/lib64:/usr/lib:/usr/lib64:${ADAFLOW_PREFIX}/lib:${ADAFLOW_PREFIX}/lib64:/usr/local/cuda/targets/x86_64-linux/lib/:/usr/local/cuda/targets/x86_64-linux/lib/:/usr/local/cuda/lib64"
ENV LDFLAGS="$LDFLAGS -L$ADAFLOW_PREFIX/lib -L$ADAFLOW_PREFIX/lib64"
ENV PATH=$PATH:${ADAFLOW_PREFIX}/bin
ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES video,compute,utility


SHELL ["/bin/bash", "--login", "-c"]

# install cmake
RUN wget -q https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/cmake-3.25.2.tar.gz && \
    tar -zxf cmake-3.* && \
    cd cmake-3.* && \
    ./bootstrap --prefix=/usr/local && \
    make -j$(nproc) && \
    make install


# Install TensorRT
ENV TRT_VERSION=${TRT_VERSION:-8.4.3.1}
RUN v="${TRT_VERSION%.*}-1.cuda${CUDA_VERSION%.*}" &&\
    yum-config-manager --add-repo https://developer.download.nvidia.com/compute/cuda/repos/rhel7/x86_64/cuda-rhel7.repo &&\
    yum -y install libnvinfer8-${v} libnvparsers8-${v} libnvonnxparsers8-${v} libnvinfer-plugin8-${v} \
        libnvinfer-devel-${v} libnvparsers-devel-${v} libnvonnxparsers-devel-${v} libnvinfer-plugin-devel-${v}

# remove old packages and install deps for gst at best and gst will download other deps using meson
RUN yum remove -y python3 python3-devel &&  \
    yum remove -y gstreamer gstreamer-plugins-base gstreamer-devel gstreamer-plugins-bad-free gstreamer-plugins-bad gstreamer-plugins-good gstreamer-plugins-ugly gstreamer-python gstreamer-python-devel gstreamer-tools && \
    yum remove -y gstreamer1 gstreamer1-devel gstreamer1-plugins-bad-free gstreamer1-plugins-base gstreamer1-plugins-good gstreamer1-plugins-ugly gstreamer1-python && \
    yum install -y bison bison-devel flex flex-devel ninja-build bash-completion cairo-gobject cairo-gobject-devel cairo cairo-devel xz-devel && \
            yum clean all && \
    rm -rf /var/cache/yum/*

# install python to ADAFLOW_PREFIX:  --enable-shared is required for gst
ENV PYTHON_VERSION=${PYTHON_VERSION:-3.7.16}
RUN wget -q https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/Python-${PYTHON_VERSION}.tgz && \
    tar -xzf Python-${PYTHON_VERSION}.tgz && \
    cd Python-${PYTHON_VERSION} && \
    ./configure --enable-optimizations --enable-shared --prefix=${ADAFLOW_PREFIX} && \
    make install

# install meson
RUN --mount=type=cache,target=/root/.cache/pip pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple meson

# update glib
RUN wget -q https://download.gnome.org/sources/glib/2.75/glib-2.75.2.tar.xz && \
    tar -xf glib-2.75.2.tar.xz && \
    cd glib-2.75.2 && \
    meson setup --prefix=$ADAFLOW_PREFIX builddir && \
    meson compile -C builddir && \
    meson install -C builddir

# remove glib2 in system
RUN rpm -e --nodeps glib2 glib2-devel && \
    echo "$ADAFLOW_PREFIX/lib" >> /etc/ld.so.conf && \
    echo "$ADAFLOW_PREFIX/lib64" >> /etc/ld.so.conf && \
    ldconfig

# update gobject-introspection
RUN wget -q https://download.gnome.org/sources/gobject-introspection/1.75/gobject-introspection-1.75.4.tar.xz && \
    tar -xf gobject-introspection-1.75.4.tar.xz && \
    cd gobject-introspection-1.75.4 && \
    meson setup --prefix=$ADAFLOW_PREFIX builddir && \
    meson compile -C builddir && \
    meson install -C builddir

# instlal extra python packages
RUN --mount=type=cache,target=/root/.cache/pip pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple pycairo pygobject

## install nasm for x264
RUN wget -q --no-check-certificate https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/nasm-2.15.05.tar.gz && \
    tar -zxf nasm-2.15.05.tar.gz && \
    cd nasm-2.15.05 && \
    ./configure --prefix=$ADAFLOW_PREFIX && \
    make -j${nproc} && \
    make install

# build x264
RUN wget https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/x264-stable.tar.gz && \
    tar -xzf x264-stable.tar.gz && \
    cd x264-stable && \
    ./configure --enable-shared --enable-static --prefix=$ADAFLOW_PREFIX \
    make -j${nproc} && make install

# build gst
ENV GST_VERSION=${GST_VERSION:-1.22.1}
RUN wget -q https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/gstreamer-$GST_VERSION.tar.gz && \
    tar -xzf gstreamer-$GST_VERSION.tar.gz && \
    cd gstreamer-* && \
    meson setup builddir -Dgpl=enabled -Dexamples=disabled -Dtests=disabled -Dges=disabled --prefix=$ADAFLOW_PREFIX && \
    meson compile -C builddir && \
    meson install -C builddir && \
    python3 -c 'import gi; gi.require_version("Gst", "1.0"); gi.require_version("GstApp", "1.0"); gi.require_version("GstVideo", "1.0"); from gi.repository import Gst, GLib, GObject, GstApp, GstVideo'


# hotfix for tf-trt: https://github.com/tensorflow/tensorflow/issues/57679
RUN --mount=type=cache,target=/root/.cache/pip pip3 install nvidia-pyindex && pip3 install nvidia-tensorrt==7.2.3.4
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${ADAFLOW_PREFIX}/lib/python3.7/site-packages/tensorrt

# install common python packages
RUN --mount=type=cache,target=/root/.cache/pip pip3 install torch==1.13.1+cu116 torchvision==0.14.1+cu116 torchaudio==0.13.1 --extra-index-url https://download.pytorch.org/whl/cu116 && \
    pip3 install opencv-python==4.6.0.66 scipy==1.7.3 tensorflow==2.11.0

# install modelscope
RUN --mount=type=cache,target=/root/.cache/pip SKLEARN_ALLOW_DEPRECATED_SKLEARN_PACKAGE_INSTALL=True pip3 install "modelscope[cv]" -f https://modelscope.oss-cn-beijing.aliyuncs.com/releases/repo.html

RUN --mount=type=cache,target=/root/.cache/pip python3 -m pip install --upgrade pip setuptools && \
     pip install -U openmim &&  \
     mim install mmcv-full
