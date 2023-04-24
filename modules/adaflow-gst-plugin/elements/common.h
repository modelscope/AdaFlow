
#ifndef ADAFLOW_COMMON_H
#define ADAFLOW_COMMON_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

gint find_key_strv(const gchar** strv, const gchar* key);
// global function
void I420uv_resize(const gfloat* input, gint src_h, gint src_w, gfloat* output,
                   gint dst_h, gint dst_w);
void NVuv_resize(const gfloat* input, gint src_h, gint src_w, gfloat* output,
                 gint dst_h, gint dst_w);

#ifdef __cplusplus
}
#endif

#endif//ADAFLOW_COMMON_H
