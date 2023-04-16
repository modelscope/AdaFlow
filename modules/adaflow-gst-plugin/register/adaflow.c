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

/**
 * @brief Function to initialize all adaflow elements
 */
static gboolean plugin_init(GstPlugin* plugin) {
  gboolean ret = FALSE;
  ret |= GST_ELEMENT_REGISTER(flow_video_aggregate, plugin);
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
                  "0.0.1",
                  "Apache License 2.0",
                  "adaflow",
                  "https://github.com/modelscope/AdaFlow");
