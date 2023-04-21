/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

/**
* @file av_data_packet.h
* @brief This file contains ADA::AVDataFrame class to control particular inferenced frame and attached
*/

#include "data/av_data_frame.h"

namespace ADA {

    class AVDataPacket {
    protected:
        GstBuffer *buffer;
        GstXtensor* meta;

    public:
        int width;
        int height;
        int channel;
        int batch;
        int size;
        int maxsize;
        int buffer_size;
        int frame_num;

        AVDataPacket(GstBuffer *buffer) : buffer(buffer) {
            if (not buffer)
                throw std::invalid_argument("ADA::AVDataFrame: buffer is nullptr");
        }

        AVDataPacket(GstBuffer *buffer, GstXtensor* meta) : buffer(buffer), meta(meta)
        {
            if (not buffer)
                throw std::invalid_argument("ADA::AVDataFrame: buffer is nullptr");
            width = meta->tensor_info.W;
            height = meta->tensor_info.H;
            channel = meta->tensor_info.C;
            batch = meta->tensor_info.N;
            size = meta->tensor_info.size;
            maxsize = meta->tensor_info.maxsize;

            buffer_size = gst_buffer_get_size(buffer);
            frame_num = int(buffer_size/size);

        }

        std::vector<ADA::AVDataFrame> AVDataFrame_iter()
        {
            std::vector<ADA::AVDataFrame> AVDataFrame_list;

            for(int i=0; i<frame_num; i++)
            {
                ADA::AVDataFrame video_frame(buffer, meta, i*size);
                AVDataFrame_list.push_back(video_frame);
            }
            return AVDataFrame_list;

        }

    };

} // namespace ADA
