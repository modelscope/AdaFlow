#!/usr/bin/env bash
set -e
set -x

SCRIPT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" > /dev/null && pwd )"
PROJECT_ROOT=${SCRIPT_ROOT}/..

docker buildx build --push -t ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-devel-cpu:$(arch)-latest -f $PROJECT_ROOT/docker/adaflow-devel-cpu.dockerfile $PROJECT_ROOT

docker buildx build  --build-arg DEVEL_TAG=$(arch)-latest --pull --push  -t ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cpu:$(arch)-latest -f $PROJECT_ROOT/docker/adaflow-runtime-cpu.dockerfile $PROJECT_ROOT


docker buildx build --push -t ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-devel-cuda:latest -f $PROJECT_ROOT/docker/adaflow-devel-cuda.dockerfile $PROJECT_ROOT

docker buildx build --pull --push -t ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cuda:latest -f $PROJECT_ROOT/docker/adaflow-runtime-cuda.dockerfile $PROJECT_ROOT