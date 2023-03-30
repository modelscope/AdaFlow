/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

#include <string.h>
#include "flow_json_meta.h"

#define UNUSED(x) (void)(x)

/**
 * @brief Register metadata type and returns Gtype
 * https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/gstreamer-GstMeta.html#gst-meta-api-type-register
 */
GType gst_flow_json_meta_api_get_type(void) {
  static GType type;
  static const gchar *tags[] = {NULL};

  if (g_once_init_enter(&type)) {
    GType _type = gst_meta_api_type_register(FLOW_JSON_META_API_NAME, tags);
    g_once_init_leave(&type, _type);
  }
  return type;
}

/**
 *@brief Meta init function
 */
gboolean gst_flow_json_meta_init(GstMeta *meta, gpointer params, GstBuffer *buffer) {
  UNUSED(params);
  UNUSED(buffer);

  GstFLOWJSONMeta *json_meta = (GstFLOWJSONMeta *)meta;
  json_meta->message = 0;
  return TRUE;
}

/**
 * @brief Meta transform function
 * https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/gstreamer-GstMeta.html#GstMetaTransformFunction
 */
gboolean gst_flow_json_meta_transform(GstBuffer *dest_buf, GstMeta *src_meta, GstBuffer *src_buf, GQuark type,
                                     gpointer data) {
  UNUSED(src_buf);
  UNUSED(type);
  UNUSED(data);

  g_return_val_if_fail(gst_buffer_is_writable(dest_buf), FALSE);

  GstFLOWJSONMeta *dst = GST_FLOW_JSON_META_ADD(dest_buf);
  GstFLOWJSONMeta *src = (GstFLOWJSONMeta *)src_meta;

  if (dst->message)
    g_free(dst->message);
  dst->message = g_strdup(src->message);
  return TRUE;
}

/**
 * @brief Removes metadata (GstFLOWJSONMeta) from buffer
 */
void gst_flow_json_meta_free(GstMeta *meta, GstBuffer *buffer) {
  UNUSED(buffer);

  GstFLOWJSONMeta *json_meta = (GstFLOWJSONMeta *)meta;
  if (json_meta->message) {
    g_free(json_meta->message);
    json_meta->message = NULL;
  }
}

/**
 * @brief GstMetaInfo provides info for specific metadata implementation
 * https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/gstreamer-GstMeta.html#GstMetaInfo
 */
const GstMetaInfo *gst_flow_json_meta_get_info(void) {
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter(&meta_info)) {
    const GstMetaInfo *meta =
        gst_meta_register(gst_flow_json_meta_api_get_type(), FLOW_JSON_META_IMPL_NAME, sizeof(GstFLOWJSONMeta),
                          (GstMetaInitFunction)gst_flow_json_meta_init, (GstMetaFreeFunction)gst_flow_json_meta_free,
                          (GstMetaTransformFunction)gst_flow_json_meta_transform);
    g_once_init_leave(&meta_info, meta);
  }
  return meta_info;
}

/**
 * @brief This function returns message field of _GstFLOWJSONMeta
 */
gchar *get_json_message(GstFLOWJSONMeta *meta) {
  return meta->message;
}

/**
 * @brief This function sets message field of _GstFLOWJSONMeta
 */
void set_json_message(GstFLOWJSONMeta *meta, const gchar *message) {
  gst_flow_json_meta_free((GstMeta *)meta, NULL);
  meta->message = g_strdup(message);
}

/**
 * @brief This function add GstFLOWJSONMeta to pass buffer
 */
GstFLOWJSONMeta* gst_buffer_add_json_info_meta(GstBuffer *buffer, const gchar *message)
{
  GstFLOWJSONMeta *gst_json_info_meta = NULL;

  g_return_val_if_fail(GST_IS_BUFFER(buffer), NULL);
  g_return_val_if_fail(gst_buffer_is_writable(buffer), NULL);

  gst_json_info_meta = (GstFLOWJSONMeta *) gst_buffer_add_meta (buffer, gst_flow_json_meta_get_info(), NULL);

  gst_json_info_meta->message = g_strdup(message);

  return gst_json_info_meta;
}

/**
 * @brief This function get meta message from pass buffer
 */
gchar* gst_buffer_get_json_info_meta(GstBuffer *buffer)
{
  GstFLOWJSONMeta* meta = (GstFLOWJSONMeta*)gst_buffer_get_meta((buffer), gst_flow_json_meta_api_get_type());

  gchar* has_meta = "NULL";
  if (meta == NULL)
    return has_meta;
  else
    return meta->message;
}

/**
 * @brief This function remove GstFLOWJSONMeta from pass buffer
 */
gboolean gst_buffer_remove_json_info_meta(GstBuffer *buffer)
{
  g_return_val_if_fail(GST_IS_BUFFER(buffer), NULL);

  GstFLOWJSONMeta* meta = (GstFLOWJSONMeta*)gst_buffer_get_meta((buffer), gst_flow_json_meta_api_get_type());

  if (meta == NULL)
    return TRUE;

  if ( !gst_buffer_is_writable(buffer))
    return FALSE;

  // https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstBuffer.html#gst-buffer-remove-meta
  return gst_buffer_remove_meta(buffer, &meta->meta);
}
