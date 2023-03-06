//
// Created by JingYao on 2023/3/6.
//

#include <string.h>

#include "flow_json_meta.h"

#define UNUSED(x) (void)(x)

GType gst_flow_json_meta_api_get_type(void) {
  static GType type;
  static const gchar *tags[] = {NULL};

  if (g_once_init_enter(&type)) {
    GType _type = gst_meta_api_type_register(FLOW_JSON_META_API_NAME, tags);
    g_once_init_leave(&type, _type);
  }
  return type;
}

gboolean gst_flow_json_meta_init(GstMeta *meta, gpointer params, GstBuffer *buffer) {
  UNUSED(params);
  UNUSED(buffer);

  GstFLOWJSONMeta *json_meta = (GstFLOWJSONMeta *)meta;
  json_meta->message = 0;
  return TRUE;
}

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

void gst_flow_json_meta_free(GstMeta *meta, GstBuffer *buffer) {
  UNUSED(buffer);

  GstFLOWJSONMeta *json_meta = (GstFLOWJSONMeta *)meta;
  if (json_meta->message) {
    g_free(json_meta->message);
    json_meta->message = NULL;
  }
}

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

gchar *get_json_message(GstFLOWJSONMeta *meta) {
  return meta->message;
}

void set_json_message(GstFLOWJSONMeta *meta, const gchar *message) {
  gst_flow_json_meta_free((GstMeta *)meta, NULL);
  meta->message = g_strdup(message);
}

GstFLOWJSONMeta* gst_buffer_add_json_info_meta(GstBuffer *buffer, const gchar *message)
{
  GstFLOWJSONMeta *gst_json_info_meta = NULL;

  g_return_val_if_fail(GST_IS_BUFFER(buffer), NULL);
  g_return_val_if_fail(gst_buffer_is_writable(buffer), NULL);

  gst_json_info_meta = (GstFLOWJSONMeta *) gst_buffer_add_meta (buffer, gst_flow_json_meta_get_info(), NULL);

  gst_json_info_meta->message = g_strdup(message);

  return gst_json_info_meta;
}

gchar* gst_buffer_get_json_info_meta(GstBuffer *buffer)
{
  GstFLOWJSONMeta* meta = (GstFLOWJSONMeta*)gst_buffer_get_meta((buffer), gst_flow_json_meta_api_get_type());
  return meta->message;
}
