/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/
#ifndef __FLOW_SUBPLUGIN_FILTER_TEMP_H__
#define __FLOW_SUBPLUGIN_FILTER_TEMP_H__

#include "flow_subplugin_base_class.h"
#include "data/av_data_packet.h"
#include <string.h>

class test_filter_class {
 public:
  int init();
  int set_in_tensor(const TensorInfo* info);
  int get_out_tensor_info(TensorInfo* info);
  int set_option(const char* value);
  int invoke(const TensorMemory* input, TensorMemory* output);
  int metapost(GstBuffer* buffer, GstXtensor* meta);
  void release();

 private:
  // usr subplugin resource
  TensorInfo in_info_;
  TensorInfo out_info_;

  // usr set parameter
  const char* strength;
  int out_width;
  int out_height;
  int out_channel;
  int out_batch;
};

#endif