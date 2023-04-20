#ifndef ADAFLOW_META_TENSOR_H
#define ADAFLOW_META_TENSOR_H

#include <gst/gst.h>
#include <gst/gstmeta.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

G_BEGIN_DECLS

GType gst_xtensor_api_get_type (void);
const GstMetaInfo *gst_xtensor_get_info (void);
void gst_xtensor_alloc (GstMeta * meta);

#define GST_XTENSOR_GET(buf) \
  ((GstXtensor*)gst_buffer_get_meta(buf, gst_xtensor_api_get_type()))
#define GST_XTENSOR_ADD(buf)                                     \
  ((GstXtensor*)gst_buffer_add_meta(buf, gst_xtensor_get_info(), \
                                    (gpointer)NULL))

#define GST_XTENSOR_MALLOC(meta) ((void)gst_xtensor_alloc(meta))

#define FLOW_TENSOR_DIM_LIMIT (4)
#define FLOW_TENSOR_BATCH_LIMIT_MAX (16)
#define FLOW_TENSOR_BATCH_LIMIT_MIN (8)
// #define MAX_WIDTH 1920
// #define MAX_HEIGHT 1080

#define FLOW_MODEL_DIMS (10)

typedef uint32_t tensor_dim[FLOW_TENSOR_DIM_LIMIT];

typedef enum device_type
{
  CPU = 0,
  CUDA,
  OPENCL,

  DEVICE_END,
} DEVICE_TYPE;

/**
 * @brief Internal data structure for format type.
 */
typedef enum format_type
{
  FLOW_RGB = 0,
  FLOW_I420,
  FLOW_NV12,
  FLOW_NV21,
  FLOW_GRAY8,

  FLOW_END,
} FORMAT_TYPE;

typedef enum tensor_type
{
  DATA_INT32 = 0,
  DATA_UINT32,
  DATA_INT16,
  DATA_UINT16,
  DATA_INT8,
  DATA_UINT8,
  DATA_FLOAT64,
  DATA_FLOAT32,
  DATA_INT64,
  DATA_UINT64,
  DATA_FLOAT16,

  DATA_END,
} VARIABLE_TYPE;

static const guint tensor_element_size[] = {
  [DATA_INT32] = 4,[DATA_UINT32] = 4,[DATA_INT16] = 2,[DATA_UINT16] = 2,
  [DATA_INT8] = 1,[DATA_UINT8] = 1,[DATA_FLOAT64] = 8,[DATA_FLOAT32] = 4,
  [DATA_INT64] = 8,[DATA_UINT64] = 8,[DATA_FLOAT16] = 2,[DATA_END] = 0,
};

// gsize gst_tensor_get_element_size(VARIABLE_TYPE type)
// {
//     g_return_val_if_fail(type >= 0 && type <= DATA_END, 0);
//     return tensor_element_size[type];
// }
typedef enum _TensorLayout
{
  FLOW_NCHW = 0,
  FLOW_NHWC,

  FLOW_LAYOUT_END
} TensorLayout;

typedef struct _TensorWindow
{
  int window[FLOW_TENSOR_BATCH_LIMIT_MAX];   /**< Select the input tensor(s) to
                                             invoke the models */
  int out_window_i[FLOW_TENSOR_BATCH_LIMIT_MAX];   /**< Select the output tensor(s)
                                                   from the input tensor(s) */
  bool window_defined;   /**< True if input windownation is defined */
} TensorWindow;

typedef struct _TensorInfo TensorInfo;
typedef struct _TensorInfo
{
  int N;                        // number:batch size
  int C;                        // channel
  int H;                        // height
  int W;                        // width
  gint size;
  gint maxsize;
  void *data;
  DEVICE_TYPE device;
  VARIABLE_TYPE type;
  FORMAT_TYPE format;

  const char *name;
  int tensor_nums;
  int dim[4];
  TensorWindow window;
  TensorLayout layout;

  int rate_n;
  int rate_d;

} _TensorInfo;

typedef struct
{
  GstMeta meta;

  _TensorInfo tensor_info;

} GstXtensor;

G_END_DECLS
#endif //ADAFLOW_META_TENSOR_H
