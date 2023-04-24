/*******************************************************************************
* Copyright (c) Alibaba, Inc. and its affiliates.
*
* Licensed under the Apache License, Version 2.0
******************************************************************************/

#include <glib.h>
#ifndef __FLOW_SUBPLUGIN_H__
#define __FLOW_SUBPLUGIN_H__

typedef enum {
    XS_SUBPLUGIN_FILTER,
    XS_SUBPLUGIN_CONVERTER,
    XS_SUBPLUGIN_END
} subplugin_type;

typedef struct _SubpluginData{
  char *name;
  const void *data;
} SubpluginData;


const void *get_subplugin(subplugin_type type, const char *subplugin_name);

gboolean register_subplugin(subplugin_type type, const char *subplugin_name, const void *subplugin);

gboolean unregister_subplugin(subplugin_type type, const char *subplugin_name);


#endif
