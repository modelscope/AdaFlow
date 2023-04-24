/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

#include "flow_extension_util.h"

#include <dlfcn.h>
#include <glib-object.h>
#include <gst/gst.h>
#include <string.h>

#define g_free_const(x) g_free((void*)(long)(x))

void gst_almighty_filter_install_properties(GObjectClass* gobject_class) {
  g_object_class_install_property(
      gobject_class, PROP_FRAMEWORK,
      g_param_spec_string("framework", "Framework", "Neural network framework",
                          "auto", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(
      gobject_class, PROP_MODEL,
      g_param_spec_string("model", "Model filepath",
                          "File path to the model file. Separated with ',' in "
                          "case of multiple model files(like caffe2)",
                          "", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(
      gobject_class, PROP_DEVICE,
      g_param_spec_string("device", "Hardware device",
                          "Hardware device assigned to run model", "",
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(
      gobject_class, PROP_INPUT,
      g_param_spec_string(
          "input", "Input dimension",
          "Input tensor dimension from inner array, up to 4 dimensions ?", "",
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(
      gobject_class, PROP_INPUTNAME,
      g_param_spec_string("inputname", "Name of Input Tensor",
                          "The Name of Input Tensor", "",
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(
      gobject_class, PROP_INPUTTYPE,
      g_param_spec_string("inputtype", "Input tensor element type",
                          "Type of each element of the input tensor ?", "",
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(
      gobject_class, PROP_INPUTLAYOUT,
      g_param_spec_string(
          "inputlayout", "Input Data Layout",
          "Set channel first(NCHW) or channel last layout(NHWC) or None for "
          "input data. "
          "Layout of the data can be any or NHWC or NCHW or none for now. ",
          "", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(
      gobject_class, PROP_INPUTRANKS,
      g_param_spec_string("inputranks", "Rank of Input Tensor",
                          "The Rank of the Input Tensor, which is separated "
                          "with ',' in case of multiple Tensors",
                          "", G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(
      gobject_class, PROP_OUTPUTNAME,
      g_param_spec_string("outputname", "Name of Output Tensor",
                          "The Name of Output Tensor", "",
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(
      gobject_class, PROP_OUTPUT,
      g_param_spec_string(
          "output", "Output dimension",
          "Output tensor dimension from inner array, up to 4 dimensions ?", "",
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(
      gobject_class, PROP_OUTPUTTYPE,
      g_param_spec_string("outputtype", "Output tensor element type",
                          "Type of each element of the output tensor ?", "",
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(
      gobject_class, PROP_OUTPUTLAYOUT,
      g_param_spec_string(
          "outputlayout", "Output Data Layout",
          "Set channel first(NCHW) or channel last layout(NHWC) or None for "
          "output data. "
          "Layout of the data can be any or NHWC or NCHW or none for now. ",
          "", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(
      gobject_class, PROP_OUTPUTRANKS,
      g_param_spec_string("outputranks", "Rank of Out Tensor",
                          "The Rank of the Out Tensor, which is separated with "
                          "',' in case of multiple Tensors",
                          "", G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
}

void gst_almighty_filter_common_init_property(GstAlmightyFilterFramework* fw) {
  memset(fw, 0, sizeof(GstAlmightyFilterFramework));
}

void gst_almighty_filter_common_close_fw(GstAlmightyFilterFramework* fw) {
  if (fw->data.fw_opened) {
    if (fw->api.close) {
      fw->api.close(fw->data.subplugin_handle);
      if (fw->data.subpluginlib_handle) {
        dlclose(fw->data.subpluginlib_handle);
        fw->data.subpluginlib_handle = NULL;
      }
    }
    fw->data.fw_opened = FALSE;
    g_free_const(fw->data.fw_name);
    fw->data.fw_name = NULL;
  }
}

void gst_almighty_filter_common_free_property(GstAlmightyFilterFramework* fw) {
  GstAlmightyFilterData* data = &fw->data;

  g_free_const(data->fw_name);
  g_free_const(data->device);
  g_free_const(data->model);
}

int load_subplugin(GstAlmightyFilterFramework* fw, const char* lib_path) {
  GstAlmightyFilterData* data = &fw->data;
  GstAlmightyFilterApi* api = &fw->api;
  int status = -1;
  data->subpluginlib_handle = dlopen(lib_path, RTLD_NOW);
  if (!data->subpluginlib_handle) {
    return status;
  }

  api->open = (FUNC_OPEN)dlsym(data->subpluginlib_handle, "api_open");
  api->set_in_tensor =
      (FUNC_SETINTENSOR)dlsym(data->subpluginlib_handle, "api_set_in_tensor");
  api->get_out_tensor = (FUNC_GETOUTPUTTENSOR)dlsym(data->subpluginlib_handle,
                                                    "api_get_out_tensor_info");
  api->set_option =
      (FUNC_SETOPTION)dlsym(data->subpluginlib_handle, "api_set_option");
  api->invoke = (FUNC_INVOKE)dlsym(data->subpluginlib_handle, "api_invoke");
  api->metapost = (FUNC_INVOKE)dlsym(data->subpluginlib_handle, "api_metapost");
  api->close = (FUNC_CLOSE)dlsym(data->subpluginlib_handle, "api_close");
  if (!api->close) {
    return status;
  }
  status = 0;
  return status;
}

int set_FRAMEWORK(GstAlmightyFilterFramework* fw, const GValue* value) {
  gint status = 0;
  GstAlmightyFilterData* data = &fw->data;
  GstAlmightyFilterApi* api = &fw->api;
  const char* fw_name = g_value_get_string(value);
  if (api) {
    if (g_strcmp0(data->fw_name, fw_name) != 0) {
      gst_almighty_filter_common_close_fw(fw);
    }
  }
  status = load_subplugin(fw, fw_name);
  if (!status) {
    (api->open)(&data->subplugin_handle);
    data->fw_opened = TRUE;
    fw->data_configed = TRUE;
    fw->api_configed = TRUE;
  }
  return status;
}

int set_MODEL(GstAlmightyFilterFramework* fw, const GValue* value) {
  gint status = 0;
  GstAlmightyFilterData* data = &fw->data;
  const char* model = g_value_get_string(value);
  if (!model) {
    status = -1;
    return status;
  }
  data->model = model;
  // load model
  return status;
}

int set_DIMENSION(GstAlmightyFilterData* data, const GValue* value,
                  const gboolean is_input) {
  gint status = 0;
  guint num_dims;
  gchar** str_dims;
  str_dims = g_strsplit_set(g_value_get_string(value), ":", -1);
  num_dims = g_strv_length(str_dims);
  if (num_dims > FLOW_TENSOR_DIM_LIMIT) {
    status = -1;
    return status;
  }

  for (int i = 0; i < num_dims; i++) {
    guint64 val = g_ascii_strtoull(str_dims[i], NULL, 10);
    if (is_input) {
      data->input_tensor_meta.dim[i] = val;
    } else {
      data->output_tensor_meta.dim[i] = val;
    }
  }
  return status;
}

int set_DATATYPE(GstAlmightyFilterData* data, const GValue* value,
                 const gboolean is_input) {
  gint status = 0;
  gchar* str_type = g_value_get_string(value);
  guint64 val = g_ascii_strtoull(str_type, NULL, 10);

  if (val > DATA_END) {
    status = -1;
    return status;
  }
  if (is_input) {
    data->input_tensor_meta.type = val;
  } else {
    data->output_tensor_meta.type = val;
  }
  return status;
}

int set_LAYOUT(GstAlmightyFilterData* data, const GValue* value,
               const gboolean is_input) {
  gint status = 0;
  gchar* str_layout = g_value_get_string(value);
  guint64 val = g_ascii_strtoull(str_layout, NULL, 10);

  if (val > FLOW_LAYOUT_END) {
    status = -1;
    return status;
  }
  if (is_input) {
    data->input_tensor_meta.layout = val;
  } else {
    data->output_tensor_meta.layout = val;
  }
  return status;
}

int set_DEVICE(GstAlmightyFilterData* data, const GValue* value) {
  gint status = 0;
  const char* device = g_value_get_string(value);
  if (!device) {
    status = -1;
    return status;
  }
  data->device = device;
  return status;
}

int set_NAME(GstAlmightyFilterData* data, const GValue* value,
             const gboolean is_input) {
  gint status = 0;
  const char* name = g_value_get_string(value);
  if (!name) {
    status = -1;
    return status;
  }
  if (is_input) {
    data->input_tensor_meta.name = name;
  } else {
    data->input_tensor_meta.name = name;
  }

  return status;
}

int set_WINDOW(GstAlmightyFilterData* data, const GValue* value,
               const gboolean is_input) {
  gint status = 0;
  guint window_size;
  gchar** str_window;
  str_window = g_strsplit_set(g_value_get_string(value), ":", -1);
  window_size = g_strv_length(str_window);
  if (window_size > FLOW_TENSOR_BATCH_LIMIT_MAX) {
    status = -1;
    return status;
  }

  for (int i = 0; i < window_size; i++) {
    guint64 val = g_ascii_strtoull(str_window[i], NULL, 10);
    if (is_input) {
      data->input_tensor_meta.window.window[i] = val;
    } else {
      data->input_tensor_meta.window.window[i] = val;
    }
  }
  return status;
}

gboolean gst_almighty_filter_common_set_property(GstAlmightyFilterFramework* fw,
                                                 guint prop_id,
                                                 const GValue* value,
                                                 GParamSpec* pspec) {
  gint status = 0;
  GstAlmightyFilterData* data = &(fw->data);

  switch (prop_id) {
    case PROP_FRAMEWORK:
      status = set_FRAMEWORK(fw, value);
      break;
    case PROP_MODEL:
      status = set_MODEL(fw, value);
      break;
    case PROP_INPUT:
      status = set_DIMENSION(data, value, TRUE);
      break;
    case PROP_OUTPUT:
      status = set_DIMENSION(data, value, FALSE);
      break;
    case PROP_INPUTTYPE:
      status = set_DATATYPE(data, value, TRUE);
      break;
    case PROP_OUTPUTTYPE:
      status = set_DATATYPE(data, value, FALSE);
      break;
    case PROP_INPUTNAME:
      status = set_NAME(data, value, TRUE);
      break;
    case PROP_OUTPUTNAME:
      status = set_NAME(data, value, FALSE);
      break;
    case PROP_ACCELERATOR:
      status = set_DEVICE(data, value);
      break;
    case PROP_INPUTLAYOUT:
      status = set_LAYOUT(data, value, TRUE);
      break;
    case PROP_OUTPUTLAYOUT:
      status = set_LAYOUT(data, value, FALSE);
      break;
    case PROP_INPUTWINDOW:
      status = set_WINDOW(data, value, TRUE);
      break;
    case PROP_OUTPUTWINDOW:
      status = set_WINDOW(data, value, FALSE);
      break;
    default:
      return FALSE;
  }
  if (0 != status) return FALSE;

  return TRUE;
}

guint gst_almighty_parse_dimension(const gchar* dimstr, uint32_t* dim) {
  guint rank = 0;
  guint64 val;
  gchar** strv;
  gchar* dim_string;
  guint i, num_dims;

  if (dimstr == NULL) return 0;

  /* remove spaces */
  dim_string = g_strdup(dimstr);
  g_strstrip(dim_string);

  strv = g_strsplit(dim_string, ":", FLOW_TENSOR_DIM_LIMIT);
  num_dims = g_strv_length(strv);

  for (i = 0; i < num_dims; i++) {
    g_strstrip(strv[i]);
    if (strv[i] == NULL || strlen(strv[i]) == 0) break;

    val = g_ascii_strtoull(strv[i], NULL, 10);
    dim[i] = (uint32_t)val;
    rank = i + 1;
  }

  for (; i < FLOW_TENSOR_DIM_LIMIT; i++) dim[i] = 1;

  g_strfreev(strv);
  g_free(dim_string);
  return rank;
}

gchar* gst_almighty_get_dimension_string(uint32_t* dim) {
  guint i;
  GString* dim_str;
  dim_str = g_string_new(NULL);

  for (i = 0; i < FLOW_TENSOR_DIM_LIMIT; i++) {
    g_string_append_printf(dim_str, "%d", dim[i]);

    if (i < FLOW_TENSOR_DIM_LIMIT - 1) {
      g_string_append(dim_str, ":");
    }
  }
  return g_string_free(dim_str, FALSE);
}

uint gst_almighty_get_tensor_size(GstAlmightyFilterFramework* fw, int tensor_id,
                                  gboolean is_input) {
  int n = 0;
  int c = 0;
  int h = 0;
  int w = 0;
  int ele_size = 0;
  if (is_input) {
    n = fw->data.input_tensor_meta.dim[0];
    c = fw->data.input_tensor_meta.dim[1];
    h = fw->data.input_tensor_meta.dim[2];
    w = fw->data.input_tensor_meta.dim[3];
    VARIABLE_TYPE ele_type = fw->data.input_tensor_meta.type;
    ele_size = tensor_element_size[ele_type];
  } else {
    n = fw->data.output_tensor_meta.dim[0];
    c = fw->data.output_tensor_meta.dim[1];
    h = fw->data.output_tensor_meta.dim[2];
    w = fw->data.output_tensor_meta.dim[3];
    VARIABLE_TYPE ele_type = fw->data.output_tensor_meta.type;
    ele_size = tensor_element_size[ele_type];
  }
  return n * c * h * w * ele_size;
}

// gboolean gst_almighty_config_from_structure(TensorsConfig *config, const
// GstStructure *structure) {
//     const gchar *name;
//     tensor_format format = _NNS_TENSOR_FORMAT_STATIC;

//     if(!config || !structure) {
//         return FALSE;
//     }

//     gst_almighty_config_init(config);

//     name = gst_structure_get_name(structure);
//     config->info.num_tensors = 1;

//     if(gst_structure_has_field(structure, "dimension")) {
//         const gchar *dim_str = gst_structure_get_string(structure,
//         "dimension"); gst_almighty_parse_dimension(dim_str,
//         config->info.info[0].dimension);
//     }

//     if(gst_structure_has_field(structure, "type")) {
//         const gchar *type_str = gst_structure_get_string(structure, "type");
//         config->info.info[0].type = gst_almighty_get_type(type_str);
//     }

//     if(gst_structure_has_field(structure, "framerate")) {
//         gst_structure_get_fraction(structure, "framerate", &config->rate_n,
//         &config->rate_d);
//     }

//     return TRUE;
// }

void gst_almighty_config_init(TensorConfig* config) {
  if (!config) {
    return;
  }

  config->rate_n = -1;
  config->rate_d = -1;
  gst_almighty_info_init(&config->info);
}

void gst_almighty_info_init(TensorInfo* info) {
  if (!info) {
    return;
  }
  memset(info, 0, sizeof(TensorInfo));
  info->type = DATA_END;
}