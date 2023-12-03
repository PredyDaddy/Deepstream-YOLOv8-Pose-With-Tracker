#include <gst/gst.h>
#include <glib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <cuda_runtime_api.h>
#include "cuda_runtime_api.h"
#include "nvds_yml_parser.h"

#include "gstnvdsmeta.h"
#include "nvds_analytics_meta.h"
#include <gst/rtsp-server/rtsp-server.h>
#include "gstnvdsinfer.h"  // 用NvDsInferTensorMeta的
// #include "nms_kernel.h"  
#include "output.h"

// probe draw
#include "nvdsinfer_custom_impl.h"
// #include "nvosd_objects.h"
#include "nvdsmeta.h"

// gie 配置文件
#define PGIE_CONFIG_FILE "configs/dstest1_pgie_YOLOv8-Pose_config.txt"
#define MAX_DISPLAY_LEN 64
// tracking 配置文件
#define TRACKER_CONFIG_FILE "configs/tracker_config.txt"
#define MAX_TRACKING_ID_LEN 16

#define PGIE_CLASS_ID_VEHICLE 2
#define PGIE_CLASS_ID_PERSON 0

/* The muxer output resolution must be set if the input streams will be of
 * different resolution. The muxer will scale all the input frames to this
 * resolution. */
#define MUXER_OUTPUT_WIDTH 640
#define MUXER_OUTPUT_HEIGHT 640

/* Muxer batch formation timeout, for e.g. 40 millisec. Should ideally be set
 * based on the fastest source's framerate. */
#define MUXER_BATCH_TIMEOUT_USEC 40000

gint frame_number = 0;

std::ofstream logFile("log.txt", std::ios_base::app); // 创建一个文件输出流

bool is_aarch64()
{
#if defined(__aarch64__)
    return true;
#else
    return false;
#endif
}

typedef struct
{
    guint64 n_frames;
    guint64 last_fps_update_time;
    gdouble fps;
} PERF_DATA;

PERF_DATA g_perf_data = {0, 0, 0.0};


gboolean perf_print_callback(gpointer user_data)
{
    PERF_DATA *perf_data = (PERF_DATA *)user_data;
    guint64 current_time = g_get_monotonic_time();
    guint64 time_elapsed = current_time - perf_data->last_fps_update_time;

    if (time_elapsed > 0)
    {
        perf_data->fps = 1000000.0 * perf_data->n_frames / time_elapsed;
        g_print("FPS: %0.2f\n", perf_data->fps);
        perf_data->n_frames = 0;
        perf_data->last_fps_update_time = current_time;
    }

    return G_SOURCE_CONTINUE;
}
void update_frame_counter()
{
    g_perf_data.n_frames++;
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *)data;
    switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_EOS:
        g_print("End of stream\n");
        g_main_loop_quit(loop);
        break;
    case GST_MESSAGE_ERROR:
    {
        gchar *debug;
        GError *error;
        gst_message_parse_error(msg, &error, &debug);
        g_printerr("ERROR from element %s: %s\n",
                   GST_OBJECT_NAME(msg->src), error->message);
        if (debug)
            g_printerr("Error details: %s\n", debug);
        g_free(debug);
        g_error_free(error);
        g_main_loop_quit(loop);
        break;
    }
    default:
        break;
    }
    return TRUE;
}

/* Tracker config parsing */

#define CHECK_ERROR(error)                                                   \
    if (error)                                                               \
    {                                                                        \
        g_printerr("Error while parsing config file: %s\n", error->message); \
        goto done;                                                           \
    }

#define CONFIG_GROUP_TRACKER "tracker"
#define CONFIG_GROUP_TRACKER_WIDTH "tracker-width"
#define CONFIG_GROUP_TRACKER_HEIGHT "tracker-height"
#define CONFIG_GROUP_TRACKER_LL_CONFIG_FILE "ll-config-file"
#define CONFIG_GROUP_TRACKER_LL_LIB_FILE "ll-lib-file"
#define CONFIG_GROUP_TRACKER_ENABLE_BATCH_PROCESS "enable-batch-process"
#define CONFIG_GPU_ID "gpu-id"

static gchar *
get_absolute_file_path(gchar *cfg_file_path, gchar *file_path)
{
    gchar abs_cfg_path[PATH_MAX + 1];
    gchar *abs_file_path;
    gchar *delim;

    if (file_path && file_path[0] == '/')
    {
        return file_path;
    }

    if (!realpath(cfg_file_path, abs_cfg_path))
    {
        g_free(file_path);
        return NULL;
    }

    // Return absolute path of config file if file_path is NULL.
    if (!file_path)
    {
        abs_file_path = g_strdup(abs_cfg_path);
        return abs_file_path;
    }

    delim = g_strrstr(abs_cfg_path, "/");
    *(delim + 1) = '\0';

    abs_file_path = g_strconcat(abs_cfg_path, file_path, NULL);
    g_free(file_path);

    return abs_file_path;
}

static gboolean set_tracker_properties(GstElement *nvtracker)
{
    gboolean ret = FALSE;
    GError *error = NULL;
    gchar **keys = NULL;
    gchar **key = NULL;
    GKeyFile *key_file = g_key_file_new();

    if (!g_key_file_load_from_file(key_file, TRACKER_CONFIG_FILE, G_KEY_FILE_NONE,
                                   &error))
    {
        g_printerr("Failed to load config file: %s\n", error->message);
        return FALSE;
    }

    keys = g_key_file_get_keys(key_file, CONFIG_GROUP_TRACKER, NULL, &error);
    CHECK_ERROR(error);

    for (key = keys; *key; key++)
    {
        if (!g_strcmp0(*key, CONFIG_GROUP_TRACKER_WIDTH))
        {
            gint width =
                g_key_file_get_integer(key_file, CONFIG_GROUP_TRACKER,
                                       CONFIG_GROUP_TRACKER_WIDTH, &error);
            CHECK_ERROR(error);
            g_object_set(G_OBJECT(nvtracker), "tracker-width", width, NULL);
        }
        else if (!g_strcmp0(*key, CONFIG_GROUP_TRACKER_HEIGHT))
        {
            gint height =
                g_key_file_get_integer(key_file, CONFIG_GROUP_TRACKER,
                                       CONFIG_GROUP_TRACKER_HEIGHT, &error);
            CHECK_ERROR(error);
            g_object_set(G_OBJECT(nvtracker), "tracker-height", height, NULL);
        }
        else if (!g_strcmp0(*key, CONFIG_GPU_ID))
        {
            guint gpu_id =
                g_key_file_get_integer(key_file, CONFIG_GROUP_TRACKER,
                                       CONFIG_GPU_ID, &error);
            CHECK_ERROR(error);
            g_object_set(G_OBJECT(nvtracker), "gpu_id", gpu_id, NULL);
        }
        else if (!g_strcmp0(*key, CONFIG_GROUP_TRACKER_LL_CONFIG_FILE))
        {
            char *ll_config_file = get_absolute_file_path(TRACKER_CONFIG_FILE,
                                                          g_key_file_get_string(key_file,
                                                                                CONFIG_GROUP_TRACKER,
                                                                                CONFIG_GROUP_TRACKER_LL_CONFIG_FILE, &error));
            CHECK_ERROR(error);
            g_object_set(G_OBJECT(nvtracker), "ll-config-file", ll_config_file, NULL);
        }
        else if (!g_strcmp0(*key, CONFIG_GROUP_TRACKER_LL_LIB_FILE))
        {
            char *ll_lib_file = get_absolute_file_path(TRACKER_CONFIG_FILE,
                                                       g_key_file_get_string(key_file,
                                                                             CONFIG_GROUP_TRACKER,
                                                                             CONFIG_GROUP_TRACKER_LL_LIB_FILE, &error));
            CHECK_ERROR(error);
            g_object_set(G_OBJECT(nvtracker), "ll-lib-file", ll_lib_file, NULL);
        }
        else if (!g_strcmp0(*key, CONFIG_GROUP_TRACKER_ENABLE_BATCH_PROCESS))
        {
            gboolean enable_batch_process =
                g_key_file_get_integer(key_file, CONFIG_GROUP_TRACKER,
                                       CONFIG_GROUP_TRACKER_ENABLE_BATCH_PROCESS, &error);
            CHECK_ERROR(error);
            g_object_set(G_OBJECT(nvtracker), "enable_batch_process",
                         enable_batch_process, NULL);
        }
        else
        {
            g_printerr("Unknown key '%s' for group [%s]", *key,
                       CONFIG_GROUP_TRACKER);
        }
    }

    ret = TRUE;
done:
    if (error)
    {
        g_error_free(error);
    }
    if (keys)
    {
        g_strfreev(keys);
    }
    if (!ret)
    {
        g_printerr("%s failed", __func__);
    }
    return ret;
}

// This function will be called when there is a new pad to be connected
static void cb_newpad(GstElement *decodebin, GstPad *decoder_src_pad, gpointer data)
{
    g_print("In cb_newpad\n");

    GstCaps *caps = gst_pad_get_current_caps(decoder_src_pad);
    GstStructure *gststruct = gst_caps_get_structure(caps, 0);
    const gchar *gstname = gst_structure_get_name(gststruct);
    GstElement *source_bin = (GstElement *)data;
    GstCapsFeatures *features = gst_caps_get_features(caps, 0);

    g_print("gstname=%s\n", gstname);

    // Need to check if the pad created by the decodebin is for video and not audio.
    if (strstr(gstname, "video") != NULL)
    {
        // Link the decodebin pad only if decodebin has picked the NVIDIA
        // decoder plugin nvdec_*. We do this by checking if the pad caps contain
        // NVMM memory features.
        g_print("features=%s\n", gst_caps_features_to_string(features));

        if (gst_caps_features_contains(features, "memory:NVMM"))
        {
            // Get the source bin ghost pad
            GstPad *bin_ghost_pad = gst_element_get_static_pad(source_bin, "src");

            if (!gst_ghost_pad_set_target(GST_GHOST_PAD(bin_ghost_pad), decoder_src_pad))
            {
                g_printerr("Failed to link decoder src pad to source bin ghost pad\n");
            }

            gst_object_unref(bin_ghost_pad);
        }
        else
        {
            g_printerr("Error: Decodebin did not pick NVIDIA decoder plugin.\n");
        }
    }

    gst_caps_unref(caps);
}

static void decodebin_child_added(GstChildProxy *child_proxy, GObject *object, gchar *name, gpointer user_data)
{
    g_print("Decodebin child added: %s\n", name);

    if (strstr(name, "decodebin") != NULL)
    {
        g_signal_connect(object, "child-added", G_CALLBACK(decodebin_child_added), user_data);
    }
}

// 读取视频文件
GstElement *create_source_bin(guint index, const gchar *uri)
{
    g_print("Creating source bin\n");

    // Create a source GstBin to abstract this bin's content from the rest of the pipeline
    gchar bin_name[16];
    g_snprintf(bin_name, sizeof(bin_name), "source-bin-%02d", index);
    g_print("%s\n", bin_name);
    GstElement *nbin = gst_bin_new(bin_name);

    // Source element for reading from the URI
    GstElement *uri_decode_bin = gst_element_factory_make("uridecodebin", "uri-decode-bin");

    // Set the input URI to the source element
    g_object_set(G_OBJECT(uri_decode_bin), "uri", uri, NULL);

    // Connect to the "pad-added" signal of the decodebin
    g_signal_connect(uri_decode_bin, "pad-added", G_CALLBACK(cb_newpad), nbin);
    g_signal_connect(uri_decode_bin, "child-added", G_CALLBACK(decodebin_child_added), nbin);

    // Add the URI decode bin to the source bin
    gst_bin_add(GST_BIN(nbin), uri_decode_bin);

    // Create a ghost pad for the source bin
    GstPad *bin_pad = gst_ghost_pad_new_no_target("src", GST_PAD_SRC);
    if (!bin_pad)
    {
        g_printerr("Failed to add ghost pad in source bin\n");
        return NULL;
    }

    gst_element_add_pad(nbin, bin_pad);
    return nbin;
}

// // Output is a start point
// struct Output {
//     float xywh[4];
//     float confidence;
//     float kpts[51];
// };

// xy为中心点的iou
float iou(Output& box1, Output& box2) {
    // 计算边界框的左上角和右下角
    float box1_x1 = box1.xywh[0] - box1.xywh[2] / 2.0;
    float box1_y1 = box1.xywh[1] - box1.xywh[3] / 2.0;
    float box1_x2 = box1.xywh[0] + box1.xywh[2] / 2.0;
    float box1_y2 = box1.xywh[1] + box1.xywh[3] / 2.0;
    
    float box2_x1 = box2.xywh[0] - box2.xywh[2] / 2.0;
    float box2_y1 = box2.xywh[1] - box2.xywh[3] / 2.0;
    float box2_x2 = box2.xywh[0] + box2.xywh[2] / 2.0;
    float box2_y2 = box2.xywh[1] + box2.xywh[3] / 2.0;

    // 计算交集的坐标
    float x1 = std::max(box1_x1, box2_x1);
    float y1 = std::max(box1_y1, box2_y1);
    float x2 = std::min(box1_x2, box2_x2);
    float y2 = std::min(box1_y2, box2_y2);

    // 计算交集的面积
    float intersection = std::max(0.0f, x2 - x1) * std::max(0.0f, y2 - y1);

    // 计算并集的面积
    float union_ = box1.xywh[2] * box1.xywh[3] + box2.xywh[2] * box2.xywh[3] - intersection;

    // 返回交并比
    return intersection / union_;
}

// 对检测结果应用非极大值抑制 (NMS)
std::vector<Output> applyNMS(std::vector<Output>& boxes, float iou_threshold) {
    // 先根据置信度进行排序
    std::sort(boxes.begin(), boxes.end(), [](const Output& a, const Output& b) { return a.confidence > b.confidence; });
    
    std::vector<Output> kept_boxes;
    
    while (!boxes.empty()) {
        // 保留置信度最大的边界框
        Output current_box = boxes[0];
        kept_boxes.push_back(current_box);
        
        // 移除已处理的边界框
        boxes.erase(boxes.begin());
        
        // 检查剩余的边界框
        for (auto it = boxes.begin(); it != boxes.end(); ) {
            // 如果当前边界框与其他边界框的交并比大于阈值，则移除该边界框
            if (iou(current_box, *it) > iou_threshold) {
                it = boxes.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    return kept_boxes;
}

// osd_sink_pad_buffer_probe
static GstPadProbeReturn osd_sink_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    GstBuffer *buf = (GstBuffer *)info->data;
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
    // 检查batch_meta是否为NULL
    // if (batch_meta == NULL) {
    //     std::cout << "batch_meta is NULL" << std::endl;
    // } else {
    //     std::cout << "batch_meta is not NULL" << std::endl;
    // }

    for (NvDsMetaList *l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next)
    {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *)(l_frame->data);
        // if (frame_meta == NULL) {
        //     std::cout << "frame_meta is NULL" << std::endl;
        // } else {
        //     std::cout << "frame_meta is not NULL" << std::endl;
        // }

        for (NvDsMetaList *l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next)
        {
            NvDsUserMeta *user_meta = (NvDsUserMeta *) l_user->data;
            // if (user_meta == NULL) {
            //     std::cout << "user_meta is NULL" << std::endl;
            // } else {
            //     std::cout << "user_meta is not NULL" << std::endl;
            // }

            if (user_meta->base_meta.meta_type != NVDSINFER_TENSOR_OUTPUT_META)
                continue;

            NvDsInferTensorMeta *meta = (NvDsInferTensorMeta *) user_meta->user_meta_data;
            NvDsInferNetworkInfo network_info = meta->network_info;

            for (unsigned int i = 0; i < meta->num_output_layers; i++)
            {
                NvDsInferLayerInfo *info = &meta->output_layers_info[i];
                info->buffer = meta->out_buf_ptrs_host[i];
            }

            std::vector <NvDsInferLayerInfo> outputLayersInfo(meta->output_layers_info, meta->output_layers_info + meta->num_output_layers);

            NvDsInferLayerInfo* info = &meta->output_layers_info[0];
            float* data = static_cast<float*>(info->buffer);
            std::vector<Output> outputs(8400);

            for (int col = 0; col < 8400; ++col)
            {
                Output& output = outputs[col];

                for (int row = 0; row < 56; ++row)
                {
                    int index = row * 8400 + col;
                    float value = data[index];

                    if (row < 4)
                    {
                        output.xywh[row] = value;
                    }
                    else if (row == 4)
                    {
                        output.confidence = value;
                    }
                    else
                    {
                        output.kpts[row - 5] = value;
                    }
                }
            }

            // 将输出的坐标从相对坐标转换为绝对坐标, 提升的时候用就好了
            // float scaleX = MUXER_OUTPUT_WIDTH / 640;  // Assuming your network was trained on 300x300 images
            // float scaleY = MUXER_OUTPUT_HEIGHT / 640; // Assuming your network was trained on 300x300 images

            // for (Output& output : outputs)
            // {
            //     output.xywh[0] *= scaleX;
            //     output.xywh[1] *= scaleY;
            //     output.xywh[2] *= scaleX;
            //     output.xywh[3] *= scaleY;
            // }

            std::vector<Output> confident_outputs;
            for (const Output& output : outputs)
            {
                if (output.confidence >= 0.25f)
                {
                    confident_outputs.push_back(output);
                }
            }

            // 对输出的数据执行NMS操作, CPU
            std::vector<Output> filtered_outputs = applyNMS(confident_outputs, 0.65f);

            // 颜色数组，按顺序为红色，绿色，蓝色，黄色
            NvOSD_ColorParams color_arr[] = {
                {1.0, 0.0, 0.0, 1.0},
                {0.0, 1.0, 0.0, 1.0},
                {0.0, 0.0, 1.0, 1.0},
                {1.0, 1.0, 0.0, 1.0}
            };

            // 绘制
            for (const Output& output : filtered_outputs) 
            {
                NvDsDisplayMeta *display_meta = nvds_acquire_display_meta_from_pool(batch_meta);

                nvds_add_display_meta_to_frame(frame_meta, display_meta);

                // 绘制四边形
                NvOSD_RectParams *rect_params = &display_meta->rect_params[display_meta->num_rects];
                rect_params->left = static_cast<unsigned int>(output.xywh[0] - output.xywh[2] / 2.0);
                rect_params->top = static_cast<unsigned int>(output.xywh[1] - output.xywh[3] / 2.0);
                rect_params->width = static_cast<unsigned int>(output.xywh[2]);
                rect_params->height = static_cast<unsigned int>(output.xywh[3]);
                rect_params->border_width = 2;
                rect_params->border_color = NvOSD_ColorParams{0.0, 0.0, 1.0, 0.8};
                rect_params->has_bg_color = 1;
                rect_params->bg_color = NvOSD_ColorParams{0.0, 0.0, 1.0, 0.5};  // 粉红色背景，半透明
                display_meta->num_rects++;

                // 绘制关键点
                for (int i = 4; i < 17; ++i)  // 不绘制前4个点
                {
                    if (display_meta->num_circles == MAX_ELEMENTS_IN_DISPLAY_META)
                    {
                        display_meta = nvds_acquire_display_meta_from_pool(batch_meta);
                        nvds_add_display_meta_to_frame(frame_meta, display_meta);
                    }
                    NvOSD_CircleParams &circle_params = display_meta->circle_params[display_meta->num_circles];
                    circle_params.xc = static_cast<unsigned int>(output.kpts[i*3]);
                    circle_params.yc = static_cast<unsigned int>(output.kpts[i*3 + 1]);
                    circle_params.radius = 3;
                    circle_params.has_bg_color = 0;
                    circle_params.circle_color = color_arr[i%4];  // 循环使用颜色数组中的颜色
                    display_meta->num_circles++;
                }
            }
        }
    }

    frame_number++;
    update_frame_counter();
    return GST_PAD_PROBE_OK;
}

// Main function
int main(int argc, char *argv[])
{
    GMainLoop *loop = NULL;
    // Create various elements
    GstElement *pipeline = NULL, *source = NULL, *streammux = NULL, *pgie = NULL, *nvtracker = NULL, *nvvidconv = NULL,
               *nvosd = NULL, *nvvidconv_postosd = NULL, *caps = NULL, *encoder = NULL, *rtppay = NULL, *sink = NULL;

    GstBus *bus = NULL;
    guint bus_watch_id = 0;
    GstPad *osd_sink_pad = NULL;
    GstCaps *caps_filter = NULL;

    guint bitrate = 5000000;
    gchar *codec = "H264";

    int current_device = -1;
    cudaGetDevice(&current_device);
    struct cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, current_device);

    /* Check input arguments */
    if (argc != 2)
    {
        g_printerr("Usage: %s <H264 filename>\n", argv[0]);
        return -1;
    }

    /* Standard GStreamer initialization */
    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

    // Create elements

    pipeline = gst_pipeline_new("v8-pipeline"); // Create pipeline

    source = create_source_bin(0, argv[1]);
    streammux = gst_element_factory_make("nvstreammux", "stream-muxer");
    pgie = gst_element_factory_make("nvinfer", "primary-nvinference-engine");
    nvtracker = gst_element_factory_make("nvtracker", "tracker");
    nvvidconv = gst_element_factory_make("nvvideoconvert", "nvvideo-converter");
    nvosd = gst_element_factory_make("nvdsosd", "nv-onscreendisplay");
    nvvidconv_postosd = gst_element_factory_make("nvvideoconvert", "convertor_postosd");
    caps = gst_element_factory_make("capsfilter", "filter");

    // Create encoder
    if (g_strcmp0(codec, "H264") == 0)
    {
        encoder = gst_element_factory_make("nvv4l2h264enc", "encoder");
        printf("Creating H264 Encoder\n");
    }
    else if (g_strcmp0(codec, "H265") == 0)
    {
        encoder = gst_element_factory_make("nvv4l2h265enc", "encoder");
        printf("Creating H265 Encoder\n");
    }

    if (g_strcmp0(codec, "H264") == 0)
    {
        rtppay = gst_element_factory_make("rtph264pay", "rtppay");
        printf("Creating H264 rtppay\n");
    }
    else if (g_strcmp0(codec, "H265") == 0)
    {
        rtppay = gst_element_factory_make("rtph265pay", "rtppay");
        printf("Creating H265 rtppay\n");
    }

    // Change sink to filesink to save the output to a file
    sink = gst_element_factory_make("filesink", "filesink");
    if (!sink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    g_object_set(G_OBJECT(sink), "location", "output_video.mp4", NULL);

    // Set streammux properties
    g_object_set(G_OBJECT(streammux), "batch-size", 1, NULL);
    g_object_set(G_OBJECT(streammux), "width", MUXER_OUTPUT_WIDTH, "height",
                 MUXER_OUTPUT_HEIGHT,
                 "batched-push-timeout", MUXER_BATCH_TIMEOUT_USEC, NULL);
    // g_object_set(G_OBJECT(streammux), "enable-padding", 1, NULL);

    // Set pgie properties
    g_object_set(G_OBJECT(pgie), "config-file-path", PGIE_CONFIG_FILE, NULL);

    set_tracker_properties(nvtracker);

    caps_filter = gst_caps_from_string("video/x-raw(memory:NVMM), format=I420, width=1920, height=1080");
    g_object_set(G_OBJECT(caps), "caps", caps_filter, NULL);

    gst_caps_unref(caps_filter);

    g_object_set(G_OBJECT(encoder), "bitrate", bitrate, NULL);

    if (is_aarch64())
    {
        g_object_set(G_OBJECT(encoder), "preset-level", 1, NULL);
        g_object_set(G_OBJECT(encoder), "insert-sps-pps", 1, NULL);
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    gst_bin_add_many(GST_BIN(pipeline),
                     source, streammux, pgie, nvtracker,
                     nvvidconv, nvosd, nvvidconv_postosd, caps, encoder, rtppay, sink, NULL);

    GstPad *sinkpad, *srcpad;
    gchar pad_name_sink[16] = "sink_0";
    gchar pad_name_src[16] = "src";

    sinkpad = gst_element_get_request_pad(streammux, pad_name_sink);

    srcpad = gst_element_get_static_pad(source, pad_name_src);

    if (gst_pad_link(srcpad, sinkpad) != GST_PAD_LINK_OK)
    {
        g_printerr("Failed to link decoder to stream muxer. Exiting.\n");
        return -1;
    }

    gst_object_unref(sinkpad);
    gst_object_unref(srcpad);

    if (!gst_element_link_many(streammux, pgie, nvtracker, nvvidconv, nvosd, nvvidconv_postosd, caps, encoder, sink, NULL)) {
        g_printerr("Elements could not be linked.\n");
        return -1;
    }

    osd_sink_pad = gst_element_get_static_pad(nvosd, "sink");
    if (!osd_sink_pad)
        g_print("Unable to get sink pad\n");
    else

        gst_pad_add_probe(osd_sink_pad, GST_PAD_PROBE_TYPE_BUFFER, osd_sink_pad_buffer_probe, NULL, NULL);
    g_timeout_add(5000, perf_print_callback, &g_perf_data);
    gst_object_unref(osd_sink_pad);

    /* Set the pipeline to "playing" state */
    g_print("Using file: %s\n", argv[1]);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    /* Iterate */
    g_print("Running...\n");
    g_main_loop_run(loop);

    /* Out of the main loop, clean up nicely */
    g_print("Returned, stopping playback\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    g_print("Deleting pipeline\n");
    gst_object_unref(GST_OBJECT(pipeline));
    g_source_remove(bus_watch_id);
    g_main_loop_unref(loop);
    return 0;
}
