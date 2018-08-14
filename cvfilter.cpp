#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cvfilter.h"

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include <glib/gstdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <chrono>
#define PLUGIN_NAME "calib"

#define CVFILTER_LOCK(opencv_filter)   (g_rec_mutex_lock (&( (CVFilter *) opencv_filter)->priv->mutex))

#define CVFILTER_UNLOCK(opencv_filter) (g_rec_mutex_unlock (&( (CVFilter *) opencv_filter)->priv->mutex))

GST_DEBUG_CATEGORY_STATIC (cvfilter_debug_category);
#define GST_CAT_DEFAULT cvfilter_debug_category

#define CVFILTER_GET_PRIVATE(obj) ( G_TYPE_INSTANCE_GET_PRIVATE ((obj),C_TYPE_OPENCV_FILTER,CVFilterPrivate) )

enum {
    PROP_0,
    PROP_TARGET_OBJECT,
    N_PROPERTIES
};

struct _CVFilterPrivate {
    GRecMutex mutex;
    cv::Mat *cv_image;
};

/* pad templates */

#define CHANNEL_NUM 3
#define SRC1 "{I420, NV12, NV21, YV12, YUY2}"
#define SRC2 "{BGR}"
#define SRC3 "{BGRA}"
#define CAPS "video/x-raw,format=(string)YV12,framerate=(fraction)30/1"

#define DST "{I420, NV12, NV21, YV12, YUY2, Y42B, Y444, YUV9, YVU9, Y41B, Y800,"\
    "Y8, GREY, Y16 , UYVY, YVYU, IYU1, v308, AYUV, A420}"

#define VIDEO_SRC_CAPS  GST_VIDEO_CAPS_MAKE(SRC2)
#define VIDEO_SINK_CAPS GST_VIDEO_CAPS_MAKE(SRC2)

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (CVFilter, cvfilter, GST_TYPE_VIDEO_FILTER,
                         GST_DEBUG_CATEGORY_INIT (cvfilter_debug_category,
                                                  PLUGIN_NAME, 0, "debug category for opencv_filter element") )

static void cvfilter_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
    CVFilter *opencv_filter = cvfilter (object);

    CVFILTER_LOCK (opencv_filter);

    switch (property_id) {
    case PROP_TARGET_OBJECT:

        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }

    CVFILTER_UNLOCK (opencv_filter);
}

static void cvfilter_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
    CVFilter *opencv_filter = cvfilter (object);

    CVFILTER_LOCK (opencv_filter);

    switch (property_id) {
    case PROP_TARGET_OBJECT:
        //    g_value_set_pointer (value, (gpointer) opencv_filter->priv->object);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }

    CVFILTER_UNLOCK (opencv_filter);
}

static void cvfilter_initialize_images (CVFilter *o, GstVideoFrame *frame, GstMapInfo &info) {

    int w = frame->info.width, h = frame->info.height;
    cv::Mat **mat = &o->priv->cv_image;
    if (*mat == NULL) {
        *mat = new cv::Mat (h, w, CV_MAKETYPE(CV_8U,CHANNEL_NUM), info.data);
    } else if ( (*mat)->cols == w && (*mat)->rows == h ) {
        (*mat)->data = info.data;
    } else {
        delete *mat;
        *mat = new cv::Mat (h, w, CV_MAKETYPE(CV_8U,CHANNEL_NUM), info.data);
    }
}

static std::string currentDateTimeString() {
    using namespace std;
    using namespace std::chrono;

    auto tp = high_resolution_clock::now();
    auto ttime_t = high_resolution_clock::to_time_t(tp);
    auto tp_sec = high_resolution_clock::from_time_t(ttime_t);
    milliseconds ms = duration_cast<milliseconds>(tp - tp_sec);

    std::tm * ttm = localtime(&ttime_t);
    char date_time_format[] = "%Y-%m-%d-%H:%M:%S";
    char time_str[] = "yyyy-mm-dd-HH:MM:SS.fff";
    char ms_str[] = "ffff";

    strftime(time_str, strlen(time_str), date_time_format, ttm);
    snprintf(ms_str, strlen(ms_str), "%03lu", ms.count());

    string result(time_str);
    result.append(".");
    result.append(ms_str);
    //    result.append(to_string(ms.count()));

    return result;
}

static void process(cv::Mat &frame){

    static int cnt = 0;
    std::string stamp = currentDateTimeString();
    auto n = std::chrono::high_resolution_clock::now();
    static auto last = n;
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(n-last).count();
    std::cerr <<stamp<< frame.size()<<", "<< diff <<"ms, "<<++cnt<<std::endl;

    int w= frame.cols,h=frame.rows;

    cv::circle(frame, cv::Point(100,300),100,cv::Scalar(0, 200, 0),-1,8);
    cv::putText(frame,stamp,cv::Point(w/4,h/4),cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,0,255,255),3,cv::LINE_AA);

    last = n;
}

static GstFlowReturn cvfilter_transform_frame_ip (GstVideoFilter *filter, GstVideoFrame *frame) {
    CVFilter *opencv_filter = cvfilter (filter);
    GstMapInfo info;
    gst_buffer_map (frame->buffer, &info, GST_MAP_READ);
    cvfilter_initialize_images (opencv_filter, frame, info);

    try {
        CVFILTER_LOCK (opencv_filter);
        process (*opencv_filter->priv->cv_image);
        CVFILTER_UNLOCK (opencv_filter);

    } catch (const std::exception & e) {
        GstMessage *message;
        GError *err = g_error_new (g_quark_from_string (e.what() ), 1, "%s", GST_ELEMENT_NAME (opencv_filter) );

        message = gst_message_new_error (GST_OBJECT (opencv_filter),err, e.what() );

        gst_element_post_message (GST_ELEMENT (opencv_filter), message);

        g_clear_error (&err);
    } catch (...) {
        GstMessage *message;
        GError *err = g_error_new (g_quark_from_string ("UNDEFINED_EXCEPTION"),  0, "%s", GST_ELEMENT_NAME (opencv_filter) );

        message = gst_message_new_error (GST_OBJECT (opencv_filter),  err, "Undefined filter error");

        gst_element_post_message (GST_ELEMENT (opencv_filter), message);

        g_clear_error (&err);
    }

    gst_buffer_unmap (frame->buffer, &info);
    return GST_FLOW_OK;
}

static void cvfilter_dispose (GObject *object){
}

static void cvfilter_finalize (GObject *object) {
    CVFilter *opencv_filter = cvfilter (object);

    delete opencv_filter->priv->cv_image;
    opencv_filter->priv->cv_image = NULL;

    g_rec_mutex_clear (&opencv_filter->priv->mutex);
}

static void cvfilter_init (CVFilter *opencv_filter) {
    opencv_filter->priv = CVFILTER_GET_PRIVATE (opencv_filter);
    g_rec_mutex_init (&opencv_filter->priv->mutex);
}

static void cvfilter_class_init (CVFilterClass *klass){
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstVideoFilterClass *video_filter_class = GST_VIDEO_FILTER_CLASS (klass);

    GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, PLUGIN_NAME, 0, PLUGIN_NAME);

    gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
                                        gst_pad_template_new ("src", GST_PAD_SRC,
                                                              GST_PAD_ALWAYS,
                                                              gst_caps_from_string (VIDEO_SRC_CAPS) ) );
    gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
                                        gst_pad_template_new ("sink", GST_PAD_SINK,
                                                              GST_PAD_ALWAYS,
                                                              gst_caps_from_string (VIDEO_SINK_CAPS) ) );

    gst_element_class_set_static_metadata (GST_ELEMENT_CLASS (klass),
                                           "camera calib library", "Video/Filter",
                                           "Create a generic opencv filter to process images",
                                           "wayne wang <cymatics@foxmail.com>");

    gobject_class->set_property = cvfilter_set_property;
    gobject_class->get_property = cvfilter_get_property;
    gobject_class->dispose = cvfilter_dispose;
    gobject_class->finalize = cvfilter_finalize;

    g_object_class_install_property (gobject_class, PROP_TARGET_OBJECT,
                                     g_param_spec_pointer ("target-object", "target object",
                                                           "Reference to target object",
                                                           (GParamFlags) G_PARAM_READWRITE) );

    video_filter_class->transform_frame_ip = GST_DEBUG_FUNCPTR (cvfilter_transform_frame_ip);

    /* Properties initialization */
    g_type_class_add_private (klass, sizeof (CVFilterPrivate) );
}

gboolean cvfilter_plugin_init (GstPlugin *plugin) {
    return gst_element_register (plugin, PLUGIN_NAME, GST_RANK_NONE,C_TYPE_OPENCV_FILTER);
}
