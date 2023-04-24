/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

#ifndef __FLOW_C_EXTENSION_H__
#define __FLOW_C_EXTENSION_H__

#include <gst/gst.h>
#include <gst/gstinfo.h>
#include <gst/video/video.h>

#include "elements/meta_tensor.h"
#include "flow_extension_common.h"

G_BEGIN_DECLS
/* */
GType gst_almighty_filter_get_type();

#define GST_TYPE_ALMIGHTY_FILTER (gst_almighty_filter_get_type())

#define GST_ALMIGHTY_FILTER(obj)                               \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_ALMIGHTY_FILTER, \
                              GstAlmightyFilter))
#define GST_ALMIGHTY_FILTER_CLASS(klass)                      \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_ALMIGHTY_FILTER, \
                           GstAlmightyFilterClass))

#define GST_IS_ALMIGHTY_FILTER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_ALMIGHTY_FILTER))
#define GST_IS_ALMIGHTY_FILTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_ALMIGHTY_FILTER))

typedef struct _GstAlmightyFilter GstAlmightyFilter;
typedef struct _GstAlmightyFilterClass GstAlmightyFilterClass;

typedef struct {
  guint id;
  GstPad* srcpad;
  GstVideoInfo video_info;
  _TensorInfo out_info;
} GstAlmightyFilterPad;

struct _GstAlmightyFilter {
  GstElement element;

  /* properties */
  guint gpu_id;
  gchar* config;
  float threshold;

  gboolean deal_meta;

  guint pad_count;
  gboolean initialized;
  // GMutex mutex;
  // Model *net;
  GstAlmightyFilterFramework fw; /**< Internal properties for almighty-filter */
};

struct _GstAlmightyFilterClass {
  GstElementClass parent_class;
};

GST_ELEMENT_REGISTER_DECLARE(flow_c_extension);

G_END_DECLS
/* */
#endif /* __FLOW_C_EXTENSION_H__ */
