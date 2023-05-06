#include "trt_infer.h"

#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

class Logger : public ILogger {
    void log(Severity severity, nvinfer1::AsciiChar const* msg) noexcept {
        // suppress info-level messages
        if (severity != Severity::kINFO) std::cout << msg << std::endl;
    }
} gLogger;

inline unsigned int TrtInfer::getElementSize(nvinfer1::DataType t)
{
    switch (t)
    {
        case nvinfer1::DataType::kINT32:
            return 4;
        case nvinfer1::DataType::kFLOAT:
            return 4;
        case nvinfer1::DataType::kHALF:
            return 2;
        case nvinfer1::DataType::kINT8:
            return 1;
    }
    throw std::runtime_error("Invalid DataType.");
    return 0;
}

inline int64_t TrtInfer::volume(const nvinfer1::Dims &d)
{
    return std::accumulate(d.d, d.d + d.nbDims, 1, std::multiplies<int64_t>());
}


TrtInfer::TrtInfer() {
    // init
    m_fp_16 = true;
    m_create_success = false;
    m_nchw = true;
    m_trt_infer_batch_size = 1;

    m_runtime = NULL;
    m_engine = NULL;
    m_context = NULL;

    cudaStreamCreateWithFlags(
            &stream,
            cudaStreamNonBlocking);  // cudaStreamDefault/cudaStreamNonBlocking

    m_create_success = true;
};

bool TrtInfer::init_model(const char* onnx_model_file,
                          const char* trt_model_file, bool flexible, const char* model_input_name,
                          int width, int height, float SR, int input_dims, bool is_nchw, int input_channel)
{
    m_onnx_model_file = onnx_model_file;
    m_trt_model_file = trt_model_file;
    m_flexible = flexible;
    m_input_dims = input_dims;
    m_nchw = is_nchw;
    m_input_name = model_input_name;
    m_input_channel = input_channel;
    ////////////////////////load trt model////////////////////////
    /*成功执行时，返回0。失败返回-1，errno被设为以下的某个值
   * mode: 0 （F_OK） 只判断是否存在
   *      2 （R_OK） 判断写入权限
   *      4 （W_OK） 判断读取权限
   *      6 （X_OK） 判断执行权限
   */
    if (((access(trt_model_file, F_OK)) == -1)) {
        trt_onnx_2_tensorrt_model();
        printf("onnx to trt success !! \n");
    }

    if ((access(trt_model_file, R_OK) == -1)) {
        printf("file: %s have read permission.\n", trt_model_file);
        return 0;
    }

    if (!m_context) {
        size_t size{0};
        std::vector<char> trtModelstream_;
        std::cout << "load and run trt engine" << std::endl;
        // 加载trt模型
        std::ifstream input_engine_file(m_trt_model_file.c_str(), std::ios::binary);
        if (input_engine_file.good()) {
            input_engine_file.seekg(0, input_engine_file.end);  // 到末尾
            size = input_engine_file.tellg();                   // 字节数
            input_engine_file.seekg(0, input_engine_file.beg);  // 到开始
            trtModelstream_.resize(size);
            std::cout << "engine size " << trtModelstream_.size() << std::endl;
            input_engine_file.read(trtModelstream_.data(), size);
            input_engine_file.close();
        }
        m_runtime = createInferRuntime(gLogger);

        // 从字符串创建推理引擎
        bool didInitPlugins = initLibNvInferPlugins(nullptr, "");
        m_engine =
                m_runtime->deserializeCudaEngine(trtModelstream_.data(), size, nullptr);
        m_context = m_engine->createExecutionContext();

        if (!m_context) {
            printf("trt_create_context failed!\n");
            return 0;
        } else {
            printf("trt_create_context success!\n");
        }

        if (!m_engine || !m_runtime || !m_context) {
            printf("TrtEngine::create trt engine failed!\n");
            return 0;
        }

        printf("TrtEngine::tensorrt_engine create success!\n");
    }
    ////////////////////////load trt model////////////////////////
    //Setup I/O bindings
    m_nbBindings = m_engine->getNbBindings();

    m_gpu_buffers.resize(m_nbBindings);
    m_cpu_buffers.resize(m_nbBindings);
    m_buffer_shape.resize(m_nbBindings);
    m_buffer_datatype.resize(m_nbBindings);
    m_buffer_size.resize(m_nbBindings);
    m_buffer_name.resize(m_nbBindings);

    for (int i = 0; i < m_nbBindings; i++)
    {
        nvinfer1::Dims dims = m_engine->getBindingDimensions(i);
        m_buffer_shape[i] = dims;
        nvinfer1::DataType dtype = m_engine->getBindingDataType(i);
        m_buffer_datatype[i] = dtype;
        const char* name  = m_engine->getBindingName(i);
        m_buffer_name[i] = name;
        int64_t total_size = volume(dims) * 1 * getElementSize(dtype);
        m_buffer_size[i] = total_size;
    }

    ////////////////////////get model shape////////////////////////
    if(m_nchw)
    {
        m_in_net_n = m_buffer_shape[0].d[0];
        m_in_net_c = m_buffer_shape[0].d[1];
        m_in_net_h = m_buffer_shape[0].d[2];
        m_in_net_w = m_buffer_shape[0].d[3];

        if(m_nbBindings==2 && SR!=1)
        {
            m_out_net_n = m_buffer_shape[1].d[0];
            m_out_net_c = m_buffer_shape[1].d[1];
            m_out_net_h = m_buffer_shape[1].d[2];
            m_out_net_w = m_buffer_shape[1].d[3];
        }else
        {
            m_out_net_n = m_in_net_n;
            m_out_net_c = m_in_net_c;
            m_out_net_h = m_in_net_h;
            m_out_net_w = m_in_net_w;
        }


    }else{
        m_in_net_n = m_buffer_shape[0].d[0];
        m_in_net_h = m_buffer_shape[0].d[1];
        m_in_net_w = m_buffer_shape[0].d[2];
        m_in_net_c = m_buffer_shape[0].d[3];

        if(m_nbBindings==2 && SR!=1)
        {
            m_out_net_n = m_buffer_shape[1].d[0];
            m_out_net_h = m_buffer_shape[1].d[1];
            m_out_net_w = m_buffer_shape[1].d[2];
            m_out_net_c = m_buffer_shape[1].d[3];
        }else
        {
            m_out_net_n = m_in_net_n;
            m_out_net_c = m_in_net_c;
            m_out_net_h = m_in_net_h;
            m_out_net_w = m_in_net_w;
        }

    }

    if (m_flexible) {
        m_in_net_h = height;
        m_in_net_w = width;

        m_out_net_h = int(height * SR);
        m_out_net_w = int(width * SR);

    }

    ////////////////////////malloc memory////////////////////////
    for (int i = 0; i < m_nbBindings; i++)
    {
        if(m_flexible && i==0)
        {
            m_buffer_size[i] = m_in_net_n * m_in_net_c * m_in_net_h * m_in_net_w * getElementSize(m_buffer_datatype[i]);
            cudaMalloc(&m_gpu_buffers[i], m_buffer_size[i]);
            cudaMallocHost(&m_cpu_buffers[i], m_buffer_size[i]);

        }else if(m_flexible && m_nbBindings==2 && i==1 ){
            m_buffer_size[i] = m_out_net_n * m_out_net_c * m_out_net_h * m_out_net_w * getElementSize(m_buffer_datatype[i]);
            cudaMalloc(&m_gpu_buffers[i], m_buffer_size[i]);
            cudaMallocHost(&m_cpu_buffers[i], m_buffer_size[i]);

        }else{
            cudaMalloc(&m_gpu_buffers[i], m_buffer_size[i]);
            cudaMallocHost(&m_cpu_buffers[i], m_buffer_size[i]);
        }

    }

    return 1;
};

TrtInfer::~TrtInfer() { release(); };

void TrtInfer::release() {
    if (stream) {
        cudaStreamDestroy(stream);
    }

    if (m_context) {
        m_context->destroy();
        m_context = NULL;
    }

    if (m_engine) {
        m_engine->destroy();
        m_engine = NULL;
    }

    if (m_runtime) {
        m_runtime->destroy();
        m_runtime = NULL;
    }

    for(int i=0; i < m_nbBindings; i++)
    {
        if(m_gpu_buffers[i])
        {
            cudaFree(m_gpu_buffers[i]);
            m_gpu_buffers[i] = NULL;
        }

        if(m_cpu_buffers[i])
        {
            cudaFreeHost(m_cpu_buffers[i]);
            m_cpu_buffers[i] = NULL;

        }
    }
}

/**
 * onnxmodel->trtmodel
 * */
void TrtInfer::trt_onnx_2_tensorrt_model() {
    const char* onnxmodelfile = m_onnx_model_file.c_str();
    const char* trtmodelfile = m_trt_model_file.c_str();

    unsigned int maxBatchSize = 1;
    const auto explicitBatch =
            1U << static_cast<uint32_t>(
                    NetworkDefinitionCreationFlag::kEXPLICIT_BATCH);

    std::cout << "Build and save trt engine" << std::endl;
    printf("onnx->trt:%s -> %s \n", onnxmodelfile, trtmodelfile);
    IBuilder* builder = createInferBuilder(gLogger);
    INetworkDefinition* network = builder->createNetworkV2(explicitBatch);
    nvonnxparser::IParser* parser = nvonnxparser::createParser(*network, gLogger);

    // 加载onnx模型
    parser->parseFromFile(onnxmodelfile,
                          static_cast<int>(Logger::Severity::kWARNING));
    for (int i = 0; i < parser->getNbErrors(); ++i) {
        std::cout << parser->getError(i)->desc() << std::endl;
        break;
    }
    std::cout << "successfully load the onnx model" << std::endl;

    // 设置batchsize大小
    builder->setMaxBatchSize(maxBatchSize);
    IBuilderConfig* config = builder->createBuilderConfig();
    // 设置workspace大小
    config->setMaxWorkspaceSize(1 << 30);

    // 设置推理的数据类型fp16或者int8
    if (m_fp_16) {
        if (!builder->platformHasFastFp16()) {
            std::cout << "Notice: the platform do not has fast for fp16" << std::endl;
        } else {
            config->setFlag(BuilderFlag::kFP16);  // kINT8, kFP16
            // builder->setFp16Mode(true);
            std::cout << "Notice: build engine for fp16" << std::endl;
        }
    }

    // 动态尺寸
    if (m_flexible) {
        IOptimizationProfile* profile = builder->createOptimizationProfile();

        if(m_nchw)
        {
            profile->setDimensions(
                    m_input_name, OptProfileSelector::kMIN,
                    Dims4(1, m_input_channel, 180, 180));  // 这里的尺寸更具你自己的输入修改（最小尺寸）
            profile->setDimensions(
                    m_input_name, OptProfileSelector::kOPT,
                    Dims4(1, m_input_channel, 360, 720));  // 这里的尺寸更具你自己的输入修改（默认尺寸）
            profile->setDimensions(
                    m_input_name, OptProfileSelector::kMAX,
                    Dims4(1, m_input_channel, 2000, 2000));  // 这里的尺寸更具你自己的输入修改（最大尺寸）

        }else{
            profile->setDimensions(
                    m_input_name, OptProfileSelector::kMIN,
                    Dims4(1, 180, 180, m_input_channel));  // 这里的尺寸更具你自己的输入修改（最小尺寸）
            profile->setDimensions(
                    m_input_name, OptProfileSelector::kOPT,
                    Dims4(1, 360, 720, m_input_channel));  // 这里的尺寸更具你自己的输入修改（默认尺寸）
            profile->setDimensions(
                    m_input_name, OptProfileSelector::kMAX,
                    Dims4(1, 2000, 2000, m_input_channel));  // 这里的尺寸更具你自己的输入修改（最大尺寸）

        }

        config->addOptimizationProfile(profile);  // 添加进 IBuilderConfig
    }

    /////////////////////////////////////////////

    ICudaEngine* engine = builder->buildEngineWithConfig(*network, *config);

    // 序列化
    IHostMemory* serializedModel = engine->serialize();

    // 创建trt模型文件流
    std::ofstream serialize_output_stream(trtmodelfile);

    // 保存trt模型
    serialize_output_stream.write((const char*)serializedModel->data(),
                                  serializedModel->size());
    serializedModel->destroy();

    parser->destroy();
    network->destroy();
    config->destroy();
    builder->destroy();
    engine->destroy();
}

/**
 * main funtions here
 * */
void TrtInfer::run_model() {

    ////////////////////////Tensorrt Inference////////////////////////
    {

        for(int i=0; i<m_input_dims; i++)
        {
            cudaMemcpyAsync(m_gpu_buffers[i], m_cpu_buffers[i],
                            m_buffer_size[i], cudaMemcpyHostToDevice,
                            stream);

        }

        cudaStreamSynchronize(stream);

        // 动态尺寸
        if (m_flexible) {
            m_context->setOptimizationProfileAsync(0, stream);
            if(m_nchw)
            {
                m_context->setBindingDimensions(
                        0, Dims4(m_in_net_n, m_in_net_c, m_in_net_h, m_in_net_w));

            }else{
                m_context->setBindingDimensions(
                        0, Dims4(m_in_net_n, m_in_net_h, m_in_net_w, m_in_net_c));
            }
        }

        (*m_context).enqueueV2(&m_gpu_buffers[0], stream, nullptr);
        cudaStreamSynchronize(stream);

        for(int i=m_input_dims; i<m_nbBindings; i++)
        {
            cudaMemcpyAsync(m_cpu_buffers[i], m_gpu_buffers[i],
                            m_buffer_size[i], cudaMemcpyDeviceToHost, stream);


        }

        cudaStreamSynchronize(stream);

    }
    ////////////////////////Tensorrt Inference////////////////////////
}
