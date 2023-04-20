#ifndef ADAFLOW_FLOW_TENSOR_TRANSFORM_H
#define ADAFLOW_FLOW_TENSOR_TRANSFORM_H

#include <gst/gst.h>
#include <gst/video/video.h>

#include "common.h"
#include "meta_tensor.h"

G_BEGIN_DECLS

GType gst_tensortransform_get_type();

#define GST_TYPE_TENSORTRANSFORM (gst_tensortransform_get_type())

#define GST_TENSORTRANSFORM(obj)                               \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TENSORTRANSFORM, \
                              GstTensortransform))
#define GST_TENSORTRANSFORM_CLASS(klass)                      \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TENSORTRANSFORM, \
                           GstTensortransformClass))

#define GST_IS_TENSORTRANSFORM(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TENSORTRANSFORM))
#define GST_IS_TENSORTRANSFORM_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TENSORTRANSFORM))

typedef struct _GstTensortransform GstTensortransform;
typedef struct _GstTensortransformClass GstTensortransformClass;

typedef struct {
    guint id;
    GstPad* srcpad;
    GstVideoInfo video_info;
} GstTensortransformPad;

typedef enum {
    GX_ARITHMETIC = 0, /**< Arithmetic. "arithmetic" */
    GX_CLAMP = 1,      /**< Clamp. "clamp" */
    GX_STAND = 2,      /**< Standardization. "stand" */
    GX_DIMCHG = 3,     /**< Dimension Change. "dimchg" */
    GX_UNKNOWN         /**< Unknown/Not-implemented-yet Mode. "unknown" */

} transform_way;

typedef enum {
    GX_OP_ADD = 0,
    GX_OP_MUL = 1,
    GX_OP_DIV = 2,

    GX_OP_UNKNOWN
} transform_operator;

typedef enum {
    GX_LINEAR = 0,
    GX_ZSCORE = 1,

    GX_STAND_UNKNOWN
} transform_stand_way;

typedef struct {
    transform_operator op;
    float value;
} transform_operator_s;

/**
 * @brief Internal data structure for arithmetic way.
 */
typedef struct _transform_arithmetic {
    GSList* values;
} transform_arithmetic;

/**
 * @brief Internal data structure for clamp way.
 */
typedef struct _transform_clamp {
    float min, max;
} transform_clamp;

/**
 * @brief Internal data structure for stand way.
 */
typedef struct _transform_stand {
    gboolean per_channel;
    transform_stand_way stand_way;
} transform_stand;

typedef enum {
    GX_NCHW = 0,
    GX_NHWC = 1,
    GX_DIMCHG_UNKNOWN
} transform_dimchg_way;

/**
 * @brief Internal data structure for dimchg way.
 */
typedef struct _transform_dimchg {
    transform_dimchg_way dimchg_way;
} transform_dimchg;

/**
 * @brief Internal data structure for tensor_transform instances.
 */
struct _GstTensortransform {
    GstElement element; /**< This is the parent object */

    /* properties */
    guint pad_count;
    gboolean initialized;

    // new properties
    gchar* way;        /**< The way to choose */
    gchar* option;     /**< Stored option value  */
    GSList* operators; /**< Arithmetic operators list */
    gboolean lum_only; /**< only deal with lum */

    transform_way tf_way;

    union {
        transform_arithmetic
                data_arithmetic; /**< Parsed option value for "arithmetic" mode. */
        transform_clamp data_clamp;   /**< Parsed option value for "clamp" mode. */
        transform_stand data_stand;   /**< Parsed option value for "stand" mode. */
        transform_dimchg data_dimchg; /**< Parsed option value for "dimchg" mode */
    };
};

/**
 * @brief GstTensorTransformClass inherits GstElement.
 */
struct _GstTensortransformClass {
    GstElementClass parent_class;
};

void gst_tensortransform_arithmetic(GstTensortransform* filter, gfloat* pdata,
                                    GstXtensor* meta);

void gst_tensortransform_clamp(GstTensortransform* filter, gfloat* pdata,
                               GstXtensor* meta);

void gst_tensortransform_stand(GstTensortransform* filter, gfloat* pdata,
                               GstXtensor* meta);

void gst_tensortransform_dimchg(GstTensortransform* filter, gfloat* pdata,
                                gfloat* pdataout, GstXtensor* meta);

/**
 * @brief Get Type function required for gst elements
 */
GST_ELEMENT_REGISTER_DECLARE(flow_tensor_transform);

G_END_DECLS
/* */

#endif//ADAFLOW_FLOW_TENSOR_TRANSFORM_H
