#include "flow_frame_convert.h"
#include <stdlib.h>

#include "meta_tensor.h"

#define gst_frameconvert_parent_class parent_class

#define GST_CAP_DEFAULT GST_VIDEO_CAPS_MAKE("{ RGB, I420, NV12, NV21 }")

static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE(
        "sink_%u", GST_PAD_SINK, GST_PAD_REQUEST, GST_STATIC_CAPS(GST_CAP_DEFAULT));

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE(
        "src_%u", GST_PAD_SRC, GST_PAD_SOMETIMES, GST_STATIC_CAPS(GST_CAP_DEFAULT));

enum {
    PROP_0,
};

G_DEFINE_TYPE(GstFrameconvert, gst_frameconvert, GST_TYPE_ELEMENT);
GST_ELEMENT_REGISTER_DEFINE(flow_frame_convert, "flow_frame_convert", GST_RANK_NONE,
                            GST_TYPE_FRAMECONVERT);

static void gst_frameconvert_set_property(GObject* object, guint prop_id,
                                          const GValue* value,
                                          GParamSpec* pspec) {
    GstFrameconvert* filter = GST_FRAMECONVERT(object);

    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void gst_frameconvert_get_property(GObject* object, guint prop_id,
                                          GValue* value, GParamSpec* pspec) {
    GstFrameconvert* filter = GST_FRAMECONVERT(object);

    switch (prop_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static gboolean gst_frameconvert_sink_event(GstPad* pad, GstObject* parent,
                                            GstEvent* event) {
    GstFrameconvertPad* filterpad = gst_pad_get_element_private(pad);

    if (GST_EVENT_TYPE(event) == GST_EVENT_CAPS) {
        GstCaps* caps;
        gst_event_parse_caps(event, &caps);

        gst_video_info_from_caps(&filterpad->video_info, caps);

        gst_pad_set_caps(filterpad->srcpad, caps);

        // guint i, len;
        // len = GST_CAPS_LEN(caps);
        // for (i = 0; i < len; i++)
        // {
        //   GstStructure *structure = gst_caps_get_structure_unchecked(caps, i);
        //   gst_structure_set_value(structure, "width", 800);
        //   gst_structure_set_value(structure, "height", 600);

        //   guint width;
        //   gst_structure_get_int(structure, "width", &width);
        //   // gst_structure_set_int(structure, "width", 800);
        //   printf("-----caps width = %d----- \n", width);
        // }

        // guint i;
        // for (i = 0; i < gst_caps_get_size(caps); i++)
        // {
        //   GstStructure *structure = gst_caps_get_structure(caps, i);
        //   g_print("--------%s---------\n", gst_structure_get_name(structure));
        // }

        // guint width, height;
        // GstStructure *structure;
        // structure = gst_caps_get_structure(caps, 0);
        // gst_structure_get_int(structure, "width", &width);
        // gst_structure_get_int(structure, "height", &height);
        // printf("-----caps width, height = %d, %d----- \n", width, height);

        // gst_caps_set_value(caps, "width", 800);
        // gst_caps_set_value(caps, "height", 600);
        // gst_structure_get_int(structure, "width", &width);
        // gst_structure_get_int(structure, "height", &height);
        // printf("-----caps width, height = %d, %d----- \n", width, height);

        // gst_video_info_set_format(&filterpad->video_info, GST_FORMAT_DEFAULT,
        // 800, 600); caps = gst_video_info_to_caps(&filterpad->video_info);

        // gst_caps_set_simple(caps, "width", G_TYPE_INT, 800, NULL);
        // gst_caps_set_simple(caps, "height", G_TYPE_INT, 600, NULL);

        // // GstVideoInfo *new_video_info = gst_video_info_new_from_caps(caps);
        // //or
        // GstVideoInfo *new_video_info = gst_video_info_new();
        // gst_video_info_init(new_video_info);
        // gst_video_info_set_format(new_video_info, GST_FORMAT_DEFAULT, 800, 600);
        // GstCaps *new_caps = gst_video_info_to_caps(new_video_info);
        // //if (!gst_video_info_is_equal(new_caps, caps))
        // {
        //   gst_pad_set_caps(filterpad->srcpad, new_caps);
        //   gst_caps_unref(new_caps);
        // }

        return TRUE;
    }

    return gst_pad_event_default(pad, parent, event);
}

static GstFlowReturn gst_frameconvert_chain(GstPad* pad, GstObject* parent,
                                            GstBuffer* buf) {
    GstFrameconvert* filter = GST_FRAMECONVERT(parent);
    GstFrameconvertPad* filterpad = gst_pad_get_element_private(pad);
    guint i, j, k, batch, channel, height, width, size, maxsize;
    VARIABLE_TYPE vtype;

    // if (!filter->initialized)
    {
        GstXtensor* meta = GST_XTENSOR_GET(buf);
        vtype = DATA_UINT8;

        // input shape
        width = meta->tensor_info.W;
        height = meta->tensor_info.H;
        channel = meta->tensor_info.C;
        batch = meta->tensor_info.N;
        size = meta->tensor_info.size;
        maxsize = meta->tensor_info.maxsize;

        // switch(meta->tensor_info.format)
        // {
        //   case XST_RGB:
        //   {
        //     printf("test-XST_RGB\n");
        //     break;
        //   }

        //   case XST_I420:
        //   {
        //     printf("test-XST_I420I\n");
        //     break;
        //   }

        //   default:
        //      GST_ERROR_OBJECT (filter, "Cannot deal with other format\n");
        // }

        // // output shape
        // filterpad->out_info.device = CPU;
        // filterpad->out_info.type = DATA_FLOAT32;
        // filterpad->out_info.N = batch;
        // filterpad->out_info.C = channel;
        // filterpad->out_info.H = height;
        // filterpad->out_info.W = width;
        // filterpad->out_info.size = size;
        // filterpad->out_info.maxsize = maxsize;

        // printf("frameconvert in width = %d \n", meta->tensor_info.W);
        // printf("frameconvert in height = %d \n", meta->tensor_info.H);
        // printf("frameconvert in channel = %d \n", meta->tensor_info.C);
        // printf("frameconvert in size = %d \n", meta->tensor_info.size);

        if (width != GST_VIDEO_INFO_WIDTH(&filterpad->video_info) ||
            height != GST_VIDEO_INFO_HEIGHT(&filterpad->video_info) ||
            channel != GST_VIDEO_INFO_SIZE(&filterpad->video_info) /
                               (GST_VIDEO_INFO_WIDTH(&filterpad->video_info) *
                                GST_VIDEO_INFO_HEIGHT(&filterpad->video_info))) {
            GstVideoInfo tmp_info;
            if (channel == 1) {
                gst_video_info_set_format(&tmp_info, GST_VIDEO_FORMAT_GRAY8, width,
                                          height);
            } else {
                gst_video_info_set_format(&tmp_info,
                                          GST_VIDEO_INFO_FORMAT(&filterpad->video_info),
                                          width, height);
            }
            tmp_info.stride[0] = width * channel;
            tmp_info.chroma_site = filterpad->video_info.chroma_site;
            tmp_info.colorimetry = filterpad->video_info.colorimetry;
            tmp_info.par_n = filterpad->video_info.par_n;
            tmp_info.par_d = filterpad->video_info.par_d;
            tmp_info.fps_n = filterpad->video_info.fps_n;
            tmp_info.fps_d = filterpad->video_info.fps_d;
            tmp_info.flags = filterpad->video_info.flags;
            tmp_info.interlace_mode = filterpad->video_info.interlace_mode;
            GstCaps* caps = gst_video_info_to_caps(&tmp_info);
            gst_pad_set_caps(filterpad->srcpad, caps);

            // printf("frameconvert out width = %d \n", tmp_info.width);
            // printf("frameconvert out height = %d \n", tmp_info.height);
            // printf("frameconvert out channel = %d \n", tmp_info.size /
            // (tmp_info.width * tmp_info.height)); printf("frameconvert out size = %d
            // \n", tmp_info.size); printf("frameconvert out stride = %d \n",
            // tmp_info.stride[0]);

            // GstVideoFormat format;
            // //gint vwidth, vheight, rate_n, rate_d;
            // format = GST_VIDEO_INFO_FORMAT(&filterpad->video_info);
            // //vwidth = GST_VIDEO_INFO_WIDTH(&filterpad->video_info);
            // //vheight = GST_VIDEO_INFO_HEIGHT(&filterpad->video_info);
            // //rate_n = GST_VIDEO_INFO_FPS_N(&filterpad->video_info);
            // //rate_d = GST_VIDEO_INFO_FPS_D(&filterpad->video_info);
            // gst_video_info_set_format(&filterpad->video_info, format, width,
            // height); GstCaps *caps =
            // gst_video_info_to_caps(&filterpad->video_info);
            // gst_pad_set_caps(filterpad->srcpad, caps);
        }

        // GST_XTENSOR_MALLOC(meta);
        // filter->initialized = TRUE;
    }

    GstMapInfo src_info, dest_info;
    gsize type = tensor_element_size[vtype];
    gsize frame_size = type * maxsize;
    gst_buffer_map(buf, &src_info, GST_MAP_READ);

    GstBuffer* inbuf = gst_buffer_new_and_alloc(frame_size);
    gst_buffer_memset(inbuf, 0, 0, frame_size);
    gst_buffer_map(inbuf, &dest_info, GST_MAP_WRITE);

    gfloat* psrc_data = (gfloat*)src_info.data;
    for (i = 0; i < size; i++) {
        dest_info.data[i] = (guint8)(psrc_data[i]);
    }

    gst_buffer_unmap(buf, &src_info);
    gst_buffer_unmap(inbuf, &dest_info);

    /** copy timestamps */
    gst_buffer_copy_into(inbuf, buf, GST_BUFFER_COPY_METADATA, 0, -1);

    if (inbuf != buf) {
        gst_buffer_unref(buf);
    }

    return gst_pad_push(filterpad->srcpad, inbuf);

    // GstMapInfo info;
    // gst_buffer_map(buf, &info, GST_MAP_READ);
    // for (i = 0; i < meta->size; i++)
    // {
    //   //info.data[i] = (guint8)(meta->data[i] * info.data[i]);
    //   info.data[i] = (guint8)(info.data[i]);
    // }

    // gst_buffer_unmap(buf, &info);

    // // g_mutex_lock(&filter->mutex);
    // // //todo
    // // g_mutex_unlock(&filter->mutex);

    // return gst_pad_push(filterpad->srcpad, buf);
}

static void gst_frameconvert_init_after_props(GstFrameconvert* filter) {
    // g_mutex_init(&filter->mutex);
    // cuda_set_device(filter->gpu_id);
    // filter->model = load_model(filter->config);
}

static GstPad* gst_frameconvert_request_new_pad(GstElement* element,
                                                GstPadTemplate* templ,
                                                const gchar* name,
                                                const GstCaps* caps) {
    GstFrameconvert* filter = GST_FRAMECONVERT(element);

    // if (filter->model == NULL)
    { gst_frameconvert_init_after_props(filter); }

    GstPad* pad = gst_pad_new_from_template(templ, name);
    GstFrameconvertPad* filterpad = g_new(GstFrameconvertPad, 1);
    filterpad->id = filter->pad_count++;
    gst_pad_set_element_private(pad, filterpad);
    gst_pad_set_event_function(pad, gst_frameconvert_sink_event);
    gst_pad_set_chain_function(pad, gst_frameconvert_chain);
    gst_element_add_pad(element, pad);

    gchar* srcname = g_strdup_printf(src_factory.name_template, filterpad->id);
    filterpad->srcpad = gst_pad_new_from_static_template(&src_factory, srcname);
    g_free(srcname);
    gst_element_add_pad(element, filterpad->srcpad);

    return pad;
}

static void gst_frameconvert_class_init(GstFrameconvertClass* klass) {
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    GstElementClass* element_class = (GstElementClass*)klass;

    object_class->set_property = gst_frameconvert_set_property;

    gst_element_class_set_details_simple(element_class, "flow_frame_convert",
                                         "adaflow", "tensor to video frame",
                                         "AUTHOR_NAME AUTHOR_EMAIL");

    gst_element_class_add_pad_template(element_class,
                                       gst_static_pad_template_get(&src_factory));

    gst_element_class_add_pad_template(
            element_class, gst_static_pad_template_get(&sink_factory));

    element_class->request_new_pad = gst_frameconvert_request_new_pad;
}

static void gst_frameconvert_init(GstFrameconvert* filter) {
    // g_print("[frame convert] init()\n");
    filter->pad_count = 0;
    filter->initialized = FALSE;
}
