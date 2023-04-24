/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

#ifndef __FLOW_SUBPLUGIN_BASE_CLASS_H__
#define __FLOW_SUBPLUGIN_BASE_CLASS_H__

#include "subplugin/flow_extension_util.h"

class SubpluginBaseClass {
 public:
  SubpluginBaseClass();
  ~SubpluginBaseClass();
  virtual int init() = 0;
  virtual int set_in_tensor(const TensorInfo* info) = 0;
  virtual int get_out_tensor_info(TensorInfo* info) = 0;
  virtual int set_option(const char* value) = 0;
  virtual int invoke(const TensorMemory* input, TensorMemory* output) = 0;
  virtual int metapost(GstBuffer* buffer, GstXtensor* meta) = 0;
  virtual void release() = 0;

 private:
};

#endif