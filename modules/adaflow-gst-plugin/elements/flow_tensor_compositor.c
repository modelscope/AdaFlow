/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common.h"
#include "flow_tensor_compositor.h"

GST_DEBUG_CATEGORY_STATIC(gst_flowtensorcompositor_debug);
#define GST_CAT_DEFAULT gst_flowtensorcompositor_debug

#define FORMATS " { RGB, I420, NV12, NV21 } "

static GstStaticPadTemplate src_factory =
       GST_STATIC_PAD_TEMPLATE("src", GST_PAD_SRC, GST_PAD_ALWAYS,
                               GST_STATIC_CAPS(GST_VIDEO_CAPS_MAKE(FORMATS)));

static GstStaticPadTemplate sink_factory =
       GST_STATIC_PAD_TEMPLATE("sink_%u", GST_PAD_SINK, GST_PAD_REQUEST,
                               GST_STATIC_CAPS(GST_VIDEO_CAPS_MAKE(FORMATS)));

static void gst_flowtensorcompositor_child_proxy_init(gpointer g_iface,
                                            gpointer iface_data);

#define GST_TYPE_FLOWTENSORCOMPOSITOR_OPERATOR (gst_flowtensorcompositor_operator_get_type())
static GType gst_flowtensorcompositor_operator_get_type(void) {
   static GType flowtensorcompositor_operator_type = 0;

   static const GEnumValue flowtensorcompositor_operator[] = {
           {FLOWTENSORCOMPOSITOR_OPERATOR_SEGMENT, "Segment", "segment"},
           {0, NULL, NULL},
   };

   if (!flowtensorcompositor_operator_type) {
       flowtensorcompositor_operator_type =
               g_enum_register_static("FlowtensorcompositorOperator", flowtensorcompositor_operator);
   }
   return flowtensorcompositor_operator_type;
}

static const gchar* gst_flowtensorcompositor_bgcolor_string[] = {
       [GX_RED] = "red",
       [GX_BLACK] = "black",
       [GX_WHITE] = "white",
       [GX_COLORUNKNOWN] = NULL};

#define DEFAULT_OPERATOR FLOWTENSORCOMPOSITOR_OPERATOR_SEGMENT
#define PROP_BGCOLOR_DEFAULT ""
#define XCLAMP(value, minValue, maxValue) \
 ((value) < (minValue) ? (minValue)      \
                       : ((value) > (maxValue) ? (maxValue) : (value)))
static void gst_flowtensorcompositor_segment_blend(
       guint8* dest, gint dstride, gint d_pstride, guint8* src1, gint sstride1,
       gint s1_pstride, guint8* src2, gint sstride2, gint s2_pstride, gint width,
       gint height, gint r_color, gint g_color, gint b_color);
static background_color gst_flowtensorcompositor_get_bgcolor(const gchar* str);
enum {
   PROP_PAD_0,
};

G_DEFINE_TYPE(FlowtensorcompositorPad, gst_flowtensorcompositor_pad,
             GST_TYPE_VIDEO_AGGREGATOR_PARALLEL_CONVERT_PAD);

static void gst_flowtensorcompositor_pad_get_property(GObject* object, guint prop_id,
                                            GValue* value, GParamSpec* pspec) {
   FlowtensorcompositorPad* pad = GST_FLOWTENSORCOMPOSITOR_PAD(object);

   switch (prop_id) {
       default:
           G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
           break;
   }
}

static void gst_flowtensorcompositor_pad_set_property(GObject* object, guint prop_id,
                                            const GValue* value,
                                            GParamSpec* pspec) {
   FlowtensorcompositorPad* pad = GST_FLOWTENSORCOMPOSITOR_PAD(object);

   switch (prop_id) {
       default:
           G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
           break;
   }
}

static void gst_flowtensorcompositor_pad_prepare_frame_start(
       GstVideoAggregatorPad* pad, GstVideoAggregator* vagg, GstBuffer* buffer,
       GstVideoFrame* prepared_frame) {
   FlowtensorcompositorPad* cpad = GST_FLOWTENSORCOMPOSITOR_PAD(pad);
   gint width, height;
   gboolean frame_obscured = FALSE;
   GList* l;
   /* The rectangle representing this frame, clamped to the video's boundaries.
  * Due to the clamping, this is different from the frame width/height above.
  */
   GstVideoRectangle frame_rect;

   GST_OBJECT_LOCK(vagg);
   /* Check if this frame is obscured by a higher-zorder frame
  * TODO: Also skip a frame if it's obscured by a combination of
  * higher-zorder frames */
   l = g_list_find(GST_ELEMENT(vagg)->sinkpads, pad);
   /* The pad might've just been removed */
   if (l) l = l->next;
   for (; l; l = l->next) {
       GstBuffer* pad_buffer;

       pad_buffer = gst_video_aggregator_pad_get_current_buffer(
               GST_VIDEO_AGGREGATOR_PAD(l->data));

       if (pad_buffer == NULL) continue;

       if (gst_buffer_get_size(pad_buffer) == 0 &&
           GST_BUFFER_FLAG_IS_SET(pad_buffer, GST_BUFFER_FLAG_GAP)) {
           continue;
       }
   }
   GST_OBJECT_UNLOCK(vagg);

   if (frame_obscured) return;

   GST_VIDEO_AGGREGATOR_PAD_CLASS(gst_flowtensorcompositor_pad_parent_class)
           ->prepare_frame_start(pad, vagg, buffer, prepared_frame);
}

static void gst_flowtensorcompositor_pad_class_init(FlowtensorcompositorPadClass* klass) {
   GObjectClass* gobject_class = (GObjectClass*)klass;
   GstVideoAggregatorPadClass* vaggpadclass = (GstVideoAggregatorPadClass*)klass;
   GstVideoAggregatorConvertPadClass* vaggcpadclass =
           (GstVideoAggregatorConvertPadClass*)klass;

   gobject_class->set_property = gst_flowtensorcompositor_pad_set_property;
   gobject_class->get_property = gst_flowtensorcompositor_pad_get_property;

   vaggpadclass->prepare_frame_start =
           GST_DEBUG_FUNCPTR(gst_flowtensorcompositor_pad_prepare_frame_start);
}

static void gst_flowtensorcompositor_pad_init(FlowtensorcompositorPad* compo_pad) {
   // compo_pad->op = DEFAULT_PAD_OPERATOR;
}

/* Flowtensorcompositor */
#define DEFAULT_BACKGROUND FLOWTENSORCOMPOSITOR_BACKGROUND_CHECKER
#define DEFAULT_ZERO_SIZE_IS_UNSCALED TRUE
#define DEFAULT_MAX_THREADS 0

enum {
   PROP_0,
   PROP_OPERATOR,
   PROP_BGCOLOR,
};

static void gst_flowtensorcompositor_get_property(GObject* object, guint prop_id,
                                        GValue* value, GParamSpec* pspec) {
   Flowtensorcompositor* self = GST_FLOWTENSORCOMPOSITOR(object);

   switch (prop_id) {
       case PROP_OPERATOR:
           g_value_set_enum(value, self->op);
           break;
       case PROP_BGCOLOR:
           g_value_set_string(value, self->bgcolor);
           break;
       default:
           G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
           break;
   }
}

static void gst_flowtensorcompositor_set_property(GObject* object, guint prop_id,
                                        const GValue* value,
                                        GParamSpec* pspec) {
   Flowtensorcompositor* self = GST_FLOWTENSORCOMPOSITOR(object);

   switch (prop_id) {
       case PROP_OPERATOR:
           self->op = g_value_get_enum(value);
           break;
       case PROP_BGCOLOR:
           self->bgcolor = g_value_dup_string(value);
           self->bg_color = gst_flowtensorcompositor_get_bgcolor(self->bgcolor);
           break;
       default:
           G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
           break;
   }
}

#define gst_flowtensorcompositor_parent_class parent_class

G_DEFINE_TYPE_WITH_CODE(
       Flowtensorcompositor, gst_flowtensorcompositor, GST_TYPE_VIDEO_AGGREGATOR,
       G_IMPLEMENT_INTERFACE(GST_TYPE_CHILD_PROXY,
                             gst_flowtensorcompositor_child_proxy_init));

GST_ELEMENT_REGISTER_DEFINE(flow_tensor_compositor, "flow_tensor_compositor", GST_RANK_PRIMARY + 1,
                           GST_TYPE_FLOWTENSORCOMPOSITOR);

static gboolean set_functions(Flowtensorcompositor* self, const GstVideoInfo* info) {
   gboolean ret = FALSE;

   switch (GST_VIDEO_INFO_FORMAT(info)) {
       case GST_VIDEO_FORMAT_RGB:

           ret = TRUE;
           break;
       default:
           break;
   }

   return ret;
}

static gboolean _negotiated_caps(GstAggregator* agg, GstCaps* caps) {
   Flowtensorcompositor* flowtensorcompositor = GST_FLOWTENSORCOMPOSITOR(agg);
   GstVideoAggregator* vagg = GST_VIDEO_AGGREGATOR(agg);
   GstVideoInfo v_info;
   guint n_threads;

   GST_DEBUG_OBJECT(agg, "Negotiated caps %" GST_PTR_FORMAT, caps);

   if (!gst_video_info_from_caps(&v_info, caps)) return FALSE;

   if (!set_functions(flowtensorcompositor, &v_info)) {
       GST_ERROR_OBJECT(agg, "Failed to setup vfuncs");
       return FALSE;
   }

   return GST_AGGREGATOR_CLASS(parent_class)->negotiated_src_caps(agg, caps);
}

static GstFlowReturn gst_flowtensorcompositor_aggregate_frames(GstVideoAggregator* vagg,
                                                     GstBuffer* outbuf) {
   Flowtensorcompositor* flowtensorcompositor = GST_FLOWTENSORCOMPOSITOR(vagg);
   GList* l;
   GstVideoFrame out_frame, *outframe;
   guint i, n_pads = 0;

   GstVideoFrame* prepared1_frame;
   GstVideoFrame* prepared2_frame;

   if (!gst_video_frame_map(&out_frame, &vagg->info, outbuf, GST_MAP_WRITE)) {
       GST_WARNING_OBJECT(vagg, "Could not map output buffer");
       return GST_FLOW_ERROR;
   }

   outframe = &out_frame;

   guint height = GST_VIDEO_FRAME_HEIGHT(outframe);
   guint width = GST_VIDEO_FRAME_WIDTH(outframe);

   GST_OBJECT_LOCK(vagg);
   for (l = GST_ELEMENT(vagg)->sinkpads; l; l = l->next) {
       GstVideoAggregatorPad* pad = l->data;

       if (n_pads == 0) {
           prepared1_frame = gst_video_aggregator_pad_get_prepared_frame(pad);
       } else {
           prepared2_frame = gst_video_aggregator_pad_get_prepared_frame(pad);
       }
       n_pads++;
   }

   gint r_color, g_color, b_color;

   switch (flowtensorcompositor->bg_color) {
       case GX_RED: {
           r_color = 255;
           g_color = 0;
           b_color = 0;
           break;
       }

       case GX_BLACK: {
           r_color = 0;
           g_color = 0;
           b_color = 0;
           break;
       }

       case GX_WHITE: {
           r_color = 255;
           g_color = 255;
           b_color = 255;
           break;
       }

       default:
           GST_ERROR_OBJECT(flowtensorcompositor, "Cannot identify background color\n");
   }

   guint8* dest = GST_VIDEO_FRAME_PLANE_DATA(outframe, 0);
   gint dstride = GST_VIDEO_FRAME_PLANE_STRIDE(outframe, 0);
   gint d_pstride = GST_VIDEO_FRAME_COMP_PSTRIDE(outframe, 0);

   guint8* src1 = GST_VIDEO_FRAME_PLANE_DATA(prepared1_frame, 0);
   gint s1stride = GST_VIDEO_FRAME_PLANE_STRIDE(prepared1_frame, 0);
   gint s1_pstride = GST_VIDEO_FRAME_COMP_PSTRIDE(prepared1_frame, 0);

   guint8* src2 = GST_VIDEO_FRAME_PLANE_DATA(prepared2_frame, 0);
   gint s2stride = GST_VIDEO_FRAME_PLANE_STRIDE(prepared2_frame, 0);
   gint s2_pstride = GST_VIDEO_FRAME_COMP_PSTRIDE(prepared2_frame, 0);

   gst_flowtensorcompositor_segment_blend(dest, dstride, d_pstride, src1, s1stride,
                                 s1_pstride, src2, s2stride, s2_pstride, width,
                                 height, r_color, g_color, b_color);

   GST_OBJECT_UNLOCK(vagg);

   gst_video_frame_unmap(outframe);

   return GST_FLOW_OK;
}

static GstPad* gst_flowtensorcompositor_request_new_pad(GstElement* element,
                                              GstPadTemplate* templ,
                                              const gchar* req_name,
                                              const GstCaps* caps) {
   GstPad* newpad;

   newpad = (GstPad*)GST_ELEMENT_CLASS(parent_class)
                    ->request_new_pad(element, templ, req_name, caps);

   if (newpad == NULL) goto could_not_create;

   return newpad;

could_not_create : {
   GST_DEBUG_OBJECT(element, "could not create/add pad");
   return NULL;
}
}

static void gst_flowtensorcompositor_release_pad(GstElement* element, GstPad* pad) {
   Flowtensorcompositor* flowtensorcompositor;

   flowtensorcompositor = GST_FLOWTENSORCOMPOSITOR(element);

   GST_DEBUG_OBJECT(flowtensorcompositor, "release pad %s:%s", GST_DEBUG_PAD_NAME(pad));

   GST_ELEMENT_CLASS(parent_class)->release_pad(element, pad);
}

static gboolean _sink_query(GstAggregator* agg, GstAggregatorPad* bpad,
                           GstQuery* query) {
   switch (GST_QUERY_TYPE(query)) {
       case GST_QUERY_ALLOCATION: {
           GstCaps* caps;
           GstVideoInfo info;
           GstBufferPool* pool;
           guint size;
           GstStructure* structure;

           gst_query_parse_allocation(query, &caps, NULL);

           if (caps == NULL) return FALSE;

           if (!gst_video_info_from_caps(&info, caps)) return FALSE;

           size = GST_VIDEO_INFO_SIZE(&info);

           pool = gst_video_buffer_pool_new();

           structure = gst_buffer_pool_get_config(pool);
           gst_buffer_pool_config_set_params(structure, caps, size, 0, 0);

           if (!gst_buffer_pool_set_config(pool, structure)) {
               gst_object_unref(pool);
               return FALSE;
           }

           gst_query_add_allocation_pool(query, pool, size, 0, 0);
           gst_object_unref(pool);
           gst_query_add_allocation_meta(query, GST_VIDEO_META_API_TYPE, NULL);

           return TRUE;
       }
       default:
           return GST_AGGREGATOR_CLASS(parent_class)->sink_query(agg, bpad, query);
   }
}

static void gst_flowtensorcompositor_finalize(GObject* object) {
   Flowtensorcompositor* flowtensorcompositor = GST_FLOWTENSORCOMPOSITOR(object);
   G_OBJECT_CLASS(parent_class)->finalize(object);
}

/* GObject boilerplate */
static void gst_flowtensorcompositor_class_init(FlowtensorcompositorClass* klass) {
   GObjectClass* gobject_class = (GObjectClass*)klass;
   GstElementClass* gstelement_class = (GstElementClass*)klass;
   GstVideoAggregatorClass* videoaggregator_class =
           (GstVideoAggregatorClass*)klass;
   GstAggregatorClass* agg_class = (GstAggregatorClass*)klass;

   gobject_class->get_property = gst_flowtensorcompositor_get_property;
   gobject_class->set_property = gst_flowtensorcompositor_set_property;
   gobject_class->finalize = gst_flowtensorcompositor_finalize;

   gstelement_class->request_new_pad =
           GST_DEBUG_FUNCPTR(gst_flowtensorcompositor_request_new_pad);
   gstelement_class->release_pad =
           GST_DEBUG_FUNCPTR(gst_flowtensorcompositor_release_pad);
   agg_class->sink_query = _sink_query;
   // agg_class->fixate_src_caps = _fixate_caps;
   agg_class->negotiated_src_caps = _negotiated_caps;
   videoaggregator_class->aggregate_frames = gst_flowtensorcompositor_aggregate_frames;

   gst_element_class_add_static_pad_template_with_gtype(
           gstelement_class, &src_factory, GST_TYPE_AGGREGATOR_PAD);
   gst_element_class_add_static_pad_template_with_gtype(
           gstelement_class, &sink_factory, GST_TYPE_FLOWTENSORCOMPOSITOR_PAD);

   g_object_class_install_property(
           gobject_class, PROP_OPERATOR,
           g_param_spec_enum(
                   "operator", "Operator",
                   "Blending operator to use for blending this pad over the previous "
                   "ones",
                   GST_TYPE_FLOWTENSORCOMPOSITOR_OPERATOR, DEFAULT_OPERATOR,
                   G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));

   g_object_class_install_property(
           gobject_class, PROP_BGCOLOR,
           g_param_spec_string("bgcolor", "Bgcolor", "Segment background color",
                               PROP_BGCOLOR_DEFAULT, G_PARAM_WRITABLE));

   gst_element_class_set_static_metadata(gstelement_class, "flow_tensor_compositor",
                                         "Filter/Editor/Video/flow_tensor_compositor",
                                         "Composite multiple video streams",
                                         "x.com, "
                                         "alibaba-damo");

   gst_type_mark_as_plugin_api(GST_TYPE_FLOWTENSORCOMPOSITOR_PAD, 0);
   gst_type_mark_as_plugin_api(GST_TYPE_FLOWTENSORCOMPOSITOR_OPERATOR, 0);
}

static void gst_flowtensorcompositor_init(Flowtensorcompositor* self) {
   self->op = DEFAULT_OPERATOR;
   self->bg_color = GX_RED;
}

/* GstChildProxy implementation */
static GObject* gst_flowtensorcompositor_child_proxy_get_child_by_index(
       GstChildProxy* child_proxy, guint index) {
   Flowtensorcompositor* flowtensorcompositor = GST_FLOWTENSORCOMPOSITOR(child_proxy);
   GObject* obj = NULL;

   GST_OBJECT_LOCK(flowtensorcompositor);
   obj = g_list_nth_data(GST_ELEMENT_CAST(flowtensorcompositor)->sinkpads, index);
   if (obj) gst_object_ref(obj);
   GST_OBJECT_UNLOCK(flowtensorcompositor);

   return obj;
}

static guint gst_flowtensorcompositor_child_proxy_get_children_count(
       GstChildProxy* child_proxy) {
   guint count = 0;
   Flowtensorcompositor* flowtensorcompositor = GST_FLOWTENSORCOMPOSITOR(child_proxy);

   GST_OBJECT_LOCK(flowtensorcompositor);
   count = GST_ELEMENT_CAST(flowtensorcompositor)->numsinkpads;
   GST_OBJECT_UNLOCK(flowtensorcompositor);
   GST_INFO_OBJECT(flowtensorcompositor, "Children Count: %d", count);

   return count;
}

static void gst_flowtensorcompositor_child_proxy_init(gpointer g_iface,
                                            gpointer iface_data) {
   GstChildProxyInterface* iface = g_iface;

   iface->get_child_by_index = gst_flowtensorcompositor_child_proxy_get_child_by_index;
   iface->get_children_count = gst_flowtensorcompositor_child_proxy_get_children_count;
}

////////////////////////////////////////////////////////////////////////////////////
static void gst_flowtensorcompositor_segment_blend(
       guint8* dest, gint dstride, gint d_pstride, guint8* src1, gint s1stride,
       gint s1_pstride, guint8* src2, gint s2stride, gint s2_pstride, gint width,
       gint height, gint r_color, gint g_color, gint b_color) {
   gint h, w;
   for (h = 0; h < height; ++h) {
       for (w = 0; w < width; ++w) {
           guint8* pixel = dest + h * dstride + w * d_pstride;
           guint8* pixel_in1 = src1 + h * s1stride + w * s1_pstride;
           guint8* pixel_in2 = src2 + h * s2stride + w * s2_pstride;

           float seg_pix0 = pixel_in2[0] / 255.0;
           float seg_pix1 = pixel_in2[1] / 255.0;
           float seg_pix2 = pixel_in2[2] / 255.0;

           pixel[0] = XCLAMP(
                   (int)(pixel_in1[0] * seg_pix0 + r_color * (1.0 - seg_pix0)), 0, 255);
           pixel[1] = XCLAMP(
                   (int)(pixel_in1[1] * seg_pix1 + g_color * (1.0 - seg_pix1)), 0, 255);
           pixel[2] = XCLAMP(
                   (int)(pixel_in1[2] * seg_pix2 + b_color * (1.0 - seg_pix2)), 0, 255);
       }
   }
}

static background_color gst_flowtensorcompositor_get_bgcolor(const gchar* str) {
   int index;

   index = find_key_strv(gst_flowtensorcompositor_bgcolor_string, str);

   return (index < 0) ? GX_COLORUNKNOWN : index;
}

///////////////////////////////////////////////////////////////////////////////////