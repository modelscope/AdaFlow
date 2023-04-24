/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/
#include "flow_c_extension.h"

#include <stdlib.h>
#include <string.h>

#include "elements/meta_tensor.h"
#include "flow_extension_common.h"
#include "flow_extension_util.h"

#define gst_almighty_filter_parent_class parent_class

static GstStaticPadTemplate sink_factory =
    GST_STATIC_PAD_TEMPLATE("sink_%u", GST_PAD_SINK, GST_PAD_REQUEST,
                            GST_STATIC_CAPS("video/x-raw,format=RGB"));

static GstStaticPadTemplate src_factory =
    GST_STATIC_PAD_TEMPLATE("src_%u", GST_PAD_SRC, GST_PAD_SOMETIMES,
                            GST_STATIC_CAPS("video/x-raw,format=RGB"));

G_DEFINE_TYPE(GstAlmightyFilter, gst_almighty_filter, GST_TYPE_ELEMENT);
GST_ELEMENT_REGISTER_DEFINE(flow_c_extension, "flow_c_extension", GST_RANK_NONE,
                            GST_TYPE_ALMIGHTY_FILTER);

#define PROP_GPU_ID_DEFAULT 0
#define PROP_CONFIG_DEFAULT ""
#define PROP_THRESHOLD_DEFAULT 0.5

/*GObject vmethod implementations */
static void gst_almighty_filter_set_property(GObject* object, guint prop_id,
                                             const GValue* value,
                                             GParamSpec* pspec);
static void gst_almighty_filter_get_property(GObject* object, guint prop_id,
                                             GValue* value, GParamSpec* pspec);
static void gst_almighty_filter_finalize(GObject* object);

static void gst_almighty_filter_set_property(GObject* object, guint prop_id,
                                             const GValue* value,
                                             GParamSpec* pspec) {
  GstAlmightyFilter* self;
  GstAlmightyFilterData* data;

  self = GST_ALMIGHTY_FILTER(object);
  GstAlmightyFilterFramework* fw;
  fw = &self->fw;

  if (!gst_almighty_filter_common_set_property(fw, prop_id, value, pspec))
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
}

static void gst_almighty_filter_get_property(GObject* object, guint prop_id,
                                             GValue* value, GParamSpec* pspec) {
  printf("gst_almighty_filter_get_property\n");
}

static gboolean gst_almighty_filter_sink_event(GstPad* pad, GstObject* parent,
                                               GstEvent* event) {
  // g_print("sink_event() \n");
  GstAlmightyFilter* filter = GST_ALMIGHTY_FILTER(parent);

  GstAlmightyFilterPad* filterpad =
      (GstAlmightyFilterPad*)gst_pad_get_element_private(pad);

  if (GST_EVENT_TYPE(event) == GST_EVENT_CAPS) {
    GstCaps* caps;
    gst_event_parse_caps(event, &caps);

    gst_video_info_from_caps(&filterpad->video_info, caps);

    gst_pad_set_caps(filterpad->srcpad, caps);
    return TRUE;
  }

  return gst_pad_event_default(pad, parent, event);
}

static GstFlowReturn gst_almighty_filter_chain(GstPad* pad, GstObject* parent,
                                               GstBuffer* buf) {
  GstAlmightyFilter* filter = GST_ALMIGHTY_FILTER(parent);
  GstAlmightyFilterPad* filterpad =
      (GstAlmightyFilterPad*)gst_pad_get_element_private(pad);
  GstAlmightyFilterData* data = &filter->fw.data;
  GstAlmightyFilterApi* api = &filter->fw.api;

  GstXtensor* meta = GST_XTENSOR_GET(buf);
  if(filter->deal_meta)
  {
    gint ret = api->metapost(data->subplugin_handle, buf, meta);
    return gst_pad_push(filterpad->srcpad, buf);
  }else{
    TensorMemory in_tensors[1];
    TensorMemory out_tensors[1];

    guint i, j, k, batch, channel, height, width, size, maxsize;
    VARIABLE_TYPE vtype;
    // if (!filter->initialized)
    {
        // input shape
        vtype = meta->tensor_info.type;
        width = meta->tensor_info.W;
        height = meta->tensor_info.H;
        channel = meta->tensor_info.C;
        batch = meta->tensor_info.N;
        size = meta->tensor_info.size;
        maxsize = meta->tensor_info.maxsize;

    }
    // set-in-tensor
    TensorInfo in_info;
    memset(&in_info, 0, sizeof(TensorInfo));
    in_info.N = batch;
    in_info.C = channel;
    in_info.H = height;
    in_info.W = width;
    in_info.size = size;
    in_info.maxsize = maxsize;
    in_info.tensor_nums = 1;
    api->set_in_tensor(data->subplugin_handle, &in_info);
    // set-option
    api->set_option(data->subplugin_handle, 0);
    // get-out-tensor
    TensorInfo out_info;
    memset(&out_info, 0, sizeof(TensorInfo));
    api->get_out_tensor(data->subplugin_handle, &out_info);

    // post process
    GstMapInfo info, dest_info;
    gst_buffer_map(buf, &info, GST_MAP_READ);

    //output shape
    meta->tensor_info.N = out_info.N;
    meta->tensor_info.C = out_info.C;
    meta->tensor_info.H = out_info.H;
    meta->tensor_info.W = out_info.W;
    meta->tensor_info.size = out_info.size;
    meta->tensor_info.maxsize = out_info.maxsize;

    gsize frame_size = maxsize * tensor_element_size[vtype];
    GstBuffer* inbuf = gst_buffer_new_and_alloc(frame_size);
    gst_buffer_memset(inbuf, 0, 0, frame_size);
    gst_buffer_map(inbuf, &dest_info, GST_MAP_WRITE);

    in_tensors[0].data = (gfloat*)info.data;
    in_tensors[0].size = size;

    out_tensors[0].data = (gfloat*)dest_info.data;
    out_tensors[0].size = size;

    gint ret = api->invoke(data->subplugin_handle, in_tensors, out_tensors);

    gst_buffer_unmap(buf, &info);
    gst_buffer_unmap(inbuf, &dest_info);

    /** copy timestamps */
    gst_buffer_copy_into(inbuf, buf, GST_BUFFER_COPY_METADATA, 0, -1);

    if (inbuf != buf) {
        gst_buffer_unref(buf);
    }

    return gst_pad_push(filterpad->srcpad, inbuf);

  }

}

static void gst_almighty_filter_finalize(GObject* object) {
  printf("gst_almighty_filter_finalize\n");
  GstAlmightyFilter* filter = GST_ALMIGHTY_FILTER(object);
  GstAlmightyFilterFramework* fw = &filter->fw;
  gst_almighty_filter_common_close_fw(fw);
  gst_almighty_filter_common_free_property(fw);

  G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void gst_almighty_filter_init_after_props(GstAlmightyFilter* filter) {
  // g_mutex_init(&filter->mutex);
  // cuda_set_device(filter->gpu_id);
  // filter->Model = load_model(filter->config);
}

static GstPad* gst_almighty_filter_request_new_pad(GstElement* element,
                                                   GstPadTemplate* templ,
                                                   const gchar* name,
                                                   const GstCaps* caps) {
  GstAlmightyFilter* filter = GST_ALMIGHTY_FILTER(element);

  // if (filter->model == NULL)
  { gst_almighty_filter_init_after_props(filter); }

  GstPad* pad = gst_pad_new_from_template(templ, name);
  GstAlmightyFilterPad* filterpad = g_new(GstAlmightyFilterPad, 1);
  filterpad->id = filter->pad_count++;
  gst_pad_set_element_private(pad, filterpad);
  gst_pad_set_event_function(pad, gst_almighty_filter_sink_event);
  gst_pad_set_chain_function(pad, gst_almighty_filter_chain);
  gst_element_add_pad(element, pad);

  gchar* srcname = g_strdup_printf(src_factory.name_template, filterpad->id);
  filterpad->srcpad = gst_pad_new_from_static_template(&src_factory, srcname);
  g_free(srcname);
  gst_element_add_pad(element, filterpad->srcpad);

  return pad;
}

static void gst_almighty_filter_class_init(GstAlmightyFilterClass* klass) {
  // g_print("class_init() \n");
  GObjectClass* object_class = G_OBJECT_CLASS(klass);
  GstElementClass* element_class = (GstElementClass*)klass;

  object_class->set_property = gst_almighty_filter_set_property;
  object_class->get_property = gst_almighty_filter_get_property;
  object_class->finalize = gst_almighty_filter_finalize;

  gst_almighty_filter_install_properties(object_class);

  gst_element_class_set_details_simple(element_class, "flow_c_extension",
                                       "adaflow", "c extension function",
                                       "AUTHOR_NAME AUTHOR_EMAIL");

  gst_element_class_add_pad_template(element_class,
                                     gst_static_pad_template_get(&src_factory));

  gst_element_class_add_pad_template(
      element_class, gst_static_pad_template_get(&sink_factory));

  element_class->request_new_pad = gst_almighty_filter_request_new_pad;
}

static void gst_almighty_filter_init(GstAlmightyFilter* filter) {
  // g_print("[xnn infer] init() \n");
  filter->gpu_id = PROP_GPU_ID_DEFAULT;
  filter->config = g_strdup(PROP_CONFIG_DEFAULT);
  filter->threshold = PROP_THRESHOLD_DEFAULT;
  filter->pad_count = 0;
  filter->initialized = FALSE;
  filter->deal_meta = TRUE;

  printf("gst_almighty_filter_init\n");
  GstAlmightyFilterFramework* fw = &filter->fw;
  gst_almighty_filter_common_init_property(fw);
}
