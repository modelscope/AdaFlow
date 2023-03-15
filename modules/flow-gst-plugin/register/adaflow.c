//
// Created by JingYao on 2023/3/14.
//

#include <gst/gst.h>
#include <elements/flow_video_aggregate.h>


static gboolean plugin_init(GstPlugin* plugin) {
  gboolean ret = FALSE;
  ret |= GST_ELEMENT_REGISTER(flowaggregator, plugin);
  return ret;
}

#ifndef PACKAGE
#define PACKAGE "adaflow"
#endif

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  flow_gst_plugin,
                  "adaflow plugins",
                  plugin_init,
                  // VERSION
                  "1.0.1",
                  // LICENSE
                  // FIXME(long.qul) which license?
                  "LGPL",
                  "adaflow",
                  "https://github.com/alibaba/adaflow");
