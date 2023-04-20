/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

/**
* @file	  gst_tensor_resize.h
* @brief  AdaFlow plugin to tensor resize
*
*/

#ifndef __FLOW_TENSOR_RESIZE_H__
#define __FLOW_TENSOR_RESIZE_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "meta_tensor.h"

G_BEGIN_DECLS
/* */
GType gst_tensorresize_get_type();

#define GST_TYPE_TENSORRESIZE (gst_tensorresize_get_type())

#define GST_TENSORRESIZE(obj) \
 (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TENSORRESIZE, GstTensorResize))
#define GST_TENSORRESIZE_CLASS(klass)                      \
 (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TENSORRESIZE, \
                          GstTensorResizeClass))

#define GST_IS_TENSORRESIZE(obj) \
 (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TENSORRESIZE))
#define GST_IS_TENSORRESIZE_CLASS(klass) \
 (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TENSORRESIZE))

typedef struct _GstTensorResize GstTensorResize;
typedef struct _GstTensorResizeClass GstTensorResizeClass;

typedef struct {
   guint id;
   GstPad* srcpad;
   GstVideoInfo video_info;
} GstTensorResizePad;

struct _GstTensorResize {
   GstElement element;

   /* properties */
   guint resized_width;
   guint resized_height;
   std::vector<signed short> x_left_crood;
   std::vector<float> x_right_scale;
   std::vector<signed short> y_up_crood;
   std::vector<float> y_down_scale;
   gfloat* reserved_data;

   guint pad_count;
   gboolean initialized;
   // GMutex mutex;
   // Model *net;
};

struct _GstTensorResizeClass {
   GstElementClass parent_class;
};

GST_ELEMENT_REGISTER_DECLARE(flow_tensor_resize);

G_END_DECLS
/* */
#endif /* __FLOW_TENSOR_RESIZE_H__ */
