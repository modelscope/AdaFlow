# Official Docker images

## Nightly images 

| REPO                                                                   | TAG           | Description                        |
|------------------------------------------------------------------------|---------------|------------------------------------|
| ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cpu  | arm64-latest  | Latest images for ARM64v8 platform |
| ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cpu  | x86_64-latest | Latest images for ARM64v8 platform |
| ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-devel-cpu    | x86_64-latest | Latest images for X86-64 platform  |
| ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-devel-cpu    | arm64-latest  | Latest images for ARM64v8 platform |
| ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-devel-cuda   | latest        | Latest images for X86-64 with CUDA |
| ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cuda | latest        | Latest images for X86-64 with CUDA |


## Stable images

### 2023-03-26

* Docker Image
    * ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-cpu-devel:arm64-20230326_1
    * ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-cpu-devel:arm64-20230326_1
    * ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-cuda-devel:x86_64-20230326_1
    * ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-cuda-runtime:x86_64-20230326_1
    * ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cuda:20230326_1
* Updates
    * System packages
      * Cuda only packages
          * Cuda 11.6.1
          * TensorRT 8.4.3.1
          * cudnn8
      * GStreamer 1.22.0
      * Python 3.7.16
    * Devel only system packages:
        * gcc 9.3.1
        * CMake 3.25.2
        * meson 1.0.0
        * ninja 1.10.2
    * Python PIP packages
        * torch 1.13.1
        * torchvision 1.13.1
        * torchaudio 1.13.1
        * opencv-python 4.6.0.66
        * scipy 1.7.3
        * tensorflow 2.11.0
        * modelscope 1.4.3