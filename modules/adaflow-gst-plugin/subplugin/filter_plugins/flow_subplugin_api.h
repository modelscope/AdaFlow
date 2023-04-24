/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

#ifndef __XST_SUBPLUGIN_FILTER_API_H__
#define __XST_SUBPLUGIN_FILTER_API_H__

#include "elements/meta_tensor.h"

#define API_PUBLIC __attribute__((visibility("default")))

#ifdef __cplusplus
extern "C" {
#endif

API_PUBLIC int api_open(void** subplugin_handler);

API_PUBLIC int api_set_in_tensor(void* subplugin_handler,
                                 const TensorInfo* info);

API_PUBLIC int api_get_out_tensor_info(void* subplugin_handler,
                                       TensorInfo* info);

API_PUBLIC int api_set_option(void* subplugin_handler, const char* value);

API_PUBLIC int api_invoke(void* subplugin_handler, const TensorMemory* input,
                          TensorMemory* output);

API_PUBLIC int api_metapost(void* subplugin_handler, GstBuffer* buffer, GstXtensor* meta);

API_PUBLIC void api_close(void* subplugin_handler);

#ifdef __cplusplus
}
#endif
#endif