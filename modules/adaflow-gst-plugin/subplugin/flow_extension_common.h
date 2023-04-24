/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

#ifndef __FLOW_EXTENSION_COMMON_H__
#define __FLOW_EXTENSION_COMMON_H__

#include <glib-object.h>

#include "elements/meta_tensor.h"

typedef int (*FUNC_OPEN)(void**);
typedef void (*FUNC_CLOSE)(void*);
typedef int (*FUNC_SETINTENSOR)(void*, const TensorInfo*);
typedef int (*FUNC_GETOUTPUTTENSOR)(void*, TensorInfo*);
typedef int (*FUNC_SETOPTION)(void*, const char* value);
typedef int (*FUNC_INVOKE)(void**, const TensorMemory*, TensorMemory*);
typedef int (*FUNC_METAPOST)(void**, GstBuffer* buffer, GstXtensor* meta);

typedef struct _GstAlmightyFilterApi {
  FUNC_OPEN open;
  FUNC_CLOSE close;
  FUNC_SETINTENSOR set_in_tensor;
  FUNC_GETOUTPUTTENSOR get_out_tensor;
  FUNC_SETOPTION set_option;
  FUNC_INVOKE invoke;
  FUNC_METAPOST metapost;
} GstAlmightyFilterApi;

typedef struct _GstAlmightyFilterData {
  bool fw_opened;
  void* subplugin_handle;
  void* subpluginlib_handle;
  const char* fw_name;
  const char* model;
  const char* device;
  const char* options;
  bool in_tensor_configured;
  bool out_tensor_configured;
  TensorInfo input_tensor_meta;
  TensorInfo output_tensor_meta;

  int latency;
  int throughput;
} GstAlmightyFilterData;

typedef struct _GstAlmightyFilterFramework {
  gboolean data_configed, api_configed;
  GstAlmightyFilterData data;
  GstAlmightyFilterApi api;
} GstAlmightyFilterFramework;

#endif