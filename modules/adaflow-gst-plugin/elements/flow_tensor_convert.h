#ifndef ADAFLOW_FLOW_TENSOR_CONVERT_H
#define ADAFLOW_FLOW_TENSOR_CONVERT_H

#include <gst/gst.h>
#include <gst/video/video.h>

#include "meta_tensor.h"

G_BEGIN_DECLS
/* */
GType gst_tensorconvert_get_type();

#define GST_TYPE_TENSORCONVERT (gst_tensorconvert_get_type())

#define GST_TENSORCONVERT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TENSORCONVERT, GstTensorconvert))
#define GST_TENSORCONVERT_CLASS(klass)                      \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TENSORCONVERT, \
                           GstTensorconvertClass))

#define GST_IS_TENSORCONVERT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TENSORCONVERT))
#define GST_IS_TENSORCONVERT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TENSORCONVERT))

typedef struct _GstTensorconvert GstTensorconvert;
typedef struct _GstTensorconvertClass GstTensorconvertClass;

typedef struct {
    guint id;
    GstPad* srcpad;
    GstVideoInfo video_info;
} GstTensorconvertPad;

/**
 * @brief Internal data structure for tensorconvert instances.
 */
struct _GstTensorconvert {
    GstElement element;

    /* properties */
    guint device;
    guint is_uint8;
    guint pad_count;
    gboolean initialized;

    gchar* text;
};

/**
 * @brief GstTensorConvertClass data structure.
 */
struct _GstTensorconvertClass {
    GstElementClass parent_class;
};

GST_ELEMENT_REGISTER_DECLARE(flow_tensor_convert);

G_END_DECLS
/* */


#endif//ADAFLOW_FLOW_TENSOR_CONVERT_H
