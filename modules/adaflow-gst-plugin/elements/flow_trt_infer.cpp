/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

/**
 * @file	 gst_trt_infer.cpp
 * @date	 2022-09-06
 * @brief	 A TensorRT inference backend plugin.
 * @see		 http://gitlab.alibaba-inc.com/visual-application/xstreamer
 * @author	JingYao <jingyao.ww@alibaba-inc.com>
 * @bug		 No known bugs.
 *
 */

/**
 * SECTION:element-trt_infer
 * @title: trt_infer
 *
 * A TensorRT inference backend plugin.
 * The input and output is always in the format of xtensor.
 *
 * Example launch line
 * |[
 * gst-launch-1.0 filesrc location=test.mp4 ! decodebin ! videoconvert ! \
 * video/x-raw,width=1280,height=720,format=RGB ! tensorconvert ! \
 * tensortransform way=arithmetic option=div:255.0 ! \
 * tensortransform way=dimchg option=nchw ! trtinfer onnx=UNetVEAlqv13.onnx
 * trt=UNetVEAlqv13.trt ! \
 * tensortransform way=dimchg option=nhwc ! tensortransform way=arithmetic
 * option=mul:255.0 ! \ tensortransform way=clamp option=0:255 ! frameconvert !
 * videoconvert ! x264enc ! mp4mux ! filesink location=test.mp4
 * ]|
 */

#include "flow_trt_infer.h"

#include <dlfcn.h>
#include <stdlib.h>

#include "meta_tensor.h"

#define gst_trtinfer_parent_class parent_class

// #define GST_TENSOR_CAP_DEFAULT "video/x-raw,format=RGB; other/tensor,
// framerate = (fraction) [ 0, max ];  other/tensors, format = (string) {
// static, flexible }, framerate = (fraction) [ 0, max ]"
//  #define GST_TENSOR_CAP_DEFAULT "video/x-raw,format=RGB"
#define GST_CAP_DEFAULT GST_VIDEO_CAPS_MAKE("{ RGB, I420, NV12, NV21 }")

static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE(
        "sink_%u", GST_PAD_SINK, GST_PAD_REQUEST, GST_STATIC_CAPS(GST_CAP_DEFAULT));

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE(
        "src_%u", GST_PAD_SRC, GST_PAD_SOMETIMES, GST_STATIC_CAPS(GST_CAP_DEFAULT));

enum {
    PROP_0,
    PROP_GPU_ID,
    PROP_ONNX,
    PROP_TRT,
    PROP_LUM_ONLY,
    PROP_FLEXIBLE,
    PROP_INPUT_NAME,
    PROP_SR,
    PROP_INPUT_STRING,
    PROP_ADD_META,
    PROP_META_KEY,

};

G_DEFINE_TYPE(GstTrtinfer, gst_trtinfer, GST_TYPE_ELEMENT);
GST_ELEMENT_REGISTER_DEFINE(flow_trtinfer, "flow_trtinfer", GST_RANK_NONE,
                            GST_TYPE_TRTINFER);

#define PROP_GPU_ID_DEFAULT 0
#define ONNX_MODEL "./model/xxx.onnx"
#define TRT_MODEL "./model/xxx.trt"
#define DEFAULT_LUM_ONLY TRUE
#define DEFAULT_FLEXIBLE FALSE
#define INPUT_NAME "input"
#define DEFAULT_SR 1
#define INPUT_STRING ""
#define DEFAULT_META_KEY "modelout"

static void gst_trtinfer_set_property(GObject* object, guint prop_id,
                                      const GValue* value, GParamSpec* pspec) {
    // g_print("set_property() \n");
    GstTrtinfer* filter = GST_TRTINFER(object);
    // g_print("[gst_trtinfer_set_property] config = %s \n", filter->config);

    switch (prop_id) {
        case PROP_GPU_ID:
            filter->gpu_id = g_value_get_uint(value);
            break;

        case PROP_ONNX:
            g_free(filter->onnx_path);
            filter->onnx_path = g_value_dup_string(value);
            break;

        case PROP_TRT:
            g_free(filter->trt_path);
            filter->trt_path = g_value_dup_string(value);
            break;

        case PROP_LUM_ONLY:
            filter->lum_only = g_value_get_boolean(value);
            break;

        case PROP_FLEXIBLE:
            filter->flexible = g_value_get_boolean(value);
            break;

        case PROP_INPUT_NAME:
            g_free(filter->model_input_name);
            filter->model_input_name = g_value_dup_string(value);
            break;

        case PROP_SR:
            filter->SR = g_value_get_float(value);
            break;

        case PROP_INPUT_STRING:
            filter->input_string = g_value_dup_string(value);
            break;

        case PROP_ADD_META:
            filter->add_meta = g_value_get_boolean(value);
            break;

        case PROP_META_KEY:
            filter->meta_key = g_value_dup_string(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }

    // g_print("[gst_trtinfer_set_property] config = %s \n", filter->config);
}

static void gst_trtinfer_get_property(GObject* object, guint prop_id,
                                      GValue* value, GParamSpec* pspec) {
    // g_print("get_property() \n");
    GstTrtinfer* filter = GST_TRTINFER(object);
    // g_print("[gst_trtinfer_get_property] config = %s \n", filter->config);

    switch (prop_id) {
        case PROP_GPU_ID:
            g_value_set_uint(value, filter->gpu_id);
            break;

        case PROP_ONNX:
            g_value_set_string(value, filter->onnx_path);
            break;

        case PROP_TRT:
            g_value_set_string(value, filter->trt_path);
            break;

        case PROP_LUM_ONLY:
            g_value_set_boolean(value, filter->lum_only);
            break;

        case PROP_FLEXIBLE:
            g_value_set_boolean(value, filter->flexible);
            break;

        case PROP_INPUT_NAME:
            g_value_set_string(value, filter->model_input_name);
            break;

        case PROP_SR:
            g_value_set_float(value, filter->SR);
            break;

        case PROP_INPUT_STRING:
            g_value_set_string(value, filter->input_string);
            break;

        case PROP_ADD_META:
            g_value_set_boolean(value, filter->add_meta);
            break;

        case PROP_META_KEY:
            g_value_set_string(value, filter->meta_key);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }

    // g_print("[gst_trtinfer_get_property] config = %s \n", filter->config);
}

static gboolean gst_trtinfer_sink_event(GstPad* pad, GstObject* parent,
                                        GstEvent* event) {
    // g_print("sink_event() \n");
    GstTrtinfer* filter = GST_TRTINFER(parent);

    GstTrtinferPad* filterpad = (GstTrtinferPad*)gst_pad_get_element_private(pad);

    if (GST_EVENT_TYPE(event) == GST_EVENT_CAPS) {
        GstCaps* caps;
        gst_event_parse_caps(event, &caps);

        gst_video_info_from_caps(&filterpad->video_info, caps);

        gst_pad_set_caps(filterpad->srcpad, caps);
        return TRUE;
    }

    return gst_pad_event_default(pad, parent, event);
}

static GstFlowReturn gst_trtinfer_chain(GstPad* pad, GstObject* parent,
                                        GstBuffer* buf) {
    // g_print("chain() \n");
    GstTrtinfer* filter = GST_TRTINFER(parent);
    GstTrtinferPad* filterpad = (GstTrtinferPad*)gst_pad_get_element_private(pad);
    guint i, j, k, batch, channel, height, width, size, maxsize;
    guint xnn_size_in;
    guint xnn_size_out;

    GstXtensor* meta = GST_XTENSOR_GET(buf);
    // if (!filter->initialized)
    {
        // input shape
        width = meta->tensor_info.W;
        height = meta->tensor_info.H;
        channel = meta->tensor_info.C;
        batch = meta->tensor_info.N;
        size = meta->tensor_info.size;
        maxsize = meta->tensor_info.maxsize;

        if (filter->lum_only && (meta->tensor_info.format == FLOW_I420 ||
                                 meta->tensor_info.format == FLOW_NV12 ||
                                 meta->tensor_info.format == FLOW_NV21)) {
            xnn_size_in = batch * width * height;
        } else {
            xnn_size_in = size;
        }

        // GST_XTENSOR_MALLOC(meta);
        // filter->initialized = TRUE;
    }

    // initialize & Load model
    if (!filter->model_trt) {
        cudaSetDevice(filter->gpu_id);

        filter->model_path = filter->onnx_path;
        printf("model_path = %s \n", filter->model_path);

        //make sure input dim
        int input_channel = 3;
        if (filter->lum_only && (meta->tensor_info.format == FLOW_I420 ||
                                 meta->tensor_info.format == FLOW_NV12 ||
                                 meta->tensor_info.format == FLOW_NV21))
        {
            input_channel = 1;
        }

        filter->model_trt = new TrtInfer();
        filter->model_trt->init_model(filter->onnx_path, filter->trt_path,
                                      filter->flexible, filter->model_input_name, width, height, filter->SR, 1, true, input_channel);

        filter->m_net_in_n = filter->model_trt->input_batch_num();
        filter->m_net_in_c = filter->model_trt->input_channel_num();
        filter->m_net_in_h = filter->model_trt->input_height_num();
        filter->m_net_in_w = filter->model_trt->input_width_num();

        filter->m_net_out_n = filter->model_trt->output_batch_num();
        filter->m_net_out_c = filter->model_trt->output_channel_num();
        filter->m_net_out_h = filter->model_trt->output_height_num();
        filter->m_net_out_w = filter->model_trt->output_width_num();

    }

    // input
    int src_n = filter->m_net_in_n;
    int src_c = filter->m_net_in_c;
    int src_h = filter->m_net_in_h;
    int src_w = filter->m_net_in_w;

    // output
    int dst_n = filter->m_net_out_n;
    int dst_c = filter->m_net_out_c;
    int dst_h = filter->m_net_out_h;
    int dst_w = filter->m_net_out_w;

    GstMapInfo info;
    gst_buffer_map(buf, &info, GST_MAP_READ);

    // net input of TRT:nchw
    memcpy(filter->model_trt->m_cpu_buffers[0], (float*)info.data,
           xnn_size_in * sizeof(float));

    // inference of TRT:nchw
    filter->model_trt->run_model();

    // net output
    float* p_out_net_data;
    if(filter->add_meta){
        p_out_net_data = (float*)info.data;

    }else{
        p_out_net_data = (float*)filter->model_trt->m_cpu_buffers[1];
    }

    gst_buffer_unmap(buf, &info);

    {
        // output shape
        meta->tensor_info.device = CPU;
        meta->tensor_info.type = DATA_FLOAT32;
        meta->tensor_info.N = batch;
        // meta->tensor_info.C = dst_c;
        meta->tensor_info.H = dst_h;
        meta->tensor_info.W = dst_w;
        // meta->tensor_info.size = batch * dst_c * dst_h * dst_w;
        // meta->tensor_info.maxsize = meta->tensor_info.size >= maxsize ?
        // meta->tensor_info.size : maxsize;

        if (filter->lum_only && (meta->tensor_info.format == FLOW_I420 ||
                                 meta->tensor_info.format == FLOW_NV12 ||
                                 meta->tensor_info.format == FLOW_NV21)) {
            xnn_size_out = batch * dst_h * dst_w;
            meta->tensor_info.size = batch * 1.5 * dst_h * dst_w;
            meta->tensor_info.maxsize =
                    meta->tensor_info.size >= maxsize ? meta->tensor_info.size : maxsize;
            meta->tensor_info.C = 3;
        } else {
            meta->tensor_info.size = batch * dst_c * dst_h * dst_w;
            meta->tensor_info.maxsize =
                    meta->tensor_info.size >= maxsize ? meta->tensor_info.size : maxsize;
            meta->tensor_info.C = dst_c;
            xnn_size_out = meta->tensor_info.size;
        }
    }

    if (meta->tensor_info.maxsize == maxsize) {
        GstMapInfo info;
        gst_buffer_map(buf, &info, GST_MAP_READ);

        gfloat* pdst = (gfloat*)info.data;
        memcpy(pdst, p_out_net_data, xnn_size_out * sizeof(float));

        gst_buffer_unmap(buf, &info);

        if(filter->add_meta)
        {
            gst_add_meta_to_buffer(filter->model_trt, buf, filter->meta_key);
        }

        return gst_pad_push(filterpad->srcpad, buf);
    } else {
        GstMapInfo src_info, dest_info;
        gsize frame_size =
                meta->tensor_info.maxsize * tensor_element_size[meta->tensor_info.type];
        gst_buffer_map(buf, &src_info, GST_MAP_READ);

        GstBuffer* inbuf = gst_buffer_new_and_alloc(frame_size);
        gst_buffer_memset(inbuf, 0, 0, frame_size);
        gst_buffer_map(inbuf, &dest_info, GST_MAP_WRITE);

        gfloat* pdst = (gfloat*)dest_info.data;
        gfloat* psrc = (gfloat*)src_info.data;
        memcpy(pdst, p_out_net_data, xnn_size_out * sizeof(float));

        if (filter->lum_only && meta->tensor_info.format == FLOW_I420) {
            // printf("format-I420\n");
            I420uv_resize(psrc, src_w, src_h, pdst, dst_w, dst_h);
        }
        if (filter->lum_only && (meta->tensor_info.format == FLOW_NV12 ||
                                 meta->tensor_info.format == FLOW_NV21)) {
            // printf("mnn-format-nv12\n");
            NVuv_resize(psrc, src_w, src_h, pdst, dst_w, dst_h);
        }

        gst_buffer_unmap(buf, &src_info);
        gst_buffer_unmap(inbuf, &dest_info);

        /** copy timestamps */
        gst_buffer_copy_into(inbuf, buf, GST_BUFFER_COPY_METADATA, 0, -1);

        if (inbuf != buf) {
            gst_buffer_unref(buf);
        }

        return gst_pad_push(filterpad->srcpad, inbuf);
    }

    // g_mutex_lock(&filter->mutex);
    // //todo
    // g_mutex_unlock(&filter->mutex);
}

static void gst_trtinfer_finalize(GObject* object) {
    GstTrtinfer* filter = GST_TRTINFER(object);

    // Release char *
    if (filter->onnx_path) {
        g_free(filter->onnx_path);
        filter->onnx_path = NULL;
    }
    if (filter->trt_path) {
        g_free(filter->trt_path);
        filter->trt_path = NULL;
    }

    // Release memory
    if (filter->model_trt) {
        filter->model_trt->release();
        filter->model_trt = NULL;
    }

    G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void gst_trtinfer_init_after_props(GstTrtinfer* filter) {
    // g_mutex_init(&filter->mutex);
    // cuda_set_device(filter->gpu_id);
    // filter->Model = load_model(filter->config);
}

static GstPad* gst_trtinfer_request_new_pad(GstElement* element,
                                            GstPadTemplate* templ,
                                            const gchar* name,
                                            const GstCaps* caps) {
    GstTrtinfer* filter = GST_TRTINFER(element);

    // if (filter->model == NULL)
    { gst_trtinfer_init_after_props(filter); }

    GstPad* pad = gst_pad_new_from_template(templ, name);
    GstTrtinferPad* filterpad = g_new(GstTrtinferPad, 1);
    filterpad->id = filter->pad_count++;
    // filterpad->image_scaled = make_image(filter->net->w, filter->net->h, 3);
    gst_pad_set_element_private(pad, filterpad);
    gst_pad_set_event_function(pad, gst_trtinfer_sink_event);
    gst_pad_set_chain_function(pad, gst_trtinfer_chain);
    gst_element_add_pad(element, pad);

    gchar* srcname = g_strdup_printf(src_factory.name_template, filterpad->id);
    filterpad->srcpad = gst_pad_new_from_static_template(&src_factory, srcname);
    g_free(srcname);
    gst_element_add_pad(element, filterpad->srcpad);

    return pad;
}

static void gst_trtinfer_class_init(GstTrtinferClass* klass) {
    // g_print("class_init() \n");
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    GstElementClass* element_class = (GstElementClass*)klass;

    object_class->set_property = gst_trtinfer_set_property;
    object_class->get_property = gst_trtinfer_get_property;
    object_class->finalize = gst_trtinfer_finalize;

    constexpr auto param_flags =
            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_property(
            object_class, PROP_GPU_ID,
            g_param_spec_uint("gpu-id", "gpu-id", "GPU to use for inference",
                              PROP_GPU_ID_DEFAULT, 100, 0, param_flags));

    g_object_class_install_property(
            object_class, PROP_ONNX,
            g_param_spec_string("onnx", "Onnx", "path to a onnx model file",
                                ONNX_MODEL, param_flags));

    g_object_class_install_property(
            object_class, PROP_TRT,
            g_param_spec_string("trt", "Trt", "path to save trt model file",
                                TRT_MODEL, param_flags));

    g_object_class_install_property(
            object_class, PROP_LUM_ONLY,
            g_param_spec_boolean("lum-only", "Lum Only",
                                 "Only apply filter on luminance", DEFAULT_LUM_ONLY,
                                 param_flags));

    g_object_class_install_property(
            object_class, PROP_FLEXIBLE,
            g_param_spec_boolean("flexible", "Flexible",
                                 "if shape of model is flexible?", DEFAULT_FLEXIBLE,
                                 param_flags));

    g_object_class_install_property(
            object_class, PROP_INPUT_NAME,
            g_param_spec_string("input-name", "Input-name",
                                "name of model input layer", INPUT_NAME,
                                param_flags));

    g_object_class_install_property(
            object_class, PROP_SR,
            g_param_spec_float("sr", "SR", "size change", 0, 20, DEFAULT_SR,
                               param_flags));

    g_object_class_install_property(
            object_class, PROP_INPUT_STRING,
            g_param_spec_string("input-string", "Input-string",
                                "string description/json file of model input need",
                                INPUT_STRING, param_flags));

    g_object_class_install_property(
            object_class, PROP_ADD_META,
            g_param_spec_boolean("add-meta", "add-meta",
                                "is to add metadata attach buffer",
                                false, param_flags));
    g_object_class_install_property(
            object_class, PROP_META_KEY,
            g_param_spec_string("meta-key", "meta-key",
                                "the keyword of the results which attach to the GstBuffer",
                                DEFAULT_META_KEY, param_flags));

    gst_element_class_set_details_simple(element_class, "flow_trtinfer", "adaflow",
                                         "flow_trtinfer", "AUTHOR_NAME AUTHOR_EMAIL");

    gst_element_class_add_pad_template(element_class,
                                       gst_static_pad_template_get(&src_factory));

    gst_element_class_add_pad_template(
            element_class, gst_static_pad_template_get(&sink_factory));

    element_class->request_new_pad = gst_trtinfer_request_new_pad;
}

static void gst_trtinfer_init(GstTrtinfer* filter) {
    // g_print("[trt infer] init() \n");

    filter->gpu_id = PROP_GPU_ID_DEFAULT;
    filter->onnx_path = g_strdup(ONNX_MODEL);
    filter->trt_path = g_strdup(TRT_MODEL);
    filter->pad_count = 0;
    filter->initialized = FALSE;

    filter->model_trt = NULL;
    filter->lum_only = DEFAULT_LUM_ONLY;
    filter->flexible = DEFAULT_FLEXIBLE;
    filter->model_input_name = g_strdup(INPUT_NAME);
    filter->SR = DEFAULT_SR;
    filter->input_string = g_strdup(INPUT_STRING);
    filter->meta_key = g_strdup(DEFAULT_META_KEY);

}

void gst_add_meta_to_buffer(TrtInfer* model_trt, GstBuffer* buffer, std::string meta_key)
{
    json jobject = json::object({});

    int nbBindings = model_trt->m_nbBindings;

    for(int i=1; i<nbBindings; i++)
    {
        const char* output_name = model_trt->m_buffer_name[i];
        float *p = (float*)model_trt->m_cpu_buffers[i];

        json sjarry = json::array();
        json jarry = json::array({});
        int shape_dim = model_trt->m_buffer_shape[i].nbDims;

        if(shape_dim==1)
        {
            for(int i_0=0; i_0<model_trt->m_buffer_shape[i].d[0]; i_0++)
            {
                sjarry.push_back(p[i_0]);
            }
            jarry.push_back(sjarry);

        }else if(shape_dim==2)
        {
            for(int i_0=0; i_0<model_trt->m_buffer_shape[i].d[0]; i_0++)
            {
                sjarry ={};
                int i_offset = i_0*model_trt->m_buffer_shape[i].d[1];
                for(int i_1=0; i_1<model_trt->m_buffer_shape[i].d[1]; i_1++)
                {
                    sjarry.push_back(p[i_offset+i_1]);
                }
                jarry.push_back(sjarry);
            }

        }else if(shape_dim==3)
        {
            for(int i_0=0; i_0<model_trt->m_buffer_shape[i].d[0]; i_0++)
            {
                int i_offset_0 = i_0*model_trt->m_buffer_shape[i].d[1]*model_trt->m_buffer_shape[i].d[2];
                for(int i_1=0; i_1<model_trt->m_buffer_shape[i].d[1]; i_1++)
                {
                    sjarry ={};
                    int i_offset_1 = i_1*model_trt->m_buffer_shape[i].d[2];
                    for(int i_2=0; i_2<model_trt->m_buffer_shape[i].d[2]; i_2++)
                    {
                        sjarry.push_back(p[i_offset_0+i_offset_1+i_2]);
                    }
                    jarry.push_back(sjarry);
                }
            }

        }

        jobject.push_back({output_name,jarry});
    }

    json jobject_key = json::object({});

    jobject_key.push_back({meta_key, jobject});

    std::string message = jobject_key.dump(1);

    GstFLOWJSONMeta *jsonmeta =gst_buffer_add_json_info_meta(buffer, message.c_str());

    printf("successful add json meta\n");

}


