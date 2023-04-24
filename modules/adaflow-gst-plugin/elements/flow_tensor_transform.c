#include "flow_tensor_transform.h"
#include <math.h>
#include <stdlib.h>

#include "meta_tensor.h"

#define GST_CAP_DEFAULT GST_VIDEO_CAPS_MAKE("{ RGB, I420, NV12, NV21 }")
#define gst_tensortransform_parent_class parent_class

/**
 * @brief The capabilities of the inputs
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE(
        "sink_%u", GST_PAD_SINK, GST_PAD_REQUEST, GST_STATIC_CAPS(GST_CAP_DEFAULT));
/**
 * @brief The capabilities of the outputs
 */
static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE(
        "src_%u", GST_PAD_SRC, GST_PAD_SOMETIMES, GST_STATIC_CAPS(GST_CAP_DEFAULT));
/**
 * @brief tensortransform properties
 */
enum {
    PROP_0,
    PROP_WAY,
    PROP_OPTION,
    PROP_LUM_ONLY,
};

static const gchar* gst_tensortransform_operator_string[] = {
        [GX_OP_ADD] = "add",
        [GX_OP_MUL] = "mul",
        [GX_OP_DIV] = "div",
        [GX_OP_UNKNOWN] = NULL};

static const gchar* gst_tensortransform_way_string[] = {
        [GX_ARITHMETIC] = "arithmetic",
        [GX_CLAMP] = "clamp",
        [GX_STAND] = "stand",
        [GX_DIMCHG] = "dimchg",
        [GX_UNKNOWN] = NULL};

static const gchar* gst_tensortransform_stand_way_string[] = {
        [GX_LINEAR] = "linear", [GX_ZSCORE] = "zscore", [GX_STAND_UNKNOWN] = NULL};

static const gchar* gst_tensortransform_dimchg_way_string[] = {
        [GX_NCHW] = "nchw", [GX_NHWC] = "nhwc", [GX_DIMCHG_UNKNOWN] = NULL};

G_DEFINE_TYPE(GstTensortransform, gst_tensortransform, GST_TYPE_ELEMENT);
GST_ELEMENT_REGISTER_DEFINE(flow_tensor_transform, "flow_tensor_transform", GST_RANK_NONE,
                            GST_TYPE_TENSORTRANSFORM);

#define PROP_WAY_DEFAULT ""
#define PROP_OPTION_DEFAULT ""
#define DEFAULT_LUM_ONLY TRUE
#define XCLAMP(value, minValue, maxValue) \
  ((value) < (minValue) ? (minValue)      \
                        : ((value) > (maxValue) ? (maxValue) : (value)))
#define XMAX2(a, b) ((a) > (b) ? (a) : (b))
#define XMIN2(a, b) ((a) < (b) ? (a) : (b))

/////////////////function-claration///////////////////////////////////////////////////////
static transform_operator gst_tensortransform_get_operator(const gchar* str);
static transform_way gst_tensortransform_get_way(const gchar* str);
static gboolean gst_tensortransform_set_option(GstTensortransform* filter);
static transform_stand_way gst_tensortransform_get_stand_way(const gchar* str);
static transform_dimchg_way gst_tensortransform_get_dimchg_way(
        const gchar* str);

//////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Set property (gst element vmethod)
 */
static void gst_tensortransform_set_property(GObject* object, guint prop_id,
                                             const GValue* value,
                                             GParamSpec* pspec) {
    GstTensortransform* filter = GST_TENSORTRANSFORM(object);

    switch (prop_id) {
        case PROP_WAY:
            filter->way = g_value_dup_string(value);
            break;

        case PROP_OPTION:
            filter->option = g_value_dup_string(value);
            gst_tensortransform_set_option(filter);
            break;

        case PROP_LUM_ONLY:
            filter->lum_only = g_value_get_boolean(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

/**
 * @brief Get property (gst element vmethod)
 */
static void gst_tensortransform_get_property(GObject* object, guint prop_id,
                                             GValue* value, GParamSpec* pspec) {
    GstTensortransform* filter = GST_TENSORTRANSFORM(object);

    switch (prop_id) {
        case PROP_WAY:
            g_value_set_string(value, filter->way);
            break;
        case PROP_OPTION:
            g_value_set_string(value, filter->option);
            break;

        case PROP_LUM_ONLY:
            g_value_set_boolean(value, filter->lum_only);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

/**
 * @brief Finalize transform(gst element vmethod)
 */
static void gst_tensortransform_finalize(GObject* object) {
    GstTensortransform* filter;

    filter = GST_TENSORTRANSFORM(object);

    if (filter->option) {
        g_free(filter->option);
        filter->option = NULL;
    }

    if (filter->operators) {
        g_slist_free_full(filter->operators, g_free);
        filter->operators = NULL;
    }

    G_OBJECT_CLASS(parent_class)->finalize(object);
}

/**
 * @brief event function
 */
static gboolean gst_tensortransform_sink_event(GstPad* pad, GstObject* parent,
                                               GstEvent* event) {
    GstTensortransform* filter = GST_TENSORTRANSFORM(parent);

    GstTensortransformPad* filterpad = gst_pad_get_element_private(pad);

    if (GST_EVENT_TYPE(event) == GST_EVENT_CAPS) {
        GstCaps* caps;
        gst_event_parse_caps(event, &caps);
        gst_video_info_from_caps(&filterpad->video_info, caps);
        gst_pad_set_caps(filterpad->srcpad, caps);
        return TRUE;
    }

    return gst_pad_event_default(pad, parent, event);
}

/**
 * @brief main function. required vmethod for GstElement class.
 * @param[in/out] trans "super" pointer
 * @param[in] inbuf The input gst buffer
 * @param[out] outbuf The output gst buffer
 * @return Gst Flow Status
 */
static GstFlowReturn gst_tensortransform_chain(GstPad* pad, GstObject* parent,
                                               GstBuffer* buf) {
    GstTensortransform* filter = GST_TENSORTRANSFORM(parent);
    GstTensortransformPad* filterpad = gst_pad_get_element_private(pad);
    guint i, j, k, batch, channel, height, width, size, maxsize;

    GstXtensor* meta = GST_XTENSOR_GET(buf);
    // if (!filter->initialized)
    {
        // input shape
        maxsize = meta->tensor_info.maxsize;

        // GST_XTENSOR_MALLOC(meta);
        // filter->initialized = TRUE;
    }

    GstMapInfo info;
    gst_buffer_map(buf, &info, GST_MAP_READ);

    gfloat* p_input_rgbrgb = (gfloat*)info.data;

    // main-function-here
    switch (filter->tf_way) {
        case GX_ARITHMETIC: {
            gst_tensortransform_arithmetic(filter, p_input_rgbrgb, meta);
            break;
        }
        case GX_CLAMP: {
            gst_tensortransform_clamp(filter, p_input_rgbrgb, meta);
            break;
        }

        case GX_STAND: {
            gst_tensortransform_stand(filter, p_input_rgbrgb, meta);
            break;
        }

        case GX_DIMCHG: {
            gsize frame_size = maxsize * sizeof(float);
            gfloat* p_output_rgbrgb = (gfloat*)malloc(frame_size);

            gst_tensortransform_dimchg(filter, p_input_rgbrgb, p_output_rgbrgb, meta);

            gst_buffer_unref(p_output_rgbrgb);

            break;
        }

        default:
            GST_ERROR_OBJECT(filter, "Cannot identify way\n");
    }

    gst_buffer_unmap(buf, &info);

    return gst_pad_push(filterpad->srcpad, buf);
}

/**
 * @brief update new pad
 */
static GstPad* gst_tensortransform_request_new_pad(GstElement* element,
                                                   GstPadTemplate* templ,
                                                   const gchar* name,
                                                   const GstCaps* caps) {
    GstTensortransform* filter = GST_TENSORTRANSFORM(element);

    GstPad* pad = gst_pad_new_from_template(templ, name);
    GstTensortransformPad* filterpad = g_new(GstTensortransformPad, 1);
    filterpad->id = filter->pad_count++;
    gst_pad_set_element_private(pad, filterpad);
    gst_pad_set_event_function(pad, gst_tensortransform_sink_event);
    gst_pad_set_chain_function(pad, gst_tensortransform_chain);
    gst_element_add_pad(element, pad);

    gchar* srcname = g_strdup_printf(src_factory.name_template, filterpad->id);
    filterpad->srcpad = gst_pad_new_from_static_template(&src_factory, srcname);
    g_free(srcname);
    gst_element_add_pad(element, filterpad->srcpad);

    return pad;
}

/**
 * @brief initialize the tensor_transform's class
 */
static void gst_tensortransform_class_init(GstTensortransformClass* klass) {
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    GstElementClass* element_class = (GstElementClass*)klass;

    object_class->set_property = gst_tensortransform_set_property;
    object_class->get_property = gst_tensortransform_get_property;
    object_class->finalize = gst_tensortransform_finalize;

    g_object_class_install_property(
            object_class, PROP_WAY,
            g_param_spec_string("way", "Way", "Way used for transforming tensor ?",
                                PROP_WAY_DEFAULT, G_PARAM_WRITABLE));

    g_object_class_install_property(
            object_class, PROP_OPTION,
            g_param_spec_string("option", "Option",
                                "Option for the tensor transform way ?",
                                PROP_OPTION_DEFAULT, G_PARAM_WRITABLE));

    g_object_class_install_property(
            object_class, PROP_LUM_ONLY,
            g_param_spec_boolean("lum-only", "Lum Only",
                                 "Only apply filter on luminance", DEFAULT_LUM_ONLY,
                                 G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

    gst_element_class_set_details_simple(
            element_class, "flow_tensor_transform", "FIXME:Generic",
            "FIXME:Generic Template Element", "AUTHOR_NAME AUTHOR_EMAIL");

    gst_element_class_add_pad_template(element_class,
                                       gst_static_pad_template_get(&src_factory));

    gst_element_class_add_pad_template(
            element_class, gst_static_pad_template_get(&sink_factory));

    element_class->request_new_pad = gst_tensortransform_request_new_pad;
}

/**
 * @brief initialize the tensor_transform
 */
static void gst_tensortransform_init(GstTensortransform* filter) {
    filter->pad_count = 0;
    filter->initialized = FALSE;
    filter->way = g_strdup(PROP_WAY_DEFAULT);
    filter->option = g_strdup(PROP_OPTION_DEFAULT);
    filter->operators = NULL;
    filter->pad_count = 0;
    filter->tf_way = GX_UNKNOWN;
    filter->lum_only = DEFAULT_LUM_ONLY;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Setup internal data (data_* in GstTensorTransform)
 * @param[in/out] filter "this" pointer. mode & option MUST BE set already.
 * @retval TRUE if OK or operation-skipped, FALSE if fatal-error.
 */
static gboolean gst_tensortransform_set_option(GstTensortransform* filter) {
    gchar* filter_name;
    gboolean ret = FALSE;

    // special consideration
    if (filter->way == GX_UNKNOWN || filter->way == NULL) return TRUE;

    filter_name = gst_object_get_name((GstObject*)filter);

    filter->tf_way = gst_tensortransform_get_way(filter->way);

    switch (filter->tf_way) {
        case GX_ARITHMETIC: {
            gchar* str_option;
            gchar** str_operators;
            gchar** str_op;
            transform_operator_s* op_s;
            guint i, num_operators, num_op;

            str_option = g_strdup(filter->option);
            str_operators = g_strsplit(str_option, ",", -1);
            num_operators = g_strv_length(str_operators);

            // assign one by one
            for (i = 0; i < num_operators; ++i) {
                str_op = g_strsplit(str_operators[i], ":", -1);
                op_s = g_new0(transform_operator_s, 1);
                g_assert(op_s);
                op_s->op = gst_tensortransform_get_operator(str_op[0]);
                op_s->value = g_ascii_strtod(str_op[1], NULL);
                // filter->data_arithmetic.value = g_ascii_strtod(str_op[1], NULL);

                // append operator
                if (op_s->op != GX_OP_UNKNOWN) {
                    filter->operators = g_slist_append(filter->operators, op_s);
                    // filter->data_arithmetic.values =
                    // g_slist_append(filter->data_arithmetic.values,
                    // g_ascii_strtod(str_op[1], NULL));
                } else {
                    g_free(op_s);
                }
                g_strfreev(str_op);
            }
            g_strfreev(str_operators);
            g_free(str_option);
            ret = TRUE;
            break;
        }

        case GX_CLAMP: {
            gchar* str_option;
            gchar** str_options;
            str_option = g_strdup(filter->option);
            str_options = g_strsplit(str_option, ":", 2);

            filter->data_clamp.min = g_ascii_strtod(str_options[0], NULL);
            filter->data_clamp.max = g_ascii_strtod(str_options[1], NULL);

            if (filter->data_clamp.min > filter->data_clamp.max) {
                g_error("clamp: CLAMP_MIN can not larger than CLAMP_MAX \n");
            }

            g_free(str_option);
            g_free(str_options);
            ret = TRUE;
            break;
        }

        case GX_STAND: {
            gchar* str_option;
            gchar** str_options;
            guint num_operators;
            str_option = g_strdup(filter->option);
            str_options = g_strsplit(str_option, ":", -1);
            num_operators = g_strv_length(str_options);

            filter->data_stand.stand_way =
                    gst_tensortransform_get_stand_way(str_options[0]);

            if (num_operators > 1 &&
                g_ascii_strcasecmp(str_options[1], "false") == 0) {
                filter->data_stand.per_channel = FALSE;
            } else {
                filter->data_stand.per_channel = TRUE;
            }

            g_free(str_option);
            g_free(str_options);
            ret = TRUE;
            break;
        }

        case GX_DIMCHG: {
            gchar* str_option;
            str_option = g_strdup(filter->option);
            filter->data_dimchg.dimchg_way =
                    gst_tensortransform_get_dimchg_way(str_option);
            g_free(str_option);

            ret = TRUE;
            break;
        }

        default:
            GST_ERROR_OBJECT(filter, "Cannot identify way\n");
            ret = FALSE;
    }
    g_free(filter_name);
    return ret;
}

/**
 * @brief Get the corresponding operator from the string value
 * @param[in] str The string value for the operator
 * @return corresponding operator for the string (GX_OP_UNKNOWN for errors)
 */
static transform_operator gst_tensortransform_get_operator(const gchar* str) {
    int index;

    index = find_key_strv(gst_tensortransform_operator_string, str);

    return (index < 0) ? GX_OP_UNKNOWN : index;
}

/**
 * @brief Get the corresponding transform from the string value
 * @param[in] str The string value for the operator
 * @return corresponding operator for the string (GX_UNKNOWN for errors)
 */
static transform_way gst_tensortransform_get_way(const gchar* str) {
    int index;

    index = find_key_strv(gst_tensortransform_way_string, str);

    return (index < 0) ? GX_UNKNOWN : index;
}

/**
 * @brief Get the corresponding stand mode from the string value
 * @param[in] str The string value for the operator
 * @return corresponding operator for the string (GX_STAND_UNKNOWN for errors)
 */
static transform_stand_way gst_tensortransform_get_stand_way(const gchar* str) {
    int index;

    index = find_key_strv(gst_tensortransform_stand_way_string, str);

    return (index < 0) ? GX_STAND_UNKNOWN : index;
}

/**
 * @brief Get the corresponding dimchg mode from the string value
 * @param[in] str The string value for the operator
 * @return corresponding operator for the string (GX_DIMCHG_UNKNOWN for errors)
 */
static transform_dimchg_way gst_tensortransform_get_dimchg_way(
        const gchar* str) {
    int index;

    index = find_key_strv(gst_tensortransform_dimchg_way_string, str);

    return (index < 0) ? GX_DIMCHG_UNKNOWN : index;
}

/**
 * @brief subrouting for tensor-tranform, "arithmetic" case.
 * @param[in/out] filter "this" pointer
 * @param[in] inptr input tensor
 * @param[out] outptr output tensor
 * @return Gst void
 */
void gst_tensortransform_arithmetic(GstTensortransform* filter, gfloat* pdata,
                                    GstXtensor* meta) {
    guint i, j, k;
    // Processed in user-specified order
    GSList* order;
    transform_operator_s* op_s;
    order = filter->operators;

    int cal_size;
    if (filter->lum_only && (meta->tensor_info.format == FLOW_I420 ||
                             meta->tensor_info.format == FLOW_NV12 ||
                             meta->tensor_info.format == FLOW_NV21)) {
        cal_size = meta->tensor_info.H * meta->tensor_info.W * meta->tensor_info.N;
    } else {
        cal_size = meta->tensor_info.size;
    }

    while (order) {
        op_s = (transform_operator_s*)order->data;
        switch (op_s->op) {
            case GX_OP_ADD: {
                float add_value = op_s->value;
                for (i = 0; i < cal_size; i++) {
                    pdata[i] = pdata[i] + add_value;
                }
                break;
            }
            case GX_OP_MUL: {
                float mul_value = op_s->value;
                for (i = 0; i < cal_size; i++) {
                    pdata[i] = pdata[i] * mul_value;
                }
                break;
            }
            case GX_OP_DIV: {
                float div_value = op_s->value;

                if (div_value == 0.0) {
                    g_error("Divisor cannot be 0, will not operate \n");
                }

                for (i = 0; i < cal_size; i++) {
                    pdata[i] = pdata[i] / div_value;
                }
                break;
            }
            default:
                GST_ERROR_OBJECT(filter, "Cannot identify operator\n");
        }
        order = g_slist_next(order);
    }
}

/**
 * @brief subrouting for tensor-tranform, "clamp" case.
 * @param[in/out] filter "this" pointer
 * @param[in] inptr input tensor
 * @param[out] outptr output tensor
 * @return Gst void
 */
void gst_tensortransform_clamp(GstTensortransform* filter, gfloat* pdata,
                               GstXtensor* meta) {
    guint i, j, k;
    float min_value = filter->data_clamp.min;
    float max_value = filter->data_clamp.max;

    int cal_size;
    if (filter->lum_only && (meta->tensor_info.format == FLOW_I420 ||
                             meta->tensor_info.format == FLOW_NV12 ||
                             meta->tensor_info.format == FLOW_NV21)) {
        cal_size = meta->tensor_info.H * meta->tensor_info.W * meta->tensor_info.N;
    } else {
        cal_size = meta->tensor_info.size;
    }

    for (i = 0; i < cal_size; i++) {
        pdata[i] = XCLAMP(pdata[i], min_value, max_value);
    }
}

/**
 * @brief subrouting for tensor-tranform, "stand" case.
 * @param[in/out] filter "this" pointer
 * @param[in] inptr input tensor
 * @param[out] outptr output tensor
 * @return Gst void
 */
void gst_tensortransform_stand(GstTensortransform* filter, gfloat* pdata,
                               GstXtensor* meta) {
    guint i, j, k;
    // per_channel or not
    if (filter->data_stand.per_channel) {
        switch (filter->data_stand.stand_way) {
            case GX_LINEAR: {
                // 1.per-channel-max-min
                float max_c0 = 0.0, max_c1 = 0.0, max_c2 = 0.0;
                float min_c0 = 0.0, min_c1 = 0.0, min_c2 = 0.0;
                float data_c0, data_c1, data_c2;
                for (i = 0; i < meta->tensor_info.size; i++) {
                    data_c0 = pdata[i];
                    max_c0 = XMAX2(data_c0, max_c0);
                    min_c0 = XMIN2(data_c0, min_c0);
                }

                // 2.pix=(pix-min)/(max-min)
                float div_max_min_c0 = max_c0 - min_c0;
                float div_max_min_c1 = max_c1 - min_c1;
                float div_max_min_c2 = max_c2 - min_c2;

                if (div_max_min_c0 == 0.0 || div_max_min_c1 == 0.0 ||
                    div_max_min_c2 == 0.0) {
                    g_warning("Divisor cannot be 0, will not operate \n");
                    break;
                } else {
                    for (i = 0; i < meta->tensor_info.size; i++) {
                        pdata[i] = (pdata[i] - min_c0) / div_max_min_c0;
                    }
                }
                break;
            }
            case GX_ZSCORE: {
                // 1.cal-mean-std
                float mean_c0[4] = {0, 0, 0, 0};
                float std_c0[4] = {0, 0, 0, 0};
                float sum_c0[4] = {0, 0, 0, 0};
                float std_sum_c0[4] = {0, 0, 0, 0};
                float sum_all_c = meta->tensor_info.H * meta->tensor_info.W;
                for (i = 0; i < meta->tensor_info.H * meta->tensor_info.W; i++) {
                    guint idx = i * meta->tensor_info.C;
                    for (k = 0; k < meta->tensor_info.C; k++) {
                        sum_c0[k] += pdata[idx + k];
                    }
                }

                for (k = 0; k < meta->tensor_info.C; k++) {
                    mean_c0[k] = sum_c0[k] / sum_all_c;
                }

                for (i = 0; i < meta->tensor_info.H * meta->tensor_info.W; i++) {
                    guint idx = i * meta->tensor_info.C;
                    for (k = 0; k < meta->tensor_info.C; k++) {
                        std_sum_c0[k] += pow(pdata[idx + k] - mean_c0[k], 2);
                    }
                }

                for (k = 0; k < meta->tensor_info.C; k++) {
                    std_sum_c0[k] = (std_sum_c0[k] != 0.0)
                                            ? sqrt(std_sum_c0[k] / sum_all_c)
                                            : (1e-10);
                }

                // 2.pix=(pix-mean)/std
                for (i = 0; i < meta->tensor_info.H * meta->tensor_info.W; i++) {
                    guint idx = i * meta->tensor_info.C;
                    for (k = 0; k < meta->tensor_info.C; k++) {
                        pdata[idx + k] = (pdata[idx + k] - mean_c0[k]) / std_c0[k];
                    }
                }

                break;
            }
            default:
                GST_ERROR_OBJECT(filter, "Cannot identify stand way\n");
        }
    } else {
        switch (filter->data_stand.stand_way) {
            case GX_LINEAR: {
                // 1.per-channel-max-min
                float max_c = 0.0;
                float min_c = 0.0;
                float data_c0 = 0;
                float data_tmp = 0;
                for (i = 0; i < meta->tensor_info.H * meta->tensor_info.W; i++) {
                    guint idx = i * meta->tensor_info.C;
                    for (k = 0; k < meta->tensor_info.C; k++) {
                        data_c0 = pdata[idx + k];
                        data_tmp = XMAX2(data_tmp, data_c0);
                        max_c = XMAX2(data_tmp, max_c);
                        data_tmp = XMIN2(data_tmp, data_c0);
                        min_c = XMIN2(data_tmp, min_c);
                    }
                }

                // 2.pix=(pix-min)/(max-min)
                float div_max_min_c = max_c - min_c;

                if (div_max_min_c == 0.0) {
                    g_warning("Divisor cannot be 0, will not operate \n");
                    break;
                } else {
                    for (i = 0; i < meta->tensor_info.size; i++) {
                        pdata[i] = (pdata[i] - min_c) / div_max_min_c;
                    }
                }

                break;
            }
            case GX_ZSCORE: {
                // 1.cal-mean-std
                float mean_c;
                float sum_c = 0.0;
                float std_sum_c = 0.0;
                float sum_all_c = meta->tensor_info.size;
                for (i = 0; i < meta->tensor_info.size; i++) {
                    sum_c += pdata[i];
                }
                mean_c = sum_c / sum_all_c;

                for (i = 0; i < meta->tensor_info.size; i++) {
                    std_sum_c += pow(pdata[i] - mean_c, 2);
                }

                std_sum_c = (std_sum_c != 0.0) ? sqrt(std_sum_c / sum_all_c) : (1e-10);

                // 2.pix=(pix-mean)/std
                for (i = 0; i < meta->tensor_info.size; i++) {
                    pdata[i] = (pdata[i] - mean_c) / std_sum_c;
                }

                break;
            }
            default:
                GST_ERROR_OBJECT(filter, "Cannot identify stand way\n");
        }
    }
}

/**
 * @brief subrouting for tensor-tranform, "dimchg" case.
 * @param[in/out] filter "this" pointer
 * @param[in] inptr input tensor
 * @param[out] outptr output tensor
 * @return Gst void
 */
void gst_tensortransform_dimchg(GstTensortransform* filter, gfloat* pdata,
                                gfloat* pdataout, GstXtensor* meta) {
    guint i, j, k;

    guint width = meta->tensor_info.W;
    guint height = meta->tensor_info.H;

    switch (filter->data_dimchg.dimchg_way) {
        case GX_NCHW: {
            int sparital_num = width * height;
            for (i = 0; i < height; i++) {
                for (j = 0; j < width; j++) {
                    guint idx_in = i * width * 3 + j * 3;
                    guint idx_out = i * width + j;

                    pdataout[idx_out] = pdata[idx_in + 0];
                    pdataout[idx_out + sparital_num] = pdata[idx_in + 1];
                    pdataout[idx_out + 2 * sparital_num] = pdata[idx_in + 2];
                }
            }
            memcpy(pdata, pdataout, meta->tensor_info.size * sizeof(float));
            break;
        }

        case GX_NHWC: {
            int sparital_num = width * height;
            for (i = 0; i < height; i++) {
                for (j = 0; j < width; j++) {
                    guint idx_in = i * width + j;
                    guint idx_out = i * width * 3 + j * 3;

                    pdataout[idx_out + 0] = pdata[idx_in];
                    pdataout[idx_out + 1] = pdata[idx_in + sparital_num];
                    pdataout[idx_out + 2] = pdata[idx_in + 2 * sparital_num];
                }
            }
            memcpy(pdata, pdataout, meta->tensor_info.size * sizeof(float));
            break;
        }

        default:
            GST_ERROR_OBJECT(filter, "Cannot identify dimchg way\n");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
