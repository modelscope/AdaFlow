/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

#ifndef ADAFLOW_FLOW_FRAME_CONVERT_H
#define ADAFLOW_FLOW_FRAME_CONVERT_H

#include <gst/gst.h>
#include <gst/video/video.h>

#include "meta_tensor.h"

#include "metadata/flow_json_meta.h"
#include <fstream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

G_BEGIN_DECLS
/* */
GType gst_frameconvert_get_type();

#define GST_TYPE_FRAMECONVERT (gst_frameconvert_get_type())

#define GST_FRAMECONVERT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_FRAMECONVERT, GstFrameconvert))
#define GST_FRAMECONVERT_CLASS(klass)                      \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_FRAMECONVERT, \
                           GstFrameconvertClass))

#define GST_IS_FRAMECONVERT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_FRAMECONVERT))
#define GST_IS_FRAMECONVERT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_FRAMECONVERT))

typedef struct _GstFrameconvert GstFrameconvert;
typedef struct _GstFrameconvertClass GstFrameconvertClass;

typedef struct {
    guint id;
    GstPad* srcpad;
    GstVideoInfo video_info;
    _TensorInfo out_info;
} GstFrameconvertPad;

/**
 * @brief Internal data structure for frameconvert instances.
 */
struct _GstFrameconvert {
    GstElement element;

    /* properties */
    guint pad_count;
    gboolean initialized;

    char* json_path;

};

/**
 * @brief GstFrameconvertClass inherits GstElement.
 */
struct _GstFrameconvertClass {
    GstElementClass parent_class;
};

GST_ELEMENT_REGISTER_DECLARE(flow_tensor_decode);

G_END_DECLS
/* */

#endif//ADAFLOW_FLOW_FRAME_CONVERT_H
