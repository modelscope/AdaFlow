# docker buildx build --push -t ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-devel-cuda:latest -f ./docker/adaflow-devel-cuda.dockerfile .

# baseimage: nvidia/cuda:11.1.1-cudnn8-devel-centos7
ARG CUDA_VERSION=11.1.1
ARG OS_VERSION=7
FROM nvidia/cuda:${CUDA_VERSION}-cudnn8-devel-centos${OS_VERSION}

ARG GST_TAG=1.22.0
ARG ADAFLOW_PREFIX
ARG XST_BUILD_TYPE=Debug
ARG TRT_VERSION=7.2.3.4
ARG PYTHON_VERSION=3.7.16
ENV ADAFLOW_PREFIX=${ADAFLOW_PREFIX:-/xst-install}
ENV TRT_VERSION=${TRT_VERSION:-7.2.3.4}
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
ENV LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/lib64:/usr/lib:/usr/lib64:${ADAFLOW_PREFIX}/lib:${ADAFLOW_PREFIX}/lib64:/usr/local/cuda-11.1/targets/x86_64-linux/lib/:/usr/local/cuda-11.2/targets/x86_64-linux/lib/:/usr/local/cuda-11.2"
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
RUN wget -q https://viapi-test-bj.oss-accelerate.aliyuncs.com/github/nv-tensorrt-repo-rhel7-cuda11.1-trt7.2.3.4-ga-20210226-1-1.x86_64.rpm && \
    rpm -Uvh nv-tensorrt-repo-rhel7-cuda11.1-trt7.2.3.4-ga-20210226-1-1.x86_64.rpm
RUN yum install -y libnvinfer7 libnvparsers7 libnvonnxparsers7 libnvinfer-plugin7 \
    libnvinfer-devel-7.2.3 libnvparsers-devel-7.2.3 libnvonnxparsers-devel-7.2.3 libnvinfer-plugin-devel-7.2.3

# remove old packages and install deps for gst at best and gst will download other deps using meson
RUN yum remove -y python3 python3-devel &&  \
    yum remove -y gstreamer gstreamer-plugins-base gstreamer-devel gstreamer-plugins-bad-free gstreamer-plugins-bad gstreamer-plugins-good gstreamer-plugins-ugly gstreamer-python gstreamer-python-devel gstreamer-tools && \
    yum remove -y gstreamer1 gstreamer1-devel gstreamer1-plugins-bad-free gstreamer1-plugins-base gstreamer1-plugins-good gstreamer1-plugins-ugly gstreamer1-python && \
    yum install -y bison bison-devel flex flex-devel ninja-build bash-completion cairo-gobject cairo-gobject-devel cairo cairo-devel xz-devel && \
            yum clean all && \
    rm -rf /var/cache/yum/*

# install python to ADAFLOW_PREFIX:  --enable-shared is required for gst
RUN wget -q https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/Python-${PYTHON_VERSION}.tgz && \
    tar -xzf Python-${PYTHON_VERSION}.tgz && \
    cd Python-${PYTHON_VERSION} && \
    ./configure --enable-optimizations --enable-shared --prefix=${ADAFLOW_PREFIX} && \
    make install

# install meson
RUN pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple meson

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
RUN pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple pycairo pygobject

# install nasm for x264
RUN wget -q --no-check-certificate https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/nasm-2.15.05.tar.gz && \
    tar -zxf nasm-2.15.05.tar.gz && \
    cd nasm-2.15.05 && \
    ./configure --prefix=$ADAFLOW_PREFIX && \
    make -j${nproc} && \
    make install

# build gst
RUN wget -q https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/gstreamer-$GST_TAG.tar.gz && \
    tar -xzf gstreamer-$GST_TAG.tar.gz && \
    cd gstreamer-${GST_TAG} && \
    meson setup builddir -Dgpl=enabled -Dexamples=disabled -Dtests=disabled --prefix=$ADAFLOW_PREFIX && \
    meson compile -C builddir && \
    meson install -C builddir

# build tensorflow lite
#RUN wget -q https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/tensorflow-2.8.3.tar.gz && \
#    tar -xzf tensorflow-2.8.3.tar.gz && \
#    mkdir tflite_build && \
#    cd tflite_build && \
#    cmake -DCMAKE_INSTALL_PREFIX=$ADAFLOW_PREFIX \
#    -DTFLITE_ENABLE_INSTALL=ON \
#    -DBUILD_SHARED_LIBS=OFF \
#    -DCMAKE_BUILD_TYPE=$XST_BUILD_TYPE \
#    ../tensorflow-2.8.3/tensorflow/lite && \
#    cmake --build . -j$(nproc) && \
#    cp libtensorflow-lite.so $ADAFLOW_PREFIX/lib64 && \
#    mkdir -p $ADAFLOW_PREFIX/include/tensorflow && \
#    cp -r ../tensorflow-2.8.3/tensorflow/lite $ADAFLOW_PREFIX/include/tensorflow/ && \
#    cp -r ../tensorflow-2.8.3/tensorflow/core $ADAFLOW_PREFIX/include/tensorflow/ && \
#    cp -r ./flatbuffers/include/flatbuffers $ADAFLOW_PREFIX/include

# install common python packages
RUN pip3 install torch==1.9.0+cu111 torchvision==0.10.0+cu111 torchaudio==0.9.0 -f https://download.pytorch.org/whl/torch_stable.html && \
    pip3 install opencv-python==4.6.0.66 scipy==1.7.3 tensorflow==2.11.0

# build adaflow
ADD . /build/adaflow/
RUN --mount=type=cache,target=/build/adaflow/build cd xstreamer/build && \
    cmake \
        -DCMAKE_BUILD_TYPE=$XST_BUILD_TYPE \
        -DCMAKE_INSTALL_PREFIX=$ADAFLOW_PREFIX \
         .. && \
    make -j${nproc} && make install

