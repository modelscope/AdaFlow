/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/
#include "flow_filter_yolodetect.h"

#include "stdio.h"

int yolodetect_filter_class::init() {
  printf("test subplugin init\n");
  return 0;
}

int yolodetect_filter_class::set_in_tensor(const TensorInfo* info) {
  if (info) {
    memcpy(&in_info_, info, sizeof(TensorInfo));
  }
  return 0;
}

int yolodetect_filter_class::get_out_tensor_info(TensorInfo* info) {
  if (info) {
    memcpy(info, &out_info_, sizeof(TensorInfo));
  }
  return 0;
}

int yolodetect_filter_class::set_option(const char* value) {
  strength = value;
  // when output.info == input.info
  memcpy(&out_info_, &in_info_, sizeof(TensorInfo));

  // set change output info
  //  out_info_.N = in_info_.N;
  //  out_info_.C = in_info_.C;
  //  out_info_.H = in_info_.H * 2;
  //  out_info_.W = in_info_.W * 2;
  //  out_info_.size = in_info_.size;
  //  out_info_.maxsize = in_info_.maxsize;
  return 0;
}

int yolodetect_filter_class::invoke(const TensorMemory* input,
                                    TensorMemory* output) {
  int in_height = in_info_.H;
  int in_width = in_info_.W;
  int in_channel = in_info_.C;
  // main usr function
  printf("put your function here !!!\n");
  return 0;
}

int yolodetect_filter_class::metapost(GstBuffer* buffer, GstXtensor* meta) {
  ADA::AVDataPacket avpacket(buffer, meta);
  // main usr function
  printf("put your function here !!!\n");
  return 0;
}

void yolodetect_filter_class::release() {
  printf("test subplugin close\n");
  return;
}

#include "flow_subplugin_api.h"

#define SUBPLUGIN_CLASSNAME yolodetect_filter_class

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