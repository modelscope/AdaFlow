/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

#include "flow_subplugin_api.h"

#define SUBPLUGIN_CLASSNAME test_filter_class

int api_open(void** subplugin_handler) {
  SUBPLUGIN_CLASSNAME* subplugin_handler_local = new SUBPLUGIN_CLASSNAME;
  *subplugin_handler = subplugin_handler_local;
  return subplugin_handler_local->init();
}

int api_set_in_tensor(void* subplugin_handler, const TensorInfo* info) {
  SUBPLUGIN_CLASSNAME* subplugin_handler_local =
      (SUBPLUGIN_CLASSNAME*)subplugin_handler;
  return subplugin_handler_local->set_in_tensor(info);
}

int api_get_out_tensor_info(void* subplugin_handler, TensorInfo* info) {
  SUBPLUGIN_CLASSNAME* subplugin_handler_local =
      (SUBPLUGIN_CLASSNAME*)subplugin_handler;
  return subplugin_handler_local->get_out_tensor_info(info);
}

int api_set_option(void* subplugin_handler, const char* value) {
  SUBPLUGIN_CLASSNAME* subplugin_handler_local =
      (SUBPLUGIN_CLASSNAME*)subplugin_handler;
  return subplugin_handler_local->set_option(value);
}

int api_invoke(void* subplugin_handler, const TensorMemory* input,
               TensorMemory* output) {
  SUBPLUGIN_CLASSNAME* subplugin_handler_local =
      (SUBPLUGIN_CLASSNAME*)subplugin_handler;
  return subplugin_handler_local->invoke(input, output);
}

int api_metapost(void* subplugin_handler, GstBuffer* buffer, GstXtensor* meta) {
  SUBPLUGIN_CLASSNAME* subplugin_handler_local =
          (SUBPLUGIN_CLASSNAME*)subplugin_handler;
  return subplugin_handler_local->metapost(buffer, meta);
}

void api_close(void* subplugin_handler) {
  SUBPLUGIN_CLASSNAME* subplugin_handler_local =
      (SUBPLUGIN_CLASSNAME*)subplugin_handler;
  subplugin_handler_local->release();
  delete subplugin_handler_local;
}