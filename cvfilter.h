#ifndef _CVFILTER_H_
#define _CVFILTER_H_

#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS
#define C_TYPE_OPENCV_FILTER   (cvfilter_get_type())
#define cvfilter(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),C_TYPE_OPENCV_FILTER,CVFilter))
#define cvfilter_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),C_TYPE_OPENCV_FILTER,CVFilterClass))
#define C_IS_OPENCV_FILTER(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),C_TYPE_OPENCV_FILTER))
#define C_IS_OPENCV_FILTER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass),C_TYPE_OPENCV_FILTER))
typedef struct _CVFilter CVFilter;
typedef struct _CVFilterClass CVFilterClass;
typedef struct _CVFilterPrivate CVFilterPrivate;

struct _CVFilter
{
    GstVideoFilter base;
    CVFilterPrivate *priv;
};

struct _CVFilterClass
{
    GstVideoFilterClass base_opencv_filter_class;
};

GType cvfilter_get_type (void);

gboolean cvfilter_plugin_init (GstPlugin * plugin);

G_END_DECLS
#endif /* _cvfilter_H_ */
