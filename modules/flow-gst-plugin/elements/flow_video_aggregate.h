/**
* @file flow_video_aggregate.h
* @brief flow_video_aggregate is a plugin to aggregate the frame using GstAdapter
*/

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

/**
 * @brief GstFlowaggregatorPad data structure.
 */
typedef struct {
  guint id;
  GstPad* srcpad;
  GstVideoInfo video_info;
} GstFlowaggregatorPad;

/**
 * @brief GstFlowaggregator data structure.
 */
struct _GstFlowaggregator {
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
 * @brief GstFlowaggregatorClass data structure.
 */
struct _GstFlowaggregatorClass {
  GstElementClass parent_class; /**< parent class */
};

GST_ELEMENT_REGISTER_DECLARE(flow_video_aggregate);

G_END_DECLS

#endif // FLOW_FLOW_VIDEO_AGGREGATE_H
