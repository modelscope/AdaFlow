/**
* @file	 adaflow.c
* @brief Registers all adaflow C-plugins for gstreamer so that we can have a single big binary
*/

#include <gst/gst.h>
#include <elements/flow_video_aggregate.h>

/**
 * @brief Function to initialize all adaflow elements
 */
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
