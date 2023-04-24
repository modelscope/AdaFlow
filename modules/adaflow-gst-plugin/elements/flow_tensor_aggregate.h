/*******************************************************************************
 * Copyright (c) Alibaba, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0
 ******************************************************************************/

/**
* @file flow_tensor_aggregate.h
* @brief flow_tensor_aggregate is a plugin to aggregate the frame using GstAdapter
*/

#ifndef FLOW_FLOW_TENSOR_AGGREGATE_H
#define FLOW_FLOW_TENSOR_AGGREGATE_H

#include <gst/gst.h>
#include <gst/video/video.h>
#include "elements/meta_tensor.h"

G_BEGIN_DECLS

GType gst_flowtensoraggregator_get_type();

#define GST_TYPE_FLOWTENSORAGGREGATOR (gst_flowtensoraggregator_get_type())
#define GST_FLOWTENSORAGGREGATOR(obj)                               \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_FLOWTENSORAGGREGATOR, \
                              GstFlowtensoraggregator))
#define GST_FLOWTENSORAGGREGATOR_CLASS(klass)                      \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_FLOWTENSORAGGREGATOR, \
                           GstFlowtensoraggregatorClass))
#define GST_IS_FLOWTENSORAGGREGATOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_FLOWTENSORAGGREGATOR))
#define GST_IS_FLOWTENSORAGGREGATOR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_FLOWTENSORAGGREGATOR))

typedef struct _GstFlowtensoraggregator GstFlowtensoraggregator;
typedef struct _GstFlowtensoraggregatorClass GstFlowtensoraggregatorClass;

/**
 * @brief GstFlowtensoraggregatorPad data structure.
 */
typedef struct {
  guint id;
  GstPad* srcpad;
  GstVideoInfo video_info;
} GstFlowtensoraggregatorPad;

/**
 * @brief GstFlowtensoraggregator data structure.
 */
struct _GstFlowtensoraggregator {
  GstElement element; /**< parent object */

  GstPad* sinkpad; /**< sink pad */
  GstPad* srcpad;  /**< src pad */

  gboolean silent;    /**< true to print minimized log */
  guint frames_in;    /**< number of frames in input buffer */
  guint frames_out;   /**< number of frames in output buffer */
  guint frames_flush; /**< number of frames to flush */

  guint pad_count;    /**< number of pad */

  GstAdapter* adapter; /**< adapt incoming tensor */
};

/**
 * @brief GstFlowtensoraggregatorClass data structure.
 */
struct _GstFlowtensoraggregatorClass {
  GstElementClass parent_class; /**< parent class */
};

GST_ELEMENT_REGISTER_DECLARE(flow_tensor_aggregate);

G_END_DECLS

#endif // FLOW_FLOW_TENSOR_AGGREGATE_H
