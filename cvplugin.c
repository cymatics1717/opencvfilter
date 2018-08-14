#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>

#include "cvfilter.h"
#define PACKAGE "cymatics"
static gboolean init (GstPlugin * plugin)
{
    if (!cvfilter_plugin_init (plugin))
        return FALSE;

    return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR, cvfilter,
                   "computer vision related algorithm library",
                   init, "VERSION 0.1.1", "Proprietary", "cymatics", "http://www.cymatics.cc")
