#include "flow_tensor_convert.h"

#include <stdio.h>

#include "meta_tensor.h"

#define gst_tensorconvert_parent_class parent_class

#define GST_CAP_DEFAULT GST_VIDEO_CAPS_MAKE("{ RGB, I420, NV12, NV21, GRAY8, BGR}")

static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE(
        "sink_%u", GST_PAD_SINK, GST_PAD_REQUEST, GST_STATIC_CAPS(GST_CAP_DEFAULT));

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE(
        "src_%u", GST_PAD_SRC, GST_PAD_SOMETIMES, GST_STATIC_CAPS(GST_CAP_DEFAULT));

enum {
    PROP_0,
    PROP_DEVICE,
    PROP_ISUINT8,
    PROP_TEXT,
};

G_DEFINE_TYPE(GstTensorconvert, gst_tensorconvert, GST_TYPE_ELEMENT);
GST_ELEMENT_REGISTER_DEFINE(flow_tensor_convert, "flow_tensor_convert", GST_RANK_NONE,
                            GST_TYPE_TENSORCONVERT);

#define PROP_DEVICE_DEFAULT 0
#define PROP_ISUINT8_DEFAULT 0
#define PROP_TEXT_DEFAULT ""

static void gst_tensorconvert_set_property(GObject* object, guint prop_id,
                                           const GValue* value,
                                           GParamSpec* pspec) {
    GstTensorconvert* filter = GST_TENSORCONVERT(object);

    switch (prop_id) {
        case PROP_DEVICE:
            filter->device = g_value_get_uint(value);
            break;
        case PROP_ISUINT8:
            filter->is_uint8 = g_value_get_uint(value);
            break;
        case PROP_TEXT:
            filter->text = g_value_dup_string(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void gst_tensorconvert_get_property(GObject* object, guint prop_id,
                                           GValue* value, GParamSpec* pspec) {
    GstTensorconvert* filter = GST_TENSORCONVERT(object);

    switch (prop_id) {
        case PROP_DEVICE:
            g_value_set_uint(value, filter->device);
            break;
        case PROP_ISUINT8:
            g_value_set_uint(value, filter->is_uint8);
            break;
        case PROP_TEXT:
            g_value_set_string(value, filter->text);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static gboolean gst_tensorconvert_sink_event(GstPad* pad, GstObject* parent,
                                             GstEvent* event) {
    GstTensorconvertPad* filterpad = gst_pad_get_element_private(pad);
    GstTensorconvert* filter = GST_TENSORCONVERT(parent);

    if (GST_EVENT_TYPE(event) == GST_EVENT_CAPS) {
        GstCaps* caps;
        gst_event_parse_caps(event, &caps);

        gst_video_info_from_caps(&filterpad->video_info, caps);

        gst_pad_set_caps(filterpad->srcpad, caps);

        return TRUE;
    }

    return gst_pad_event_default(pad, parent, event);
}

static GstFlowReturn gst_tensorconvert_chain(GstPad* pad, GstObject* parent,
                                             GstBuffer* buf) {
    GstTensorconvert* filter = GST_TENSORCONVERT(parent);
    GstTensorconvertPad* filterpad = gst_pad_get_element_private(pad);
    guint i, j, k, batch, channel, height, width, stride, size, maxsize;
    gint rate_n, rate_d;
    VARIABLE_TYPE vtype;
    FORMAT_TYPE format;

    // if (!filter->initialized)
    {
        buf = gst_buffer_make_writable(buf);
        GstXtensor* meta = GST_XTENSOR_ADD(buf);
        guint num_tensors = gst_buffer_n_memory(buf);

        width = filterpad->video_info.width;
        height = filterpad->video_info.height;
        channel = filterpad->video_info.size / (height * width);
        stride = filterpad->video_info.stride[0];
        batch = num_tensors;
        // size = batch * channel * height * width;
        // maxsize = size;

        if (filter->is_uint8) {
            vtype = DATA_UINT8;
        } else {
            vtype = DATA_FLOAT32;
        }

        rate_n = filterpad->video_info.fps_n;
        rate_d = filterpad->video_info.fps_d;

        // video format
        // printf("format:%s",GST_VIDEO_INFO_FORMAT(&filterpad->video_info));
        switch (GST_VIDEO_INFO_FORMAT(&filterpad->video_info)) {
            case GST_VIDEO_FORMAT_RGB: {
                format = FLOW_RGB;
                size = batch * channel * height * width;
                maxsize = size;
                break;
            }

            case GST_VIDEO_FORMAT_I420: {
                format = FLOW_I420;
                size = batch * 1.5 * height * width;
                maxsize = size;
                break;
            }

            case GST_VIDEO_FORMAT_NV12: {
                format = FLOW_NV12;
                size = batch * 1.5 * height * width;
                maxsize = size;
                break;
            }

            case GST_VIDEO_FORMAT_NV21: {
                format = FLOW_NV21;
                size = batch * 1.5 * height * width;
                maxsize = size;
                break;
            }

            case GST_VIDEO_FORMAT_GRAY8: {
                format = FLOW_GRAY8;
                size = batch * height * width;
                maxsize = size;
                break;
            }

            default:
                GST_ERROR_OBJECT(filter, "Cannot deal with other format\n");
        }

        // input/output tensor shape
        meta->tensor_info.device = CPU;
        meta->tensor_info.type = vtype;
        meta->tensor_info.format = format;
        meta->tensor_info.N = batch;
        meta->tensor_info.C = channel;
        meta->tensor_info.H = height;
        meta->tensor_info.W = width;
        meta->tensor_info.size = size;
        meta->tensor_info.maxsize = maxsize;
        meta->tensor_info.rate_n = rate_n;
        meta->tensor_info.rate_d = rate_d;

    }

    GstMapInfo src_info, dest_info;
    gst_buffer_map(buf, &src_info, GST_MAP_READ);

    if (filter->is_uint8) {
        gst_buffer_unmap(buf, &src_info);

        return gst_pad_push(filterpad->srcpad, buf);

    } else {
        gsize frame_size = maxsize * tensor_element_size[vtype];
        GstBuffer* inbuf = gst_buffer_new_and_alloc(frame_size);
        gst_buffer_memset(inbuf, 0, 0, frame_size);
        gst_buffer_map(inbuf, &dest_info, GST_MAP_WRITE);

        guint8* psrc = (guint8*)src_info.data;
        gfloat* pdst = (gfloat*)dest_info.data;

        // convert by size
        for (i = 0; i < size; i++) {
            pdst[i] = (gfloat)(psrc[i]);
        }

        gst_buffer_unmap(buf, &src_info);
        gst_buffer_unmap(inbuf, &dest_info);

        /** copy timestamps */
        gst_buffer_copy_into(inbuf, buf, GST_BUFFER_COPY_METADATA, 0, -1);

        if (inbuf != buf) {
            gst_buffer_unref(buf);
        }

        return gst_pad_push(filterpad->srcpad, inbuf);
    }

}

static GstPad* gst_tensorconvert_request_new_pad(GstElement* element,
                                                 GstPadTemplate* templ,
                                                 const gchar* name,
                                                 const GstCaps* caps) {
    GstTensorconvert* filter = GST_TENSORCONVERT(element);

    GstPad* pad = gst_pad_new_from_template(templ, name);
    GstTensorconvertPad* filterpad = g_new(GstTensorconvertPad, 1);
    filterpad->id = filter->pad_count++;
    // filterpad->image_scaled = make_image(filter->net->w, filter->net->h, 3);
    gst_pad_set_element_private(pad, filterpad);
    gst_pad_set_event_function(pad, gst_tensorconvert_sink_event);
    gst_pad_set_chain_function(pad, gst_tensorconvert_chain);
    gst_element_add_pad(element, pad);

    gchar* srcname = g_strdup_printf(src_factory.name_template, filterpad->id);
    filterpad->srcpad = gst_pad_new_from_static_template(&src_factory, srcname);
    g_free(srcname);
    gst_element_add_pad(element, filterpad->srcpad);

    return pad;
}

static void gst_tensorconvert_class_init(GstTensorconvertClass* klass) {
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    GstElementClass* element_class = (GstElementClass*)klass;

    object_class->set_property = gst_tensorconvert_set_property;
    object_class->get_property = gst_tensorconvert_get_property;

    g_object_class_install_property(
            object_class, PROP_DEVICE,
            g_param_spec_uint("device", "device", "device of memory for alloc", 0,
                              100, PROP_DEVICE_DEFAULT,
                              G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property(
            object_class, PROP_ISUINT8,
            g_param_spec_uint("is-uint8", "is-uint8", "is to output tensor uint8", 0,
                              100, PROP_ISUINT8_DEFAULT,
                              G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property(
            object_class, PROP_TEXT,
            g_param_spec_string("text", "Text", "text to convert tensor", PROP_ISUINT8_DEFAULT,
                              G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

    gst_element_class_set_details_simple(element_class, "flow_tensor_convert",
                                         "adaflow", "video frame/text to tensor",
                                         "AUTHOR_NAME AUTHOR_EMAIL");

    gst_element_class_add_pad_template(element_class,
                                       gst_static_pad_template_get(&src_factory));

    gst_element_class_add_pad_template(
            element_class, gst_static_pad_template_get(&sink_factory));

    element_class->request_new_pad = gst_tensorconvert_request_new_pad;
}

static void gst_tensorconvert_init(GstTensorconvert* filter)
{
    filter->device = PROP_DEVICE_DEFAULT;
    filter->is_uint8 = PROP_ISUINT8_DEFAULT;
    filter->text = PROP_TEXT_DEFAULT;
    filter->pad_count = 0;
    filter->initialized = FALSE;
}
