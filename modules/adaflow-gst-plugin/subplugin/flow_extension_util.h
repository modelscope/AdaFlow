/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

#ifndef __FLOW_EXTENSION_UTIL_H__
#define __FLOW_EXTENSION_UTIL_H__

#include <glib.h>
#include <gst/gst.h>

#include "flow_extension_common.h"
#include "elements/meta_tensor.h"

enum {
  PROP_0,
  PROP_FRAMEWORK,
  PROP_MODEL,
  PROP_DEVICE,
  PROP_INPUT,
  PROP_INPUTTYPE,
  PROP_INPUTNAME,
  PROP_INPUTLAYOUT,
  PROP_INPUTRANKS,
  PROP_OUTPUT,
  PROP_OUTPUTTYPE,
  PROP_OUTPUTNAME,
  PROP_OUTPUTLAYOUT,
  PROP_OUTPUTRANKS,
  PROP_CUSTOM,
  PROP_SUBPLUGINS,
  PROP_ACCELERATOR,
  PROP_IS_UPDATABLE,
  PROP_LATENCY,
  PROP_THROUGHPUT,
  PROP_INPUTWINDOW,
  PROP_OUTPUTWINDOW,
  PROP_SHARED_TENSOR_FILTER_KEY,
};

extern void gst_almighty_filter_install_properties(GObjectClass* gobject_class);

extern void gst_almighty_filter_common_init_property(
    GstAlmightyFilterFramework* fw);
extern void gst_almighty_filter_common_close_fw(GstAlmightyFilterFramework* fw);
extern void gst_almighty_filter_common_free_property(
    GstAlmightyFilterFramework* fw);
extern gboolean gst_almighty_filter_common_set_property(
    GstAlmightyFilterFramework* fw, guint prop_id, const GValue* value,
    GParamSpec* pspec);
extern uint gst_almighty_get_tensor_size(GstAlmightyFilterFramework* fw,
                                         int tensor_id, gboolean is_input);
// extern gboolean gst_almighty_config_from_structure(GstTensorsConfig *config,
// const GstStructure *structure);
extern void gst_almighty_config_init(TensorConfig* config);
extern void gst_almighty_info_init(TensorInfo* info);

#endif