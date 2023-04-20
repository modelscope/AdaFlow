#include "meta_tensor.h"
GType gst_xtensor_api_get_type(void) {
    static GType type;

    if (g_once_init_enter(&type)) {
        const gchar* tags[] = {NULL};
        GType _type = gst_meta_api_type_register("GstXtensorAPI", tags);
        g_once_init_leave(&type, _type);
    }

    return type;
}

static gboolean gst_xtensor_init(GstMeta* meta, gpointer params,
                                 GstBuffer* buffer) {
    GstXtensor* cur_meta = (GstXtensor*)meta;

    // input tensor
    cur_meta->tensor_info.N = 0;
    cur_meta->tensor_info.C = 0;
    cur_meta->tensor_info.H = 0;
    cur_meta->tensor_info.W = 0;
    cur_meta->tensor_info.size = 0;
    cur_meta->tensor_info.maxsize = 0;
    cur_meta->tensor_info.data = NULL;
    cur_meta->tensor_info.device = CPU;
    cur_meta->tensor_info.type = DATA_FLOAT32;
    cur_meta->tensor_info.format = FLOW_RGB;
    cur_meta->tensor_info.tensor_nums = 1;

    return TRUE;
}

static gboolean gst_xtensor_transform(GstBuffer* dest_buf, GstMeta* src_meta,
                                      GstBuffer* src_buf, GQuark type,
                                      gpointer data) {
    GstXtensor* src_data = (GstXtensor*)src_meta;
    GstXtensor* dst_data = GST_XTENSOR_ADD(dest_buf);

    dst_data->tensor_info = src_data->tensor_info;

    // guint i = 0;
    // for (i = 0; i < dst_data->intensor.size; i++)
    // {
    //     dst_data->tensor_info.data[i] = src_data->intensor.data[i];
    // }

    return TRUE;
}

void gst_xtensor_alloc(GstMeta* meta) {
    GstXtensor* cur_meta = (GstXtensor*)meta;
    if (!cur_meta->tensor_info.data) {
        if (cur_meta->tensor_info.device == CPU) {
            switch (cur_meta->tensor_info.type) {
                case DATA_INT32:
                    cur_meta->tensor_info.data =
                            (gint*)malloc(cur_meta->tensor_info.maxsize * sizeof(gint));
                    break;
                case DATA_UINT8:
                    cur_meta->tensor_info.data =
                            (guint8*)malloc(cur_meta->tensor_info.maxsize * sizeof(guint8));
                    break;
                case DATA_FLOAT32:
                    cur_meta->tensor_info.data =
                            (gfloat*)malloc(cur_meta->tensor_info.maxsize * sizeof(gfloat));
                    break;

                default:
                    cur_meta->tensor_info.data =
                            (gfloat*)malloc(cur_meta->tensor_info.maxsize * sizeof(gfloat));
                    break;
            }
        } else if (cur_meta->tensor_info.device == CUDA) {
            // TODO
        } else if (cur_meta->tensor_info.device == OPENCL) {
            // TODO
        } else {
            printf("unknown device type");
        }
    }
}

static void gst_xtensor_free(GstMeta* meta, GstBuffer* buffer) {
    GstXtensor* cur_meta = (GstXtensor*)meta;
    if (cur_meta->tensor_info.data) {
        cur_meta->tensor_info.N = 0;
        cur_meta->tensor_info.C = 0;
        cur_meta->tensor_info.H = 0;
        cur_meta->tensor_info.W = 0;
        cur_meta->tensor_info.size = 0;
        cur_meta->tensor_info.maxsize = 0;
        free(cur_meta->tensor_info.data);
        cur_meta->tensor_info.data = NULL;
    }
}

const GstMetaInfo* gst_xtensor_get_info(void) {
    static const GstMetaInfo* meta_info = NULL;

    if (g_once_init_enter(&meta_info)) {
        const GstMetaInfo* meta = gst_meta_register(
                gst_xtensor_api_get_type(), "GstXtensor", sizeof(GstXtensor),
                gst_xtensor_init, gst_xtensor_free, gst_xtensor_transform);
        g_once_init_leave(&meta_info, meta);
    }

    return meta_info;
}
