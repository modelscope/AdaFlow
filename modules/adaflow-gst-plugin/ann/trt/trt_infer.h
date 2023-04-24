#ifndef ADAFLOW_TRT_INFER_H
#define ADAFLOW_TRT_INFER_H

#include <assert.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <dirent.h>
#include <driver_types.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include<numeric>

#include "NvInfer.h"
#include "NvOnnxConfig.h"
#include "NvOnnxParser.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "NvInferPlugin.h"

using namespace nvinfer1;
using namespace nvonnxparser;
using namespace std;

class TrtInfer {
private:
    IRuntime* m_runtime;
    ICudaEngine* m_engine;
    IExecutionContext* m_context;
    bool m_create_success;

    cudaStream_t stream;

    int m_trt_infer_batch_size;

    unsigned int getElementSize(nvinfer1::DataType t);
    int64_t volume(const nvinfer1::Dims& d);

public:
    bool m_fp_16;
    bool m_flexible;
    bool m_nchw;
    int m_input_dims;
    int m_input_channel;
    int m_nbBindings;

    ///////////// input shape  /////////////
    int m_in_net_n;
    int m_in_net_c;
    int m_in_net_h;
    int m_in_net_w;
    int m_src_size;

    ///////////// output shape /////////////
    int m_out_net_n;
    int m_out_net_c;
    int m_out_net_h;
    int m_out_net_w;
    int m_dst_size;

    std::vector<int64_t> m_buffer_size;
    std::vector<const char*> m_buffer_name;
    std::vector<nvinfer1::Dims> m_buffer_shape;
    std::vector<nvinfer1::DataType> m_buffer_datatype;

    std::vector<void *> m_gpu_buffers;
    std::vector<void *> m_cpu_buffers;

    const char *m_input_name;

    string m_onnx_model_file;
    string m_trt_model_file;

    TrtInfer();
    ~TrtInfer();
    bool init_model(const char* onnx_model_file, const char* trt_model_file,
                    bool flexible, const char* model_input_name, int width, int height, float SR, int input_dims, bool is_nchw, int input_dim);

    void release();

    void run_model();
    void trt_onnx_2_tensorrt_model();

    ///////////// input shape  /////////////
    int input_batch_num() { return m_in_net_n; }
    int input_channel_num() { return m_in_net_c; }
    int input_height_num() { return m_in_net_h; }
    int input_width_num() { return m_in_net_w; }

    ///////////// output shape /////////////
    int output_batch_num() { return m_out_net_n; }
    int output_channel_num() { return m_out_net_c; }
    int output_height_num() { return m_out_net_h; }
    int output_width_num() { return m_out_net_w; }
};

#endif//ADAFLOW_TRT_INFER_H
