/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

/**
* @file av_data_frame.h
* @brief This file contains ADA::AVDataFrame class to control particular inferenced frame and attached
*/
#ifndef __AV_DATA_FRAME_H__
#define __AV_DATA_FRAME_H__


#include "metadata/flow_json_meta.h"
#include "elements/meta_tensor.h"

#include <algorithm>
#include <assert.h>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <gst/gstbuffer.h>
#include <gst/gstutils.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/video.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;


namespace ADA {

   class AVDataFrame {
   protected:
       GstBuffer *buffer;

   public:

       int width;
       int height;
       int channel;
       int batch;
       int size;
       int maxsize;
       int offset;

       AVDataFrame(GstBuffer *buffer) : buffer(buffer)
       {
           if (not buffer)
               throw std::invalid_argument("ADA::AVDataFrame: buffer is nullptr");

       }

       AVDataFrame(GstBuffer *buffer, GstXtensor* meta, int offset=0) : buffer(buffer)
       {
           if (not buffer)
               throw std::invalid_argument("ADA::AVDataFrame: buffer is nullptr");

           width = meta->tensor_info.W;
           height = meta->tensor_info.H;
           channel = meta->tensor_info.C;
           batch = meta->tensor_info.N;
           size = meta->tensor_info.size;
           maxsize = meta->tensor_info.maxsize;
           offset = offset;

       }

       void add_json_meta(const std::string message, const std::string meta_key) {

           gchar* get_message_str = gst_buffer_get_json_info_meta(buffer);

           if(strcmp(get_message_str, "NULL") == 0)
           {
               // first add
               json jobject = json::object({});

               jobject.push_back({meta_key, json::parse(message)});

               std::string message = jobject.dump(1);

               GstFLOWJSONMeta *jsonmeta =gst_buffer_add_json_info_meta(buffer, message.c_str());

           }else{
               json get_message = json::parse(get_message_str);
               if (get_message.contains(std::string{ meta_key }))
               {
                   throw std::logic_error("duplicate meta key definition, change a new key\n.");
               }else{
                   get_message.push_back({meta_key, json::parse(message)});

                   std::string message = get_message.dump(1);
                   gboolean rm = gst_buffer_remove_json_info_meta(buffer);
                   GstFLOWJSONMeta *jsonmeta =gst_buffer_add_json_info_meta(buffer, message.c_str());
               }

           }
       }

       json get_json_meta(const std::string meta_key)
       {
           gchar* get_message_str = gst_buffer_get_json_info_meta(buffer);
           if(strcmp(get_message_str, "NULL") == 0)
           {
               throw std::logic_error("AVDataFrame has not metadata\n.");
               return "NULL";
           }else{
               json get_message_json = json::parse(get_message_str);
               if (get_message_json.contains(std::string{ meta_key })){
                   json get_message = get_message_json[meta_key];
                   return get_message;
               }else{
                   throw std::logic_error("AVDataFrame has not metadata\n.");
                   return "NULL";
               }
           }

       }

       void remove_json_meta()
       {
           gst_buffer_remove_json_info_meta(buffer);
       }

       float* data()
       {
           GstMapInfo info;
           gst_buffer_map(buffer, &info, GST_MAP_READ);
           return (float*)(info.data + offset);
       }


   };

} // namespace ADA

#endif