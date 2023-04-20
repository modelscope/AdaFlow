/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

/**
* @file	flow_tensor_compositor.h
* @brief AdaFlow plugin to fusion of two video streams
*
*/

#ifndef __FLOW_TENSOR_COMPOSITOR_H__
#define __FLOW_TENSOR_COMPOSITOR_H__

#include <gst/base/base.h>
#include <gst/gst.h>
#include <gst/video/gstvideoaggregator.h>
#include <gst/video/video.h>

G_BEGIN_DECLS

#define GST_TYPE_FLOWTENSORCOMPOSITOR (gst_flowtensorcompositor_get_type())
G_DECLARE_FINAL_TYPE(Flowtensorcompositor, gst_flowtensorcompositor, GST, FLOWTENSORCOMPOSITOR,
                    GstVideoAggregator)

#define GST_TYPE_FLOWTENSORCOMPOSITOR_PAD (gst_flowtensorcompositor_pad_get_type())
G_DECLARE_FINAL_TYPE(FlowtensorcompositorPad, gst_flowtensorcompositor_pad, GST,
                    FLOWTENSORCOMPOSITOR_PAD, GstVideoAggregatorParallelConvertPad)

typedef enum {
   FLOWTENSORCOMPOSITOR_OPERATOR_SEGMENT,
   FLOWTENSORCOMPOSITOR_OPERATOR_MERGEYUV,
} FlowtensorcompositorOperator;

typedef enum {
   GX_RED = 0,
   GX_BLACK = 1,
   GX_WHITE = 2,
   GX_COLORUNKNOWN
} background_color;

/**
* @brief Flowtensorcompositor data structure.
*/
struct _Flowtensorcompositor {
   GstVideoAggregator videoaggregator; /**< parent object */
   FlowtensorcompositorOperator op;          /**< operator */
   gchar* bgcolor;                     /**< option color for background */
   background_color bg_color;          /**< color mode choose: red/black/white */
};

/**
* @brief FlowtensorcompositorClass data structure.
*/
struct _FlowtensorcompositorPad {
   GstVideoAggregatorParallelConvertPad parent;
};

GST_ELEMENT_REGISTER_DECLARE(flow_tensor_compositor);

G_END_DECLS
#endif /* __FLOW_TENSOR_COMPOSITOR_H__ */
