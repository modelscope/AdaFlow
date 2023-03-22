# Official Docker images

## Nightly images 

| REPO                                                                   | TAG           | Description                        |
|------------------------------------------------------------------------|---------------|------------------------------------|
| ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-cpu-devel    | arm64-latest  | Latest images for ARM64v8 platform |
| ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-cpu-devel    | x86_64-latest | Latest images for X86-64 platform  |
| ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cuda | latest        | Latest images for X86-64 with CUDA |


## Stable images

### 2023-03-22

* Docker Image
    * ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-cpu-devel:arm64-20230223_1
    * ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-cpu-devel:x86_64-20230223_1
    * ivpd-registry.cn-hangzhou.cr.aliyuncs.com/adaflow/adaflow-runtime-cuda:20230223_1
* Updates
    * System packages
        * Cuda 11.1.1
        * TensorRT 7.2.3.4
        * GStreamer 1.22.0
        * Python 3.7.16
    * Devel only system packages:
        * gcc 9.3.1
        * CMake 3.25.2
        * meson 1.0.0
        * ninja 1.10.2
    * Python PIP packages
        * torch 1.9.0
        * torchvision 0.10.0
        * torchaudio 0.9.0
        * opencv-python 4.6.0.66
        * scipy 1.7.3
        * tensorflow 2.11.0