/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

#include "flow_tensor_resize.h"

#include "meta_tensor.h"

#define gst_tensorresize_parent_class parent_class

static GstStaticPadTemplate sink_factory =
       GST_STATIC_PAD_TEMPLATE("sink_%u", GST_PAD_SINK, GST_PAD_REQUEST,
                               GST_STATIC_CAPS("video/x-raw,format=RGB"));

static GstStaticPadTemplate src_factory =
       GST_STATIC_PAD_TEMPLATE("src_%u", GST_PAD_SRC, GST_PAD_SOMETIMES,
                               GST_STATIC_CAPS("video/x-raw,format=RGB"));

enum {
   PROP_0,
   PROP_RESIZED_WIDTH,
   PROP_RESIZED_HEIGHT,
};

G_DEFINE_TYPE(GstTensorResize, gst_tensorresize, GST_TYPE_ELEMENT);
GST_ELEMENT_REGISTER_DEFINE(flow_tensor_resize, "flow_tensor_resize", GST_RANK_NONE,
                           GST_TYPE_TENSORRESIZE);

#define PROP_RESIZED_WIDTH_DEFAULT 1280
#define PROP_RESIZED_HEIGHT_DEFAULT 720

template <typename T1, typename T2>
int _resize_bilinear_nhwc(const T1* src, int src_n, int src_c, int src_h,
                         int src_w, signed short* x_left_crood,
                         float* x_right_scale, signed short* y_up_crood,
                         float* y_down_scale, T2* dst, int dst_n, int dst_c,
                         int dst_h, int dst_w, int numThred, int idxThred);

static void gst_tensorresize_set_property(GObject* object, guint prop_id,
                                         const GValue* value,
                                         GParamSpec* pspec) {
   // g_print("set_property() \n");
   GstTensorResize* filter = GST_TENSORRESIZE(object);
   // g_print("[gst_tensorresize_set_property] config = %s \n", filter->config);

   switch (prop_id) {
       case PROP_RESIZED_WIDTH:
           filter->resized_width = g_value_get_uint(value);
           break;

       case PROP_RESIZED_HEIGHT:
           filter->resized_height = g_value_get_uint(value);
           break;

       default:
           G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
           break;
   }

   // g_print("[gst_tensorresize_set_property] config = %s \n", filter->config);
}

static void gst_tensorresize_get_property(GObject* object, guint prop_id,
                                         GValue* value, GParamSpec* pspec) {
   GstTensorResize* filter = GST_TENSORRESIZE(object);

   switch (prop_id) {
       case PROP_RESIZED_WIDTH:
           g_value_set_uint(value, filter->resized_width);
           break;

       case PROP_RESIZED_HEIGHT:
           g_value_set_uint(value, filter->resized_height);
           break;

       default:
           G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
           break;
   }
}

static void gst_tensorresize_finalize(GObject* object) {
   GstTensorResize* filter = GST_TENSORRESIZE(object);

   if (filter->reserved_data) {
       delete[] filter->reserved_data;
       filter->reserved_data = NULL;
   }

   G_OBJECT_CLASS(parent_class)->finalize(object);
}

static gboolean gst_tensorresize_sink_event(GstPad* pad, GstObject* parent,
                                           GstEvent* event) {
   // g_print("sink_event() \n");
   GstTensorResize* filter = GST_TENSORRESIZE(parent);

   GstTensorResizePad* filterpad =
           (GstTensorResizePad*)gst_pad_get_element_private(pad);

   if (GST_EVENT_TYPE(event) == GST_EVENT_CAPS) {
       GstCaps* caps;
       gst_event_parse_caps(event, &caps);

       gst_video_info_from_caps(&filterpad->video_info, caps);

       gst_pad_set_caps(filterpad->srcpad, caps);
       return TRUE;
   }

   return gst_pad_event_default(pad, parent, event);
}

static GstFlowReturn gst_tensorresize_chain(GstPad* pad, GstObject* parent,
                                           GstBuffer* buf) {
   // g_print("chain() \n");
   GstTensorResize* filter = GST_TENSORRESIZE(parent);
   GstTensorResizePad* filterpad =
           (GstTensorResizePad*)gst_pad_get_element_private(pad);
   guint i, j, k;
   guint src_batch, src_channel, src_height, src_width, src_size, src_maxsize;
   guint dst_batch, dst_channel, dst_height, dst_width, dst_size, dst_maxsize;
   VARIABLE_TYPE vtype;

   // if (!filter->initialized)
   {
       GstXtensor* meta = GST_XTENSOR_GET(buf);
       vtype = meta->tensor_info.type;

       // input shape
       src_batch = meta->tensor_info.N;
       src_channel = meta->tensor_info.C;
       src_height = meta->tensor_info.H;
       src_width = meta->tensor_info.W;
       src_size = meta->tensor_info.size;
       src_maxsize = meta->tensor_info.maxsize;

       dst_batch = meta->tensor_info.N;
       dst_channel = meta->tensor_info.C;
       dst_width = filter->resized_width > 0 ? filter->resized_width : src_width;
       dst_height =
               filter->resized_height > 0 ? filter->resized_height : src_height;
       dst_size = dst_width * dst_height * dst_channel * dst_batch;
       dst_maxsize = dst_size > src_maxsize ? dst_size : src_maxsize;

       // output shape
       meta->tensor_info.device = CPU;
       meta->tensor_info.type = vtype;
       meta->tensor_info.N = dst_batch;
       meta->tensor_info.C = dst_channel;
       meta->tensor_info.H = dst_height;
       meta->tensor_info.W = dst_width;
       meta->tensor_info.size = dst_size;
       meta->tensor_info.maxsize = dst_maxsize;

       // printf("resize in width = %d \n", meta->tensor_info.W);
       // printf("resize in height = %d \n", meta->tensor_info.H);

       // GST_XTENSOR_MALLOC(meta);
       // filter->initialized = TRUE;
   }

   if (src_channel != dst_channel) {
       printf("src_channel != dst_channel \n");
       return GST_FLOW_ERROR;
   }

   if (src_width != dst_width || src_height != dst_height) {
       if (filter->x_left_crood.size() != dst_width ||
           filter->x_right_scale.size() != dst_width) {
           filter->x_left_crood.clear();
           filter->x_left_crood.resize(dst_width);
           filter->x_right_scale.clear();
           filter->x_right_scale.resize(dst_width);
           {
               float scale_x = (float)(src_width - 1) / (dst_width - 1);
               filter->x_left_crood[0] = 0;
               filter->x_right_scale[0] = 0;
               filter->x_left_crood[dst_width - 1] = src_width - 2;
               filter->x_right_scale[dst_width - 1] = 1;
               for (int j = 1; j < dst_width - 1; j++) {
                   float x_frac = j * scale_x;
                   int jm = int(x_frac);
                   filter->x_left_crood[j] = jm;
                   filter->x_right_scale[j] = x_frac - jm;
               }
           }
       }

       if (filter->y_up_crood.size() != dst_height ||
           filter->y_down_scale.size() != dst_height) {
           filter->y_up_crood.clear();
           filter->y_up_crood.resize(dst_height);
           filter->y_down_scale.clear();
           filter->y_down_scale.resize(dst_height);
           {
               float scale_y = (float)(src_height - 1) / (dst_height - 1);
               filter->y_up_crood[0] = 0;
               filter->y_down_scale[0] = 0;
               filter->y_up_crood[dst_height - 1] = src_height - 2;
               filter->y_down_scale[dst_height - 1] = 1;
               for (int j = 1; j < dst_height - 1; j++) {
                   float y_frac = j * scale_y;
                   int jm = int(y_frac);
                   filter->y_up_crood[j] = jm;
                   filter->y_down_scale[j] = y_frac - jm;
               }
           }
       }
   }

   if (dst_maxsize == src_maxsize) {
       if (!filter->reserved_data) {
           filter->reserved_data = new gfloat[dst_size];
       }

       GstMapInfo info;
       gst_buffer_map(buf, &info, GST_MAP_READ);
       const gfloat* psrc = (gfloat*)info.data;

       if (src_width != dst_width || src_height != dst_height) {
           int ret = _resize_bilinear_nhwc<gfloat, gfloat>(
                   psrc, src_batch, src_channel, src_height, src_width,
                   filter->x_left_crood.data(), filter->x_right_scale.data(),
                   filter->y_up_crood.data(), filter->y_down_scale.data(),
                   filter->reserved_data, dst_batch, dst_channel, dst_height, dst_width,
                   0, 0);

           memcpy((gfloat*)info.data, filter->reserved_data,
                  dst_size * sizeof(gfloat));
       }

       gst_buffer_unmap(buf, &info);
       return gst_pad_push(filterpad->srcpad, buf);
   } else {
       GstMapInfo src_info, dest_info;
       gsize frame_size = dst_maxsize * tensor_element_size[vtype];
       gst_buffer_map(buf, &src_info, GST_MAP_READ);

       GstBuffer* inbuf = gst_buffer_new_and_alloc(frame_size);
       gst_buffer_memset(inbuf, 0, 0, frame_size);
       gst_buffer_map(inbuf, &dest_info, GST_MAP_WRITE);

       const gfloat* psrc = (gfloat*)src_info.data;
       gfloat* pdst = (gfloat*)dest_info.data;

       int ret = _resize_bilinear_nhwc<gfloat, gfloat>(
               psrc, src_batch, src_channel, src_height, src_width,
               filter->x_left_crood.data(), filter->x_right_scale.data(),
               filter->y_up_crood.data(), filter->y_down_scale.data(), pdst, dst_batch,
               dst_channel, dst_height, dst_width, 0, 0);

       gst_buffer_unmap(buf, &src_info);
       gst_buffer_unmap(inbuf, &dest_info);

       /** copy timestamps */
       gst_buffer_copy_into(inbuf, buf, GST_BUFFER_COPY_METADATA, 0, -1);

       if (inbuf != buf) {
           gst_buffer_unref(buf);
       }

       return gst_pad_push(filterpad->srcpad, inbuf);
   }

   // // post process
   // GstMapInfo info;
   // gst_buffer_map(buf, &info, GST_MAP_READ);

   // gfloat *prgb = (gfloat *)info.data;
   // for (i = 0; i < height; i++)
   // {
   //   for (j = 0; j < width; j++)
   //   {
   //     guint idx = i * width * 3 + j * 3;

   //     prgb[idx + 0] = 0.5f * prgb[idx + 0] + 255 * 0.5f;
   //     prgb[idx + 1] = 0.5f * prgb[idx + 1] + 0 * 0.5f;
   //     prgb[idx + 2] = 0.5f * prgb[idx + 2] + 0 * 0.5f;
   //   }
   // }

   // gst_buffer_unmap(buf, &info);

   // // g_mutex_lock(&filter->mutex);
   // // //todo
   // // g_mutex_unlock(&filter->mutex);

   // return gst_pad_push(filterpad->srcpad, buf);
}

static void gst_tensorresize_init_after_props(GstTensorResize* filter) {
   // g_mutex_init(&filter->mutex);
   // cuda_set_device(filter->gpu_id);
   // filter->Model = load_model(filter->config);
}

static GstPad* gst_tensorresize_request_new_pad(GstElement* element,
                                               GstPadTemplate* templ,
                                               const gchar* name,
                                               const GstCaps* caps) {
   GstTensorResize* filter = GST_TENSORRESIZE(element);

   // if (filter->model == NULL)
   { gst_tensorresize_init_after_props(filter); }

   GstPad* pad = gst_pad_new_from_template(templ, name);
   GstTensorResizePad* filterpad = g_new(GstTensorResizePad, 1);
   filterpad->id = filter->pad_count++;
   // filterpad->image_scaled = make_image(filter->net->w, filter->net->h, 3);
   gst_pad_set_element_private(pad, filterpad);
   gst_pad_set_event_function(pad, gst_tensorresize_sink_event);
   gst_pad_set_chain_function(pad, gst_tensorresize_chain);
   gst_element_add_pad(element, pad);

   gchar* srcname = g_strdup_printf(src_factory.name_template, filterpad->id);
   filterpad->srcpad = gst_pad_new_from_static_template(&src_factory, srcname);
   g_free(srcname);
   gst_element_add_pad(element, filterpad->srcpad);

   return pad;
}

static void gst_tensorresize_class_init(GstTensorResizeClass* klass) {
   // g_print("class_init() \n");
   GObjectClass* object_class = G_OBJECT_CLASS(klass);
   GstElementClass* element_class = (GstElementClass*)klass;

   object_class->set_property = gst_tensorresize_set_property;
   object_class->get_property = gst_tensorresize_get_property;
   object_class->finalize = gst_tensorresize_finalize;

   constexpr auto param_flags =
           static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

   g_object_class_install_property(
           object_class, PROP_RESIZED_WIDTH,
           g_param_spec_uint("width", "width", "resized width", 0, 4096,
                             PROP_RESIZED_WIDTH_DEFAULT, param_flags));

   g_object_class_install_property(
           object_class, PROP_RESIZED_HEIGHT,
           g_param_spec_uint("height", "height", "resized height", 0, 4096,
                             PROP_RESIZED_HEIGHT_DEFAULT, param_flags));

   gst_element_class_set_details_simple(element_class, "flow_tensor_resize",
                                        "adaflow", "flow_tensor_resize",
                                        "AUTHOR_NAME AUTHOR_EMAIL");

   gst_element_class_add_pad_template(element_class,
                                      gst_static_pad_template_get(&src_factory));

   gst_element_class_add_pad_template(
           element_class, gst_static_pad_template_get(&sink_factory));

   element_class->request_new_pad = gst_tensorresize_request_new_pad;
}

static void gst_tensorresize_init(GstTensorResize* filter) {
   // g_print("[tensor bilinear resize] init() \n");
   filter->resized_width = PROP_RESIZED_WIDTH_DEFAULT;
   filter->resized_height = PROP_RESIZED_HEIGHT_DEFAULT;
   filter->pad_count = 0;
   filter->initialized = FALSE;
   filter->reserved_data = NULL;
}

// tensor bilinear resize
template <typename T1, typename T2>
int _resize_bilinear_nhwc(const T1* src, int src_n, int src_c, int src_h,
                         int src_w, signed short* x_left_crood,
                         float* x_right_scale, signed short* y_up_crood,
                         float* y_down_scale, T2* dst, int dst_n, int dst_c,
                         int dst_h, int dst_w, int numThred, int idxThred) {
   if (!src || !dst) {
       return 1;
   }

   if (src_n != dst_n || src_c != dst_c) {
       return 2;
   }

   int src_width = src_w;
   int src_height = src_h;
   int src_channels = src_c;
   int dst_width = dst_w;
   int dst_height = dst_h;
   int dst_channels = dst_c;
   int src_stride = src_width * src_channels;
   int dst_stride = dst_width * dst_channels;

   int y1 = 0;
   int y2 = dst_height;
   if (numThred > 0) {
       y1 = (dst_height / numThred) * idxThred;
       y2 = (dst_height / numThred) * (idxThred + 1);
       if (idxThred == numThred - 1) y2 = dst_height;
   }

   // printf("|coordinate_transformation_mode:  align_corners\n");

   for (int n = 0; n < dst_n; n++) {
       const T1* p_src = src + n * src_h * src_stride;
       T2* p_dst = dst + n * dst_h * dst_stride;

       for (int y = y1; y < y2; y++) {
           float y_frac = y_down_scale[y];
           int im = y_up_crood[y];
           T2* dst_line = p_dst + y * dst_stride;
           const T1* src_line_1 = p_src + im * src_stride;
           const T1* src_line_2 = src_line_1 + src_stride;
           for (int x = 0; x < dst_width; x++) {
               float x_frac = x_right_scale[x];
               int idx1 = x_left_crood[x] * src_channels;
               int idx2 = idx1 + src_channels;
               int idx_dst = x * dst_channels;
               for (int c = 0; c < dst_channels; ++c) {
                   T1 x1 = src_line_1[idx1 + c];
                   T1 x2 = src_line_1[idx2 + c];
                   T1 x11 = src_line_2[idx1 + c];
                   T1 x22 = src_line_2[idx2 + c];
                   T2 y1 = x_frac * (x2 - x1) + x1;
                   T2 y2 = x_frac * (x22 - x11) + x11;
                   T2 val = y_frac * (y2 - y1) + y1;

                   dst_line[idx_dst + c] = val;
               }
           }
       }
   }

   return 0;
}
