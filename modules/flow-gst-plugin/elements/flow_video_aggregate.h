//
// Created by JingYao on 2023/3/14.
//

#ifndef FLOW_FLOW_VIDEO_AGGREGATE_H
#define FLOW_FLOW_VIDEO_AGGREGATE_H

#include <gst/gst.h>
#include <gst/video/video.h>

G_BEGIN_DECLS

GType gst_flowaggregator_get_type();

#define GST_TYPE_FLOWAGGREGATOR (gst_flowaggregator_get_type())
#define GST_FLOWAGGREGATOR(obj)                               \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_FLOWAGGREGATOR, \
                              GstFlowaggregator))
#define GST_FLOWAGGREGATOR_CLASS(klass)                      \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_FLOWAGGREGATOR, \
                           GstFlowaggregatorClass))
#define GST_IS_FLOWAGGREGATOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_FLOWAGGREGATOR))
#define GST_IS_FLOWAGGREGATOR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_FLOWAGGREGATOR))

typedef struct _GstFlowaggregator GstFlowaggregator;
typedef struct _GstFlowaggregatorClass GstFlowaggregatorClass;

typedef struct {
  guint id;
  GstPad* srcpad;
  GstVideoInfo video_info;
} GstFlowaggregatorPad;

/**
 * @brief GstTensorAggregator data structure.
 */
struct _GstFlowaggregator {
  GstElement element; /**< parent object */

  GstPad* sinkpad; /**< sink pad */
  GstPad* srcpad;  /**< src pad */

  gboolean silent;    /**< true to print minimized log */
  guint frames_in;    /**< number of frames in input buffer */
  guint frames_out;   /**< number of frames in output buffer */
  guint frames_flush; /**< number of frames to flush */

  gboolean tensor_configured; /**< True if already successfully configured
                                 tensor metadata */
  guint pad_count;

  GstAdapter* adapter;
};

/**
 * @brief GstTensorAggregatorClass data structure.
 */
struct _GstFlowaggregatorClass {
  GstElementClass parent_class; /**< parent class */
};

GST_ELEMENT_REGISTER_DECLARE(flowaggregator);

G_END_DECLS



#endif // FLOW_FLOW_VIDEO_AGGREGATE_H
