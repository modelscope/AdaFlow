/*******************************************************************************
 * Copyright (c) Alibaba, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0
 ******************************************************************************/

/**
* @file flow_trt_infer.h
* @brief A TensorRT inference backend plugin
*/


#ifndef ADAFLOW_FLOW_TRT_INFER_H
#define ADAFLOW_FLOW_TRT_INFER_H

#include <glib.h>
#include <gst/gst.h>
#include <gst/video/video.h>

#include "common.h"
#include "elements/meta_tensor.h"
#include "ann/trt/trt_infer.h"
#include "metadata/flow_json_meta.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;


G_BEGIN_DECLS
/* */
GType gst_trtinfer_get_type();

#define GST_TYPE_TRTINFER (gst_trtinfer_get_type())

#define GST_TRTINFER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TRTINFER, GstTrtinfer))
#define GST_TRTINFER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TRTINFER, GstTrtinferClass))

#define GST_IS_TRTINFER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TRTINFER))
#define GST_IS_TRTINFER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TRTINFER))

typedef struct _GstTrtinfer GstTrtinfer;
typedef struct _GstTrtinferClass GstTrtinferClass;

typedef struct {
    guint id;
    GstPad* srcpad;
    GstVideoInfo video_info;
} GstTrtinferPad;

/**
 * @brief GstTrtinfer data structure.
 */
struct _GstTrtinfer {
    GstElement element;

    /* properties */
    guint gpu_id;            /**< GPU ID */
    gchar* onnx_path;        /**< onnx model format path */
    gchar* trt_path;         /**< trt model format path */
    gchar* model_input_name; /**< model input name */
    gchar* input_string;

    guint pad_count;
    gboolean initialized;
    gboolean lum_only;
    gboolean flexible;
    gboolean add_meta;
    float SR;
    // GMutex mutex;
    // Model *model;

    char* model_path;    // xxx.trt
    TrtInfer* model_trt; /**< inference engine interface */
    // net input w and h
    int m_net_in_n; /**< input batch size */
    int m_net_in_w; /**< input width size */
    int m_net_in_h; /**< input height size */
    int m_net_in_c; /**< input channel size */
    // net output w and h
    int m_net_out_n; /**< output batch size */
    int m_net_out_w; /**< output width size */
    int m_net_out_h; /**< output height size */
    int m_net_out_c; /**< output channel size */
};

/**
 * @brief GstTrtinferClass data structure.
 */
struct _GstTrtinferClass {
    GstElementClass parent_class;
};

void gst_add_meta_to_buffer(TrtInfer* model_trt, GstBuffer* buffer);

GST_ELEMENT_REGISTER_DECLARE(flow_trtinfer);

G_END_DECLS

#endif//ADAFLOW_FLOW_TRT_INFER_H