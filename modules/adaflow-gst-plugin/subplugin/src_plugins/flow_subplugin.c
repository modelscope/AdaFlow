/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/
#include "flow_subplugin.h"

#include <glib.h>
#include <gmodule.h>
#include <printf.h>
#include <stdlib.h>

static GHashTable* subplugins[XS_SUBPLUGIN_END] = {0};

G_LOCK_DEFINE_STATIC(splock);

static void _spdata_destroy(gpointer _data) {
  SubpluginData* data = (_data);
  g_free(data->name);
  g_free(data);
}

const void* get_subplugin(subplugin_type type, const char* subplugin_name) {
  SubpluginData* spdata = NULL;
  if (subplugin_name == NULL) {
    return NULL;
  }
  G_LOCK(splock);
  if (subplugins[type] == NULL) {
    subplugins[type] =
        g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _spdata_destroy);
  } else {
    spdata = g_hash_table_lookup(subplugins[type], subplugin_name);
  }
  G_UNLOCK(splock);

  return (spdata != NULL) ? spdata->data : NULL;
}

static inline SubpluginData* get_SubpluginData(subplugin_type type,
                                               const char* subplugin_name) {
  SubpluginData* spdata = NULL;
  if (subplugin_name == NULL) {
    return spdata;
  }
  G_LOCK(splock);
  if (subplugins[type] == NULL) {
    subplugins[type] =
        g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _spdata_destroy);
  } else {
    spdata = g_hash_table_lookup(subplugins[type], subplugin_name);
  }
  G_UNLOCK(splock);

  return spdata;
}

gboolean register_subplugin(subplugin_type type, const char* subplugin_name,
                            const void* subplugin) {
  SubpluginData* spdata = NULL;
  gboolean ret;

  if (subplugin_name == NULL) {
    return FALSE;
  }
  if (subplugin == NULL) {
    return FALSE;
  }

  spdata = get_SubpluginData(type, subplugin_name);
  if (spdata) {
    printf("subplugin %s is already registered.", subplugin_name);
  }

  spdata = malloc(sizeof(SubpluginData));
  if (spdata == NULL) {
    printf("Failed to allocate mem for SubpluginData.");
    return FALSE;
  }
  spdata->name = g_strdup(subplugin_name);
  spdata->data = subplugin;
  G_LOCK(splock);
  ret = g_hash_table_insert(subplugins[type], g_strdup(subplugin_name), spdata);
  G_UNLOCK(splock);

  return ret;
}

gboolean unregister_subplugin(subplugin_type type, const char* subplugin_name) {
  gboolean ret;

  if (subplugin_name == NULL) {
    return FALSE;
  }
  if (subplugins[type] == NULL) {
    return FALSE;
  }

  G_LOCK(splock);
  ret = g_hash_table_remove(subplugins[type], subplugin_name);
  G_UNLOCK(splock);

  return ret;
}
