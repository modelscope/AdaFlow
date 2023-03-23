# docker buildx build --push -t ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-conda-build:latest -f ./docker/adaflow-conda-build.dockerfile .

# docker run --rm -v $PWD:/build ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-conda-build:latest

ARG OS_VERSION=7
FROM centos:centos${OS_VERSION}

ARG GST_VERSION
ARG PYTHON_VERSION
ENV PYTHON_VERSION=${PYTHON_VERSION:-3.8.14}
ENV GST_VERSION=${GST_VERSION:-1.20.3}


WORKDIR /build
RUN yum install -y wget && \
    yum clean all && \
    rm -rf /var/cache/yum/*

# setup conda
RUN wget -q https://repo.anaconda.com/miniconda/Miniconda3-py38_23.1.0-1-Linux-x86_64.sh \
        && mkdir $HOME/.conda \
        && bash Miniconda3-py38_23.1.0-1-Linux-x86_64.sh -b \
        && rm -f Miniconda3-py38_23.1.0-1-Linux-x86_64.sh

ENV PATH=/root/miniconda3/bin/:/root/miniconda3/envs/py37/bin:$PATH
ENV PKG_CONFIG_PATH=/root/miniconda3/envs/py37/lib/pkgconfig:$PKG_CONFIG_PATH
ENV LD_LIBRARY_PATH=/root/miniconda3/envs/py37/lib:$LD_LIBRARY_PATH
ENV GI_TYPELIB_PATH="/root/miniconda3/envs/py37/lib/girepository-1.0"
ENV LDFLAGS="$LDFLAGS -I/root/miniconda3/envs/py37/include -L/root/miniconda3/envs/py37/lib -Wl,-rpath,/root/miniconda3/envs/py37/lib"

RUN conda create --name dev conda-build && conda init bash && source ~/.bashrc && conda activate dev
SHELL ["conda", "run", "-n", "dev", "/bin/bash", "-c"]

WORKDIR /build
CMD conda build . -c conda-forge --no-include-recipe