# To build this image
# docker buildx build --pull --build-arg DEVEL_TAG=$(arch)-latest --push -t ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cpu:$(arch)-latest -f ./docker/adaflow-runtime-cpu.dockerfile .

# to run this image with interactive shell
# docker run -it --rm ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cpu:$(arch)-latest /bin/bash

ARG ADAFLOW_PREFIX
ARG DEVEL_TAG
ARG OS_VERSION=7

FROM ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-devel-cpu:$DEVEL_TAG as builder

FROM centos:centos${OS_VERSION}
ENV ADAFLOW_PREFIX=${ADAFLOW_PREFIX:-/adaflow-install}

# install basic softwares
RUN yum remove -y python3 python3-devel &&  \
        yum remove -y gstreamer gstreamer-plugins-base gstreamer-devel gstreamer-plugins-bad-free gstreamer-plugins-bad gstreamer-plugins-good gstreamer-plugins-ugly gstreamer-python gstreamer-python-devel gstreamer-tools && \
        yum remove -y gstreamer1 gstreamer1-devel gstreamer1-plugins-bad-free gstreamer1-plugins-base gstreamer1-plugins-good gstreamer1-plugins-ugly gstreamer1-python && \
    yum install -y  \
    # python deps
    openssl-devel bzip2-devel libffi-devel zlib zlib-devel wget git  \
#    glib deps
    bison bison-devel flex flex-devel ninja-build bash-completion cairo-gobject cairo-gobject-devel cairo cairo-devel xz-devel && \
    yum clean all && \
    rm -rf /var/cache/yum/*

# copy python3, gst and deps, xst and deps
COPY --from=builder $ADAFLOW_PREFIX $ADAFLOW_PREFIX

# setup env
ENV BASH_ENV="/etc/bashrc"
ENV PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:/usr/lib64/pkgconfig:/usr/lib/pkgconfig:${ADAFLOW_PREFIX}/lib/pkgconfig:${ADAFLOW_PREFIX}/lib64/pkgconfig"
ENV GI_TYPELIB_PATH="${ADAFLOW_PREFIX}/lib64/girepository-1.0"
ENV LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/lib64:/usr/lib:/usr/lib64:${ADAFLOW_PREFIX}/lib:${ADAFLOW_PREFIX}/lib64:/usr/local/cuda/lib64"
ENV PATH=$PATH:${ADAFLOW_PREFIX}/bin
SHELL ["/bin/bash", "--login", "-c"]

# remove system glib2 and refresh ldconfig
RUN rpm -e --nodeps glib2 glib2-devel && \
    echo "ADAFLOW_PREFIX/lib" >> /etc/ld.so.conf && \
    echo "ADAFLOW_PREFIX/lib64" >> /etc/ld.so.conf && \
    ldconfig
