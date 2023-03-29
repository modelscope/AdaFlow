/**
 * @file flow_json_meta.h
 * @brief This file contains helper functions to control GstFLOWJSONMeta instances
 */

#ifndef FLOW_JSON_META_H
#define FLOW_JSON_META_H

#include <gst/gst.h>

#define FLOW_JSON_META_API_NAME "GstFLOWJSONMetaAPI"
#define FLOW_JSON_META_IMPL_NAME "GstFLOWJSONMeta"

G_BEGIN_DECLS

typedef struct _GstFLOWJSONMeta GstFLOWJSONMeta;

/**
 * @brief This struct represents JSON metadata and contains instance of parent GstMeta and message
 */
struct _GstFLOWJSONMeta {
  GstMeta meta;
  gchar *message;
};

const GstMetaInfo *gst_flow_json_meta_get_info(void);
GType gst_flow_json_meta_api_get_type(void);

#define GST_FLOW_JSON_META_INFO (gst_flow_json_meta_get_info())
#define GST_FLOW_JSON_META_GET(buf) ((GstFLOWJSONMeta *)gst_buffer_get_meta(buf, gst_flow_json_meta_api_get_type()))
#define GST_FLOW_JSON_META_ITERATE(buf, state)                                                                          \
    ((GstFLOWJSONMeta *)gst_buffer_iterate_meta_filtered(buf, state, gst_flow_json_meta_api_get_type()))

#define GST_FLOW_JSON_META_ADD(buf) ((GstFLOWJSONMeta *)gst_buffer_add_meta(buf, gst_flow_json_meta_get_info(), NULL))


gchar *get_json_message(GstFLOWJSONMeta *meta);

void set_json_message(GstFLOWJSONMeta *meta, const gchar *message);

GstFLOWJSONMeta * gst_buffer_add_json_info_meta(GstBuffer *buffer, const gchar *message);
gchar* gst_buffer_get_json_info_meta(GstBuffer *buffer);
gboolean gst_buffer_remove_json_info_meta(GstBuffer *buffer);

G_END_DECLS


#endif // FLOW_JSON_META_H
