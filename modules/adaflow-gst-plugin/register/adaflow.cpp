/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

/**
* @file	 adaflow.c
* @brief Registers all adaflow C-plugins for gstreamer so that we can have a single big binary
*/

#include <gst/gst.h>
#include <elements/flow_video_aggregate.h>
#include <elements/flow_tensor_aggregate.h>
#include <elements/flow_tensor_decode.h>
#include <elements/flow_tensor_convert.h>
#include <elements/flow_tensor_transform.h>
#include <elements/flow_tensor_compositor.h>
#include <elements/flow_tensor_resize.h>
#include <subplugin/flow_c_extension.h>

#if defined(ADAFLOW_USE_TRT)
#include <elements/flow_trt_infer.h>
#endif

/**
 * @brief Function to initialize all adaflow elements
 */
static gboolean plugin_init(GstPlugin* plugin) {
  gboolean ret = FALSE;
  ret |= GST_ELEMENT_REGISTER(flow_video_aggregate, plugin);
  ret |= GST_ELEMENT_REGISTER(flow_tensor_aggregate, plugin);
  ret |= GST_ELEMENT_REGISTER(flow_tensor_convert, plugin);
  ret |= GST_ELEMENT_REGISTER(flow_tensor_decode, plugin);
  ret |= GST_ELEMENT_REGISTER(flow_tensor_transform, plugin);
  ret |= GST_ELEMENT_REGISTER(flow_tensor_compositor, plugin);
  ret |= GST_ELEMENT_REGISTER(flow_tensor_resize, plugin);
  ret |= GST_ELEMENT_REGISTER(flow_c_extension, plugin);

#if defined(ADAFLOW_USE_TRT)
  ret |= GST_ELEMENT_REGISTER(flow_trtinfer, plugin);
#endif
  return ret;
}

#ifndef PACKAGE
#define PACKAGE "adaflow"
#endif

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  flow_gst_plugin,
                  "adaflow plugins",
                  plugin_init,
                  // VERSION
                  "1.0.1",
                  // LICENSE
                  // FIXME(long.qul) which license?
                  "LGPL",
                  "adaflow",
                  "https://github.com/modelscope/AdaFlow");
