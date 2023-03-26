# docker buildx build --push -t ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cuda:latest -f ./docker/adaflow-runtime-cuda.dockerfile .

# baseimage: nvidia/cuda:11.1.1-cudnn8-runtime-centos7
ARG ADAFLOW_PREFIX
ARG DEVEL_TAG=latest
ARG CUDA_VERSION=11.1.1
ARG OS_VERSION=7
ARG TRT_VERSION

FROM ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-devel-cuda:$DEVEL_TAG as builder

# https://hub.docker.com/layers/nvidia/cuda/11.6.2-cudnn8-runtime-centos7/images/sha256-5be4a189316102a76e4d6549251dd80cc9754d149c7a6ffb88a6a943e083f4ee?context=explore
FROM nvidia/cuda:${CUDA_VERSION}-cudnn8-runtime-centos${OS_VERSION}
ENV TRT_VERSION=${TRT_VERSION:-7.2.3.4}
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

# Install TensorRT
ENV TRT_VERSION=8.4.3.1
ENV CUDA_VERSION=11.6.2
RUN v="${TRT_VERSION%.*}-1.cuda${CUDA_VERSION%.*}" &&\
    yum-config-manager --add-repo https://developer.download.nvidia.com/compute/cuda/repos/rhel7/x86_64/cuda-rhel7.repo &&\
    yum -y install libnvinfer8-${v} libnvparsers8-${v} libnvonnxparsers8-${v} libnvinfer-plugin8-${v} \
        libnvinfer-devel-${v} libnvparsers-devel-${v} libnvonnxparsers-devel-${v} libnvinfer-plugin-devel-${v}

# copy python3, gst and deps, xst and deps
COPY --from=builder $ADAFLOW_PREFIX $ADAFLOW_PREFIX

# setup env
ENV BASH_ENV="/etc/bashrc"
ENV PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:/usr/lib64/pkgconfig:/usr/lib/pkgconfig:${ADAFLOW_PREFIX}/lib/pkgconfig:${ADAFLOW_PREFIX}/lib64/pkgconfig"
ENV GI_TYPELIB_PATH="${ADAFLOW_PREFIX}/lib64/girepository-1.0"
ENV LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/lib64:/usr/lib:/usr/lib64:${ADAFLOW_PREFIX}/lib:${ADAFLOW_PREFIX}/lib64:/usr/local/cuda/lib64"
ENV PATH=$PATH:${ADAFLOW_PREFIX}/bin
ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES video,compute,utility
SHELL ["/bin/bash", "--login", "-c"]

# remove system glib2 and refresh ldconfig
RUN rpm -e --nodeps glib2 glib2-devel && \
    echo "ADAFLOW_PREFIX/lib" >> /etc/ld.so.conf && \
    echo "ADAFLOW_PREFIX/lib64" >> /etc/ld.so.conf && \
    ldconfig
