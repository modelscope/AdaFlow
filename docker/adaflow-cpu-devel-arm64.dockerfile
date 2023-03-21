# docker buildx build --push -t ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-cpu-devel-arm64:latest -f ./docker/adaflow-cpu-devel-arm64.dockerfile .

ARG OS_VERSION=7
FROM centos:centos${OS_VERSION}

ARG GST_VERSION
ARG PYTHON_VERSION
ARG ADAFLOW_PREFIX
ENV PYTHON_VERSION=${PYTHON_VERSION:-3.8.14}
ENV ADAFLOW_PREFIX=${ADAFLOW_PREFIX:-/adaflow}
ENV GST_VERSION=${GST_VERSION:-1.20.3}


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
ENV LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/lib64:/usr/lib:/usr/lib64:${ADAFLOW_PREFIX}/lib:${ADAFLOW_PREFIX}/lib64"
ENV PATH=$PATH:${ADAFLOW_PREFIX}/bin
ENV PYTHONPATH ${ADAFLOW_PREFIX}/lib/python3.8/site-packages/

SHELL ["/bin/bash", "--login", "-c"]

# install cmake
RUN wget -q https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/cmake-3.25.2.tar.gz && \
    tar -zxf cmake-3.* && \
    cd cmake-3.* && \
    ./bootstrap --prefix=/usr/local && \
    make -j$(nproc) && \
    make install


# remove old packages and install deps for gst at best and gst will download other deps using meson
RUN yum remove -y python3 python3-devel &&  \
    yum remove -y gstreamer gstreamer-plugins-base gstreamer-devel gstreamer-plugins-bad-free gstreamer-plugins-bad gstreamer-plugins-good gstreamer-plugins-ugly gstreamer-python gstreamer-python-devel gstreamer-tools && \
    yum remove -y gstreamer1 gstreamer1-devel gstreamer1-plugins-bad-free gstreamer1-plugins-base gstreamer1-plugins-good gstreamer1-plugins-ugly gstreamer1-python && \
    yum install -y bison bison-devel flex flex-devel ninja-build bash-completion cairo-gobject cairo-gobject-devel cairo cairo-devel xz-devel && \
            yum clean all && \
    rm -rf /var/cache/yum/*

# install python to ADAFLOW_PREFIX: --enable-shared is required for gst
RUN wget -q https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/Python-${PYTHON_VERSION}.tgz && \
    tar -xzf Python-${PYTHON_VERSION}.tgz && \
    cd Python-3.8.14 && \
    ./configure --enable-optimizations --enable-shared --prefix=${ADAFLOW_PREFIX} && \
    make install

# install meson
RUN pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple meson ninja

# update glib
RUN wget -q https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/glib-2.75.2.tar.xz && \
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
RUN wget -q https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/gobject-introspection-1.75.4.tar.xz && \
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
RUN wget -q https://viapi-test-bj.oss-cn-beijing.aliyuncs.com/github/gstreamer-$GST_VERSION.tar.gz && \
    tar -xzf gstreamer-$GST_VERSION.tar.gz && \
    cd gstreamer-${GST_VERSION} && \
    meson setup builddir -Dgpl=enabled -Dexamples=disabled -Dtests=disabled --prefix=$ADAFLOW_PREFIX && \
    meson compile -C builddir && \
    meson install -C builddir