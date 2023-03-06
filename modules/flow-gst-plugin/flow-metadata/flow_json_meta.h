//
// Created by JingYao on 2023/3/6.
//

#ifndef FLOW_JSON_META_H
#define FLOW_JSON_META_H

#include <gst/gst.h>

#define FLOW_JSON_META_API_NAME "GstFLOWJSONMetaAPI"
#define FLOW_JSON_META_IMPL_NAME "GstFLOWJSONMeta"

G_BEGIN_DECLS

typedef struct _GstFLOWJSONMeta GstFLOWJSONMeta;

struct _GstFLOWJSONMeta {
  GstMeta meta;   /**< parent GstMeta */
  gchar *message; /**< C-string message */
};

const GstMetaInfo *gst_flow_json_meta_get_info(void);
GType gst_flow_json_meta_api_get_type(void);

#define GST_FLOW_JSON_META_INFO (gst_flow_json_meta_get_info())
#define GST_FLOW_JSON_META_GET(buf) ((GstFLOWJSONMeta *)gst_buffer_get_meta(buf, gst_flow_json_meta_api_get_type()))

/**
 * @def GST_FLOW_JSON_META_ITERATE
 * @brief This macro iterates through _GstFLOWJSONMeta instances for passed buf, retrieving the next _GstFLOWJSONMeta. If
 * state points to NULL, the first _GstFLOWJSONMeta is returned
 * @param buf GstBuffer* of which metadata is iterated and retrieved
 * @param state gpointer* that updates with opaque pointer after macro call.
 * @return _GstFLOWJSONMeta* instance attached to buf
 */
#define GST_FLOW_JSON_META_ITERATE(buf, state)                                                                          \
    ((GstFLOWJSONMeta *)gst_buffer_iterate_meta_filtered(buf, state, gst_flow_json_meta_api_get_type()))

/**
 * @def GST_FLOW_JSON_META_ADD
 * @brief This macro attaches new _GstFLOWJSONMeta instance to passed buf
 * @param buf GstBuffer* to which metadata will be attached
 * @return _GstFLOWJSONMeta* of the newly added instance attached to buf
 */
#define GST_FLOW_JSON_META_ADD(buf) ((GstFLOWJSONMeta *)gst_buffer_add_meta(buf, gst_flow_json_meta_get_info(), NULL))

/**
 * @brief This function returns message field of _GstFLOWJSONMeta
 * @param meta _GstFLOWJSONMeta* to retrieve message of
 * @return C-style string with message
 */
gchar *get_json_message(GstFLOWJSONMeta *meta);

/**
 * @brief This function sets message field of _GstFLOWJSONMeta
 * @param meta _GstFLOWJSONMeta* to set message
 * @param message message
 * @return void
 */
void set_json_message(GstFLOWJSONMeta *meta, const gchar *message);

GstFLOWJSONMeta * gst_buffer_add_json_info_meta(GstBuffer *buffer, const gchar *message);
gchar* gst_buffer_get_json_info_meta(GstBuffer *buffer);

G_END_DECLS


#endif // FLOW_JSON_META_H
