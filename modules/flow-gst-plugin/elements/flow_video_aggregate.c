//
// Created by JingYao on 2023/3/14.
//

#include "flow_video_aggregate.h"

#define gst_flowaggregator_parent_class parent_class

#define GST_CAP_DEFAULT GST_VIDEO_CAPS_MAKE("{ RGB, I420, NV12, NV21 }")

static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE(
    "sink_%u", GST_PAD_SINK, GST_PAD_REQUEST, GST_STATIC_CAPS(GST_CAP_DEFAULT));

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE(
    "src_%u", GST_PAD_SRC, GST_PAD_SOMETIMES, GST_STATIC_CAPS(GST_CAP_DEFAULT));

enum {
  PROP_0,
  PROP_FRAMES_IN,
  PROP_FRAMES_OUT,
  PROP_FRAMES_FLUSH,
};

#define DEFAULT_FRAMES_IN 1
#define DEFAULT_FRAMES_OUT 1
#define DEFAULT_FRAMES_FLUSH 0

G_DEFINE_TYPE(GstFlowaggregator, gst_flowaggregator, GST_TYPE_ELEMENT);
GST_ELEMENT_REGISTER_DEFINE(flowaggregator, "flowaggregator", GST_RANK_NONE,
                            GST_TYPE_FLOWAGGREGATOR);

static void gst_tensor_aggregator_reset(GstFlowaggregator* self);

static void gst_flowaggregator_set_property(GObject* object, guint prop_id,
                                              const GValue* value,
                                              GParamSpec* pspec) {
  // g_print("set_property() \n");
  GstFlowaggregator* self = GST_FLOWAGGREGATOR(object);
  // g_print("[gst_flowaggregator_set_property] config = %s \n",
  // filter->config);

  switch (prop_id) {
  case PROP_FRAMES_IN:
    self->frames_in = g_value_get_uint(value);
    break;
  case PROP_FRAMES_OUT:
    self->frames_out = g_value_get_uint(value);
    break;
  case PROP_FRAMES_FLUSH:
    self->frames_flush = g_value_get_uint(value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }

  // g_print("[gst_flowaggregator_set_property] config = %s \n",
  // filter->config);
}

static void gst_flowaggregator_get_property(GObject* object, guint prop_id,
                                              GValue* value,
                                              GParamSpec* pspec) {
  // g_print("get_property() \n");
  GstFlowaggregator* self = GST_FLOWAGGREGATOR(object);
  // g_print("[gst_flowaggregator_get_property] config = %s \n",
  // filter->config);

  switch (prop_id) {
  case PROP_FRAMES_IN:
    g_value_set_uint(value, self->frames_in);
    break;
  case PROP_FRAMES_OUT:
    g_value_set_uint(value, self->frames_out);
    break;
  case PROP_FRAMES_FLUSH:
    g_value_set_uint(value, self->frames_flush);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }

  // g_print("[gst_flowaggregator_get_property] config = %s \n",
  // filter->config);
}

static gboolean gst_flowaggregator_sink_event(GstPad* pad, GstObject* parent,
                                                GstEvent* event) {
  // g_print("sink_event() \n");
  GstFlowaggregator* filter = GST_FLOWAGGREGATOR(parent);

  GstFlowaggregatorPad* filterpad =
      (GstFlowaggregatorPad*)gst_pad_get_element_private(pad);

  if (GST_EVENT_TYPE(event) == GST_EVENT_CAPS) {
    GstCaps* caps;
    gst_event_parse_caps(event, &caps);

    gst_video_info_from_caps(&filterpad->video_info, caps);

    gst_pad_set_caps(filterpad->srcpad, caps);
    return TRUE;
  }

  return gst_pad_event_default(pad, parent, event);
}

static GstFlowReturn gst_flowaggregator_chain(GstPad* pad, GstObject* parent,
                                                GstBuffer* buf) {
  GstFlowaggregator* self;
  GstFlowaggregatorPad* filterpad = gst_pad_get_element_private(pad);
  GstFlowReturn ret = GST_FLOW_OK;
  GstAdapter* adapter;
  gsize avail, buf_size, frame_size, out_size;
  guint frames_in, frames_out, frames_flush;
  GstClockTime duration;

  self = GST_FLOWAGGREGATOR(parent);
  guint i, j, k, batch, channel, height, width, size, maxsize;

  buf_size = gst_buffer_get_size(buf);
  g_return_val_if_fail(buf_size > 0, GST_FLOW_ERROR);

  frames_in = self->frames_in;
  frames_out = self->frames_out;
  frames_flush = self->frames_flush;
  frame_size = buf_size / frames_in;

  if (frames_in == frames_out) {
    return gst_pad_push(filterpad->srcpad, buf);
  }

  adapter = self->adapter;

  duration = GST_BUFFER_DURATION(buf);
  if (GST_CLOCK_TIME_IS_VALID(duration)) {
    /** supposed same duration for incoming buffer */
    duration = gst_util_uint64_scale_int(duration, frames_out, frames_in);
  }

  gst_adapter_push(adapter, buf);

  out_size = frame_size * frames_out;
  g_assert(out_size > 0);

  while ((avail = gst_adapter_available(adapter)) >= out_size &&
         ret == GST_FLOW_OK) {
    GstBuffer* outbuf;
    GstClockTime pts, dts;
    guint64 pts_dist, dts_dist;
    gsize flush;

    pts = gst_adapter_prev_pts(adapter, &pts_dist);
    dts = gst_adapter_prev_dts(adapter, &dts_dist);

    /**
     * Update timestamp.
     * If frames-in is larger then frames-out, the same timestamp (pts and dts)
     * would be returned.
     */
    if (frames_in > 1) {
      gint fn, fd;

      fn = filterpad->video_info.fps_n;
      fd = filterpad->video_info.fps_d;

      if (fn > 0 && fd > 0) {
        if (GST_CLOCK_TIME_IS_VALID(pts)) {
          pts += gst_util_uint64_scale_int(pts_dist * fd, GST_SECOND,
                                           fn * frame_size);
        }

        if (GST_CLOCK_TIME_IS_VALID(dts)) {
          dts += gst_util_uint64_scale_int(dts_dist * fd, GST_SECOND,
                                           fn * frame_size);
        }
      }
    }

    outbuf = gst_adapter_get_buffer(adapter, out_size);
    outbuf = gst_buffer_make_writable(outbuf);

    /** set timestamp */
    GST_BUFFER_PTS(outbuf) = pts;
    GST_BUFFER_DTS(outbuf) = dts;
    GST_BUFFER_DURATION(outbuf) = duration;

    ret = gst_pad_push(filterpad->srcpad, outbuf);

    /** flush data */
    if (frames_flush > 0) {
      flush = frame_size * frames_flush;
      if (flush > avail) {
        flush = avail;
      }
    } else {
      flush = out_size;
    }

    gst_adapter_flush(adapter, flush);
  }

  return ret;
}

static void gst_flowaggregator_finalize(GObject* object) {
  GstFlowaggregator* filter = GST_FLOWAGGREGATOR(object);

  G_OBJECT_CLASS(parent_class)->finalize(object);

  gst_adapter_clear(filter->adapter);
  g_object_unref(filter->adapter);
}

static void gst_flowaggregator_init_after_props(GstFlowaggregator* filter) {
  // g_mutex_init(&filter->mutex);
  // cuda_set_device(filter->gpu_id);
  // filter->Model = load_model(filter->config);
}

static GstPad* gst_flowaggregator_request_new_pad(GstElement* element,
                                                    GstPadTemplate* templ,
                                                    const gchar* name,
                                                    const GstCaps* caps) {
  GstFlowaggregator* filter = GST_FLOWAGGREGATOR(element);

  // if (filter->model == NULL)
  { gst_flowaggregator_init_after_props(filter); }

  GstPad* pad = gst_pad_new_from_template(templ, name);
  GstFlowaggregatorPad* filterpad = g_new(GstFlowaggregatorPad, 1);
  filterpad->id = filter->pad_count++;
  // filterpad->image_scaled = make_image(filter->net->w, filter->net->h, 3);
  gst_pad_set_element_private(pad, filterpad);
  gst_pad_set_event_function(pad, gst_flowaggregator_sink_event);
  gst_pad_set_chain_function(pad, gst_flowaggregator_chain);
  gst_element_add_pad(element, pad);

  gchar* srcname = g_strdup_printf(src_factory.name_template, filterpad->id);
  filterpad->srcpad = gst_pad_new_from_static_template(&src_factory, srcname);
  g_free(srcname);
  gst_element_add_pad(element, filterpad->srcpad);

  return pad;
}

static void gst_flowaggregator_class_init(GstFlowaggregatorClass* klass) {
  // g_print("class_init() \n");
  GObjectClass* object_class = G_OBJECT_CLASS(klass);
  GstElementClass* element_class = (GstElementClass*)klass;

  object_class->set_property = gst_flowaggregator_set_property;
  object_class->get_property = gst_flowaggregator_get_property;
  object_class->finalize = gst_flowaggregator_finalize;

  g_object_class_install_property(
      object_class, PROP_FRAMES_IN,
      g_param_spec_uint("frames-in", "Frames in input",
                        "The number of frames in incoming buffer", 1, G_MAXUINT,
                        DEFAULT_FRAMES_IN,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
      object_class, PROP_FRAMES_OUT,
      g_param_spec_uint("frames-out", "Frames in output",
                        "The number of frames in outgoing buffer", 1, G_MAXUINT,
                        DEFAULT_FRAMES_OUT,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
      object_class, PROP_FRAMES_FLUSH,
      g_param_spec_uint("frames-flush", "Frames to flush",
                        "The number of frames to flush (0 to flush all output)",
                        0, G_MAXUINT, DEFAULT_FRAMES_FLUSH,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_details_simple(element_class, "flowaggregator",
                                       "xstreamer", "flowaggregator",
                                       "AUTHOR_NAME AUTHOR_EMAIL");

  gst_element_class_add_pad_template(element_class,
                                     gst_static_pad_template_get(&src_factory));

  gst_element_class_add_pad_template(
      element_class, gst_static_pad_template_get(&sink_factory));

  element_class->request_new_pad = gst_flowaggregator_request_new_pad;
}

static void gst_flowaggregator_init(GstFlowaggregator* self) {
  // g_print("[trt infer] init() \n");
  /** init properties */
  self->frames_in = DEFAULT_FRAMES_IN;
  self->frames_out = DEFAULT_FRAMES_OUT;
  self->frames_flush = DEFAULT_FRAMES_FLUSH;
  self->adapter = gst_adapter_new();
}
