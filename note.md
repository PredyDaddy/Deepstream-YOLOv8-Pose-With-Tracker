# 1. 管道元素的说明：

在给定的代码中，使用了以下几个元素来构建 DeepStream 管道：

1. `source`：这个元素是用于从文件中读取视频流的输入源。它可以是任何 GStreamer 支持的视频输入源，如文件、摄像头、网络流等。

2. `streammux`：这个元素是流复用器，用于将多个流合并成一个流，并将多帧画面打包为批处理数据。它可以将不同源的视频流合并为一个统一的输入。

3. `pgie`：这个元素是主要的推理引擎（Primary GIE），用于执行物体检测和推理。它基于给定的配置文件进行推理，识别出图像中的物体并提取其特征。

4. `nvtracker`：这个元素是 DeepStream 中的跟踪器，用于对识别到的物体进行跟踪。它利用先前识别到的物体特征和当前帧中的特征进行匹配和跟踪，以实现物体的持续跟踪。

5. `nvvidconv`：这个元素是用于将视频帧格式从 NV12 转换为 RGBA 的视频转换器。在某些情况下，需要将视频帧从一种格式转换为另一种格式，以适应不同元素的要求。

6. `nvosd`：这个元素是 On-Screen Display (OSD) 元素，用于在转换后的 RGBA 缓冲区上绘制识别结果、边界框、标签等信息。

7. `nvvidconv_postosd`：这个元素是用于将转换后的 RGBA 格式的视频帧再次转换为 NV12 格式的视频转换器。这在将视频帧发送到编码器之前很常见。

8. `caps`：这个元素是 Caps Filter 元素，用于设置视频格式的约束条件。它可以指定输入或输出流的特定格式和参数，以确保流的兼容性。

9. `encoder`：这个元素是视频编码器，用于将原始视频帧编码为特定的视频编码格式，如 H.264 或 H.265。它根据指定的参数设置比特率、编码质量等。

10. `rtppay`：这个元素用于将编码后的数据打包为 RTP（Real-time Transport Protocol）数据包。RTP 是一种常用的实时流传输协议。

11. `sink`：这个元素是 UDPSink 元素，用于将 RTP 数据包通过 UDP 协议发送到网络。它通过指定目标 IP 地址和端口号来指定数据的接收位置。

以上是在给定的代码片段中使用的一些关键元素，它们在 DeepStream 管道中扮演着不同的角色，负责视频的输入、推理、跟踪、转换、绘制和输出等功能。

# 2. pipline的构建:
这是一个使用GStreamer构建的DeepStream管道。让我们逐步解释代码中的主要构建过程：

1. 首先，定义了一系列用于构建管道所需的变量和参数，包括`GstElement`指针、比特率、编码格式、端口号等。

2. 接下来，创建了GStreamer的各个元素，例如`source`、`streammux`、`pgie`、`nvtracker`等。这些元素用于处理视频流的输入、推理、跟踪和输出等功能。

3. 设置了各个元素的参数。例如，设置`streammux`的批处理大小和输出分辨率，设置`pgie`的配置文件路径，设置`nvtracker`的属性等。

4. 将各个元素添加到管道中。使用`gst_bin_add_many()`函数将元素添加到GStreamer的管道中，以便进行管理和链接。

5. 连接元素之间的数据流。使用`gst_element_link_many()`函数将元素链接在一起，以定义数据的流动路径。

6. 添加探针。使用`gst_pad_add_probe()`函数向`pgie_src_pad`和`osd_sink_pad`添加探针，用于获取元数据和处理缓冲区。

7. 创建RTSP服务器。使用`gst_rtsp_server_new()`创建一个RTSP服务器，设置服务器的服务端口号，并将RTSP流挂载到服务器上。

8. 设置管道状态为"播放"。使用`gst_element_set_state()`函数将管道设置为播放状态，开始视频流的处理和输出。

9. 启动主循环。使用`g_main_loop_run()`函数启动GStreamer的主循环，该循环用于处理事件和消息。

10. 等待退出。一直等待主循环结束，直到收到退出信号。

11. 清理和释放资源。在退出主循环后，通过设置管道状态为NULL、释放管道资源和清理其他资源来完成清理工作。

以上是该代码构建DeepStream管道的主要过程。这个管道用于读取视频文件，执行推理和跟踪，然后输出处理结果，并通过RTSP流发布到网络上。

# 3. pgie探针函数主要功能
这个Pgie回调函数的主要功能如下：

1. 获取GStreamer的buffer，并从中获取batch metadata。

2. 遍历每一帧的metadata。

3. 对于每一帧，遍历其用户metadata。

4. 如果用户metadata的类型是tensor output，那么将其转换为NvDsInferTensorMeta类型。

5. 获取模型的输入形状和输出层的信息。

6. 将输出层的数据从C类型转换为Python的numpy数组。

7. 对模型的输出进行后处理，包括调整维度、添加假的类别概率、将坐标映射到屏幕尺寸等。

8. 对处理后的输出进行进一步的后处理，包括非极大值抑制等。

9. 如果存在有效的预测结果，那么将这些结果添加到帧的对象metadata中，并显示在帧上。

10. 更新帧的帧率。

11. 标记该帧已经进行过推理。

将这个函数实现的大致步骤如下：

1. 获取GStreamer的buffer，并从中获取batch metadata。这一步使用gst_buffer_get_nvds_batch_meta()函数来获取batch metadata。

2. 遍历每一帧的metadata。这一步在C++中可以使用标准的迭代器或循环来完成。

3. 对于每一帧，遍历其用户metadata。这一步在C++中可以使用标准的迭代器或循环来完成。

4. 如果用户metadata的类型是tensor output，那么将其转换为NvDsInferTensorMeta类型。这一步使用NvDsInferNetworkInfo和NvDsInferLayerInfo来获取这些信息。

5. 获取模型的输入形状和输出层的信息。这一步在C++中可以使用DeepStream的API来完成。

6. 将输出层的数据从C类型转换为C++的数组或向量。这一步在C++中可以使用标准的数组或向量来完成。

7. 对模型的输出进行后处理，包括调整维度、添加假的类别概率、将坐标映射到屏幕尺寸等。可以使用nvds_add_display_meta_to_frame()函数来添加显示metadata到帧中。

8. 对处理后的输出进行进一步的后处理，包括非极大值抑制等。这一步在C++中可能需要使用或实现相应的算法。

9. 如果存在有效的预测结果，那么将这些结果添加到帧的对象metadata中，并显示在帧上。这一步在C++中可以使用DeepStream的API来完成。

10. 更新帧的帧率。这一步可以设置frame_meta->bInferDone为true来标记该帧已经进行过推理。

11. 标记该帧已经进行过推理。这一步在C++中可以使用DeepStream的API来完成。

以上就是将这个Python函数转换为C++的大致步骤。具体的代码实现可能会根据你的具体需求和环境有所不同。

# 4. 分步骤实现这个回调函数

## 4.1 获取GStreamer的buffer
```cpp
static GstPadProbeReturn pose_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    g_print("pose_src_pad_buffer_probe called\n");

    // 获取GstBuffer
    GstBuffer *buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buf) {
        g_print("Unable to get GstBuffer\n");
        return GST_PAD_PROBE_OK;
    }

    // 获取batch metadata
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
    if (!batch_meta) {
        g_print("Unable to get batch metadata\n");
        return GST_PAD_PROBE_OK;
    }

    // 打印一些信息
    g_print("Successfully got GstBuffer and batch metadata\n");
    g_print("Batch meta frame count: %d\n", batch_meta->num_frames_in_batch);

    return GST_PAD_PROBE_OK;
}
```

## 4.2 遍历: batch metadata ->  frame_meta_list ->  user metadata   
```cpp
static GstPadProbeReturn pose_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    g_print("pose_src_pad_buffer_probe called\n");

    // 获取GstBuffer
    GstBuffer *buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buf) {
        g_print("Unable to get GstBuffer\n");
        return GST_PAD_PROBE_OK;
    }

    // 获取batch metadata
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
    if (!batch_meta) {
        g_print("Unable to get batch metadata\n");
        return GST_PAD_PROBE_OK;
    }

    // 遍历每一帧的元数据
    for (NvDsMetaList *l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next) {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *)(l_frame->data);

        // 对于每一帧，遍历其用户metadata
        for (NvDsMetaList *l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next) {
            NvDsUserMeta *user_meta = (NvDsUserMeta *)(l_user->data);
            g_print("Successfully got user metadata\n");
            g_print("User metadata type: %d\n", user_meta->base_meta.meta_type);
        }
    }

    return GST_PAD_PROBE_OK;
}
```

```
User metadata type: 12
```
这是NvDsMetaType枚举的一部分定义：
```
typedef enum
{
  NVDS_META_INVALID = 0,
  NVDS_META_FRAME_INFO,
  NVDS_META_EVENT_MSG,
  NVDS_META_STREAM_INFO,
  NVDS_META_SOURCE_INFO,
  NVDS_META_USER,
  NVDS_META_RESERVED_1,
  NVDS_META_RESERVED_2,
  NVDS_META_RESERVED_3,
  NVDS_META_RESERVED_4,
  NVDS_META_RESERVED_5,
  NVDS_META_RESERVED_6,
  NVDSINFER_TENSOR_OUTPUT_META = 12,
  /* More types */
} NvDsMetaType;
```
**这意味着这个用户元数据是一个推理张量输出元数据，它包含了模型推理的结果**


## 4.3 取出这个数据
用到这个，里面有解释对应的头文件是什么，我代码里面注释也有的
https://docs.nvidia.com/metropolis/deepstream/4.0/dev-guide/DeepStream_Development_Guide/baggage/structNvDsInferTensorMeta.html

```cpp
static GstPadProbeReturn pose_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    g_print("pose_src_pad_buffer_probe called\n");

    // 获取GstBuffer
    GstBuffer *buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buf) {
        g_print("Unable to get GstBuffer\n");
        return GST_PAD_PROBE_OK;
    }

    // 获取batch metadata
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
    if (!batch_meta) {
        g_print("Unable to get batch metadata\n");
        return GST_PAD_PROBE_OK;
    }

    // 遍历每一帧的元数据
    for (NvDsMetaList *l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next) {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *)(l_frame->data);

        // 对于每一帧，遍历其用户metadata
        for (NvDsMetaList *l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next) 
        {
            NvDsUserMeta *user_meta = (NvDsUserMeta *)(l_user->data);
            
            // 如果用户metadata的类型是tensor output，那么将其转换为NvDsInferTensorMeta类型
            if (user_meta->base_meta.meta_type == 12) {
                NvDsInferTensorMeta *tensor_meta = (NvDsInferTensorMeta *)(user_meta->user_meta_data);
                g_print("Successfully casted user metadata to tensor metadata\n");
            }
        }
    }

    return GST_PAD_PROBE_OK;
}
```

## 4.4 使用Tensor_Meta去获取模型的输入输出

做这一步是为了确保数据读取正确，因为本项目是做的Yolov8-pose, 输入是3x640x640 输出是56x8400

**56 = bbox(4) + confidence(1) + keypoints(3 x 17) = 4 + 1 + 0 + 51 = 56**

如果这里使用的是yolov7-pose, 输出就是57

**bbox(4) + confidence(1) + cls(1) + keypoints(3 x 17) = 4 + 1 + 1 + 51 = 57**

```cpp
static GstPadProbeReturn pose_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    g_print("pose_src_pad_buffer_probe called\n");

    // 获取GstBuffer
    GstBuffer *buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buf) {
        g_print("Unable to get GstBuffer\n");
        return GST_PAD_PROBE_OK;
    }

    // 获取batch metadata
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
    if (!batch_meta) {
        g_print("Unable to get batch metadata\n");
        return GST_PAD_PROBE_OK;
    }

    // 遍历每一帧的元数据
    for (NvDsMetaList *l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next) {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *)(l_frame->data);

        // 对于每一帧，遍历其用户metadata
        for (NvDsMetaList *l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next) 
        {
            NvDsUserMeta *user_meta = (NvDsUserMeta *)(l_user->data);
            
            // 如果用户metadata的类型是tensor output，那么将其转换为NvDsInferTensorMeta类型
            if (user_meta->base_meta.meta_type == 12) {
                NvDsInferTensorMeta *tensor_meta = (NvDsInferTensorMeta *)(user_meta->user_meta_data);

                // 获取模型的输入形状
                NvDsInferNetworkInfo network_info = tensor_meta->network_info;
                g_print("Model input shape: %d x %d x %d\n", network_info.channels, network_info.height, network_info.width);

                // 获取模型的输出层信息
                for (unsigned int i = 0; i < tensor_meta->num_output_layers; i++) {
                    NvDsInferLayerInfo output_layer_info = tensor_meta->output_layers_info[i];
                    NvDsInferDims dims = output_layer_info.inferDims;
                    g_print("Output layer %d: %s, dimensions: ", i, output_layer_info.layerName);
                    for (int j = 0; j < dims.numDims; j++) {
                        g_print("%d ", dims.d[j]);
                    }
                    g_print("\n");
                }

            }
        }
    }

    return GST_PAD_PROBE_OK;
}
```

跟TensorRT推理的结果进行对齐
```
INFO: [Implicit Engine Info]: layers num: 2
0   INPUT  kFLOAT images          3x640x640       
1   OUTPUT kFLOAT output0         56x8400  
```

下面是我们打印出来的结果

```
Model input shape: 3 x 640 x 640
Output layer 0: output0, dimensions: 56 8400
```

这里有个问题是我们的yolov8-pose只有一个output也就是这个output 0, 所以还可以写的再简单一点, 就不用遍历多个循环了
```cpp
static GstPadProbeReturn pose_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    g_print("pose_src_pad_buffer_probe called\n");

    // 获取GstBuffer
    GstBuffer *buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buf) {
        g_print("Unable to get GstBuffer\n");
        return GST_PAD_PROBE_OK;
    }

    // 获取batch metadata
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
    if (!batch_meta) {
        g_print("Unable to get batch metadata\n");
        return GST_PAD_PROBE_OK;
    }

    // 遍历每一帧的元数据
    for (NvDsMetaList *l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next) {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *)(l_frame->data);

        // 对于每一帧，遍历其用户metadata
        for (NvDsMetaList *l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next) 
        {
            NvDsUserMeta *user_meta = (NvDsUserMeta *)(l_user->data);
            
            // 如果用户metadata的类型是tensor output，那么将其转换为NvDsInferTensorMeta类型
            if (user_meta->base_meta.meta_type == 12) {
                NvDsInferTensorMeta *tensor_meta = (NvDsInferTensorMeta *)(user_meta->user_meta_data);

                // 获取模型的输入形状
                NvDsInferNetworkInfo network_info = tensor_meta->network_info;
                g_print("Model input shape: %d x %d x %d\n", network_info.channels, network_info.height, network_info.width);

                // 获取模型的输出层信息
                NvDsInferLayerInfo output_layer_info = tensor_meta->output_layers_info[0];  // 直接访问第一个输出层
                NvDsInferDims dims = output_layer_info.inferDims;
                g_print("Output layer 0: %s, dimensions: %d %d %d\n", output_layer_info.layerName, dims.d[0], dims.d[1], dims.d[2] );
            }
        }
    }

    return GST_PAD_PROBE_OK;
}
```

## 4.5 获取模型的输出数据 这里开始参照nvidia 官方的案例了

**CPU版本的**
```cpp
static GstPadProbeReturn pose_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    static guint use_device_mem = 0;
    // g_print("pose_src_pad_buffer_probe called\n");

    // 获取GStreamer的buffer，并从中获取batch metadata
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (GST_BUFFER (info->data));

    // 遍历每一帧的metadata
    for (NvDsMetaList * l_frame = batch_meta->frame_meta_list; l_frame != NULL;l_frame = l_frame->next)
    {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) l_frame->data;
        for (NvDsMetaList * l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next) 
        {
            NvDsUserMeta *user_meta = (NvDsUserMeta *) l_user->data;
            // Proceed with the remaining steps
            if (user_meta->base_meta.meta_type != NVDSINFER_TENSOR_OUTPUT_META)
                continue;

            // 如果用户metadata的类型是tensor output，将其转换为NvDsInferTensorMeta类型   
            NvDsInferTensorMeta *meta = (NvDsInferTensorMeta *) user_meta->user_meta_data;
            
            // 对输出层的数据做处理
            std::vector < NvDsInferLayerInfo > outputLayersInfo (meta->output_layers_info, meta->output_layers_info + meta->num_output_layers);
            // 打印输出层的信息, 这里只打印了每个输出层的名称和维度
            for (const auto& layer : outputLayersInfo) {
                g_print("Output Layer: %s\n", layer.layerName);
                g_print("Dims: %d x %d x %d\n", layer.inferDims.d[0], layer.inferDims.d[1], layer.inferDims.d[2]);
            }
            
            // std::vector < NvDsInferNetworkInfo > objectList;
        }
    }

    return GST_PAD_PROBE_OK;
}
```

**GPU版本的**
```cpp
static GstPadProbeReturn pose_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    static guint use_device_mem = 0;
    g_print("pose_src_pad_buffer_probe called\n");

    // 获取GStreamer的buffer，并从中获取batch metadata
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (GST_BUFFER (info->data));

    // 遍历每一帧的metadata
    for (NvDsMetaList * l_frame = batch_meta->frame_meta_list; l_frame != NULL;l_frame = l_frame->next)
    {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) l_frame->data;
        for (NvDsMetaList * l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next) 
        {
            NvDsUserMeta *user_meta = (NvDsUserMeta *) l_user->data;
            // Proceed with the remaining steps
            if (user_meta->base_meta.meta_type != NVDSINFER_TENSOR_OUTPUT_META)
                continue;

            // 如果用户metadata的类型是tensor output，将其转换为NvDsInferTensorMeta类型   
            NvDsInferTensorMeta *meta = (NvDsInferTensorMeta *) user_meta->user_meta_data;

            // 获取模型的输入形状和输出层的信息， 检查用的
            for (unsigned int i = 0; i < meta->num_output_layers; i++) 
            {
                NvDsInferLayerInfo *info = &meta->output_layers_info[i];
                info->buffer = meta->out_buf_ptrs_host[i];
                if (use_device_mem && meta->out_buf_ptrs_dev[i]) {
                    cudaMemcpy (meta->out_buf_ptrs_host[i], meta->out_buf_ptrs_dev[i],
                    info->inferDims.numElements * 4, cudaMemcpyDeviceToHost);
                }
                // Print layer name and dimensions

                g_print("Output Layer: %s\n", info->layerName);
                g_print("Dims: %d x %d x %d\n", info->inferDims.d[0], info->inferDims.d[1], info->inferDims.d[2]);
            }
            
            // 对输出层的数据做处理
            std::vector < NvDsInferLayerInfo > outputLayersInfo (meta->output_layers_info, meta->output_layers_info + meta->num_output_layers);

        }
    }

    return GST_PAD_PROBE_OK;
}
```

**确保数据是连续内存的一个二维度的数组**
```cpp
static GstPadProbeReturn pose_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    static guint use_device_mem = 0;
    g_print("pose_src_pad_buffer_probe called\n");

    // 获取GStreamer的buffer，并从中获取batch metadata
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (GST_BUFFER (info->data));

    // 遍历每一帧的metadata
    for (NvDsMetaList * l_frame = batch_meta->frame_meta_list; l_frame != NULL;l_frame = l_frame->next)
    {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) l_frame->data;
        for (NvDsMetaList * l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next) 
        {
            NvDsUserMeta *user_meta = (NvDsUserMeta *) l_user->data;
            // Proceed with the remaining steps
            if (user_meta->base_meta.meta_type != NVDSINFER_TENSOR_OUTPUT_META)
                continue;

            // 如果用户metadata的类型是tensor output，将其转换为NvDsInferTensorMeta类型   
            NvDsInferTensorMeta *meta = (NvDsInferTensorMeta *) user_meta->user_meta_data;

            // 获取模型的输入形状和输出层的信息， 检查用的
            for (unsigned int i = 0; i < meta->num_output_layers; i++) 
            {
                NvDsInferLayerInfo *info = &meta->output_layers_info[i];
                info->buffer = meta->out_buf_ptrs_host[i];
                if (use_device_mem && meta->out_buf_ptrs_dev[i]) {
                    cudaMemcpy (meta->out_buf_ptrs_host[i], meta->out_buf_ptrs_dev[i],
                    info->inferDims.numElements * 4, cudaMemcpyDeviceToHost);
                }
                // Print layer name and dimensions

                g_print("Output Layer: %s\n", info->layerName);
                g_print("Dims: %d x %d x %d\n", info->inferDims.d[0], info->inferDims.d[1], info->inferDims.d[2]);
            }
            
            // 对输出层的数据做处理, 这里的outputLayersInfo是一个CPU上的数据
            std::vector < NvDsInferLayerInfo > outputLayersInfo (meta->output_layers_info, meta->output_layers_info + meta->num_output_layers);
            
            // 你的模型只有一个输出层，所以我们可以直接获取第0层
            NvDsInferLayerInfo* info = &meta->output_layers_info[0];

            // 将数据视为无符号字符（字节）的数组
            unsigned char* data = reinterpret_cast<unsigned char*>(info->buffer);

            // 计算数据的开始和结束地址
            unsigned char* data_start = &data[0];
            unsigned char* data_end = &data[56*8400*sizeof(float)]; // 注意，我们现在用的是字节地址，所以要乘以sizeof(float)

            std::cout << "Data start address: " << reinterpret_cast<void*>(data_start) << std::endl;
            std::cout << "Data end address: " << reinterpret_cast<void*>(data_end) << std::endl;

            // 计算预期的结束地址（假设内存是连续的）
            unsigned char* expected_end = data_start + 56*8400*sizeof(float);

            if (data_end == expected_end) {
                std::cout << "Memory is continuous." << std::endl;
            } else {
                std::cout << "Memory is not continuous." << std::endl;
            }


            // 遍历所有数据
            // for (int row = 0; row < 56; ++row) {
            //     for (int col = 0; col < 8400; ++col) {
            //         // 计算当前元素在内存中的位置
            //         int index = row * 8400 + col;
            //         // 获取当前元素
            //         float value = data[index];
            //         // 现在你可以对value进行你需要的操作了
            //     }
            // }
        }
    }

    return GST_PAD_PROBE_OK;
}
```

**通过这个可以看出来数据的格式是对的**
```cpp
static GstPadProbeReturn pose_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    static guint use_device_mem = 0;
    g_print("pose_src_pad_buffer_probe called\n");

    // 获取GStreamer的buffer，并从中获取batch metadata
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (GST_BUFFER (info->data));

    // 遍历每一帧的metadata
    for (NvDsMetaList * l_frame = batch_meta->frame_meta_list; l_frame != NULL;l_frame = l_frame->next)
    {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) l_frame->data;
        for (NvDsMetaList * l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next) 
        {
            NvDsUserMeta *user_meta = (NvDsUserMeta *) l_user->data;
            // Proceed with the remaining steps
            if (user_meta->base_meta.meta_type != NVDSINFER_TENSOR_OUTPUT_META)
                continue;

            // 如果用户metadata的类型是tensor output，将其转换为NvDsInferTensorMeta类型   
            NvDsInferTensorMeta *meta = (NvDsInferTensorMeta *) user_meta->user_meta_data;

            NvDsInferNetworkInfo network_info = meta->network_info;
            g_print("Model input shape: %d x %d x %d\n", network_info.channels, network_info.height, network_info.width);


            // 获取模型的输入形状和输出层的信息， 检查用的
            for (unsigned int i = 0; i < meta->num_output_layers; i++) 
            {
                NvDsInferLayerInfo *info = &meta->output_layers_info[i];
                info->buffer = meta->out_buf_ptrs_host[i];
                if (use_device_mem && meta->out_buf_ptrs_dev[i]) {
                    cudaMemcpy (meta->out_buf_ptrs_host[i], meta->out_buf_ptrs_dev[i],
                    info->inferDims.numElements * 4, cudaMemcpyDeviceToHost);
                }
                // Print layer name and dimensions

                g_print("Output Layer: %s\n", info->layerName);
                g_print("Dims: %d x %d \n", info->inferDims.d[0], info->inferDims.d[1]);
            }
            
            // 对输出层的数据做处理, 这里的outputLayersInfo是一个CPU上的数据
            std::vector < NvDsInferLayerInfo > outputLayersInfo (meta->output_layers_info, meta->output_layers_info + meta->num_output_layers);
            
            // 你的模型只有一个输出层，所以我们可以直接获取第0层
            NvDsInferLayerInfo* info = &meta->output_layers_info[0];

            // 将数据视为浮点数的数组
            float* data = static_cast<float*>(info->buffer);

            // 遍历所有数据
            for (int col = 0; col < 8400; ++col) { // 只打印前三个新组成的56的数据
                for (int row = 0; row < 56; ++row) {
                    // 计算当前元素在内存中的位置
                    // 现在我们是按照每个col来计算索引的
                    int index = row * 8400 + col;
                    // 获取当前元素
                    float value = data[index];
                    // 打印当前元素的值
                    std::cout << "Data at [" << row << "][" << col << "]: " << value << std::endl;
                }
                // 在每组之间打印一个空行，以便于区分
                std::cout << std::endl;
            }
        }
    }

    return GST_PAD_PROBE_OK;
}

```

## 5 开始对数据进行操作 CPU版本的

**这里就完成了iou和nms的操作**

```cpp
static GstPadProbeReturn pose_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    static guint use_device_mem = 0;
    g_print("pose_src_pad_buffer_probe called\n");

    // 获取GStreamer的buffer，并从中获取batch metadata
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (GST_BUFFER (info->data));

    // 遍历每一帧的metadata
    for (NvDsMetaList * l_frame = batch_meta->frame_meta_list; l_frame != NULL;l_frame = l_frame->next)
    {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) l_frame->data;
        for (NvDsMetaList * l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next) 
        {
            NvDsUserMeta *user_meta = (NvDsUserMeta *) l_user->data;
            // Proceed with the remaining steps
            if (user_meta->base_meta.meta_type != NVDSINFER_TENSOR_OUTPUT_META)
                continue;

            // 如果用户metadata的类型是tensor output，将其转换为NvDsInferTensorMeta类型   
            NvDsInferTensorMeta *meta = (NvDsInferTensorMeta *) user_meta->user_meta_data;

            NvDsInferNetworkInfo network_info = meta->network_info;
            g_print("Model input shape: %d x %d x %d\n", network_info.channels, network_info.height, network_info.width);


            // 获取模型的输入形状和输出层的信息， 检查用的
            for (unsigned int i = 0; i < meta->num_output_layers; i++) 
            {
                NvDsInferLayerInfo *info = &meta->output_layers_info[i];
                info->buffer = meta->out_buf_ptrs_host[i];
                if (use_device_mem && meta->out_buf_ptrs_dev[i]) {
                    cudaMemcpy (meta->out_buf_ptrs_host[i], meta->out_buf_ptrs_dev[i],
                    info->inferDims.numElements * 4, cudaMemcpyDeviceToHost);
                }
                // Print layer name and dimensions

                g_print("Output Layer: %s\n", info->layerName);
                g_print("Dims: %d x %d \n", info->inferDims.d[0], info->inferDims.d[1]);
            }
            
            // 对输出层的数据做处理, 这里的outputLayersInfo是一个CPU上的数据
            std::vector < NvDsInferLayerInfo > outputLayersInfo (meta->output_layers_info, meta->output_layers_info + meta->num_output_layers);
            
            // 你的模型只有一个输出层，所以我们可以直接获取第0层
            NvDsInferLayerInfo* info = &meta->output_layers_info[0];

            // 将数据视为浮点数的数组
            float* data = static_cast<float*>(info->buffer);

            // 创建一个Output对象的数组，用于存储所有的输出
            std::vector<Output> outputs(8400);

            // 遍历所有数据
            for (int col = 0; col < 8400; ++col) {
                Output& output = outputs[col]; // 获取当前的输出对象

                // 填充output的值
                for (int row = 0; row < 56; ++row) {
                    // 计算当前元素在内存中的位置
                    int index = row * 8400 + col;
                    // 获取当前元素
                    float value = data[index];

                    // 根据row的值将value分配给Output对象的不同成员
                    if (row < 4) {
                        output.xywh[row] = value;
                    } else if (row == 4) {
                        output.confidence = value;
                    } else {
                        output.kpts[row - 5] = value;
                    }
                }
            }

            // 过滤置信度低于0.25的Output
            std::vector<Output> confident_outputs;
            for (const Output& output : outputs) {
                if (output.confidence >= 0.25f) {
                    confident_outputs.push_back(output);
                }
            }

            // 对置信度较高的Output应用非极大值抑制 (NMS)
            std::vector<Output> filtered_outputs = applyNMS(confident_outputs, 0.65f);

            // 打印前三个过滤后的Output对象
            for (int i = 0; i < std::min(3, static_cast<int>(filtered_outputs.size())); ++i) {
                const Output& output = filtered_outputs[i];
                std::cout << "Filtered output " << i << ":\n";
                std::cout << "  xywh: ";
                for (int j = 0; j < 4; ++j) {
                    std::cout << output.xywh[j] << ' ';
                }
                std::cout << "\n  confidence: " << output.confidence << "\n";
                std::cout << "  kpts: ";
                for (int j = 0; j < 51; ++j) {
                    std::cout << output.kpts[j] << ' ';
                }
                std::cout << '\n';
            }  


        }
    }
    return GST_PAD_PROBE_OK;
}

``` 

**log版本**
```cpp
static GstPadProbeReturn pose_src_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    static guint use_device_mem = 0;
    g_print("pose_src_pad_buffer_probe called\n");

    // 获取GStreamer的buffer，并从中获取batch metadata
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (GST_BUFFER (info->data));

    // 遍历每一帧的metadata
    for (NvDsMetaList * l_frame = batch_meta->frame_meta_list; l_frame != NULL;l_frame = l_frame->next)
    {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) l_frame->data;
        for (NvDsMetaList * l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next) 
        {
            NvDsUserMeta *user_meta = (NvDsUserMeta *) l_user->data;
            // Proceed with the remaining steps
            if (user_meta->base_meta.meta_type != NVDSINFER_TENSOR_OUTPUT_META)
                continue;

            // 如果用户metadata的类型是tensor output，将其转换为NvDsInferTensorMeta类型   
            NvDsInferTensorMeta *meta = (NvDsInferTensorMeta *) user_meta->user_meta_data;

            NvDsInferNetworkInfo network_info = meta->network_info;
            g_print("Model input shape: %d x %d x %d\n", network_info.channels, network_info.height, network_info.width);


            // 获取模型的输入形状和输出层的信息， 检查用的
            for (unsigned int i = 0; i < meta->num_output_layers; i++) 
            {
                NvDsInferLayerInfo *info = &meta->output_layers_info[i];
                info->buffer = meta->out_buf_ptrs_host[i];
                if (use_device_mem && meta->out_buf_ptrs_dev[i]) {
                    cudaMemcpy (meta->out_buf_ptrs_host[i], meta->out_buf_ptrs_dev[i],
                    info->inferDims.numElements * 4, cudaMemcpyDeviceToHost);
                }
                // Print layer name and dimensions

                g_print("Output Layer: %s\n", info->layerName);
                g_print("Dims: %d x %d \n", info->inferDims.d[0], info->inferDims.d[1]);
            }
            
            // 对输出层的数据做处理, 这里的outputLayersInfo是一个CPU上的数据
            std::vector < NvDsInferLayerInfo > outputLayersInfo (meta->output_layers_info, meta->output_layers_info + meta->num_output_layers);
            
            // 你的模型只有一个输出层，所以我们可以直接获取第0层
            NvDsInferLayerInfo* info = &meta->output_layers_info[0];

            // 将数据视为浮点数的数组
            float* data = static_cast<float*>(info->buffer);

            // 创建一个Output对象的数组，用于存储所有的输出
            std::vector<Output> outputs(8400);

            // 遍历所有数据
            for (int col = 0; col < 8400; ++col) {
                Output& output = outputs[col]; // 获取当前的输出对象

                // 填充output的值
                for (int row = 0; row < 56; ++row) {
                    // 计算当前元素在内存中的位置
                    int index = row * 8400 + col;
                    // 获取当前元素
                    float value = data[index];

                    // 根据row的值将value分配给Output对象的不同成员
                    if (row < 4) {
                        output.xywh[row] = value;
                    } else if (row == 4) {
                        output.confidence = value;
                    } else {
                        output.kpts[row - 5] = value;
                    }
                }
            }

            // 过滤置信度低于0.25的Output
            std::vector<Output> confident_outputs;
            for (const Output& output : outputs) {
                if (output.confidence >= 0.25f) {
                    confident_outputs.push_back(output);
                }
            }

            // 对置信度较高的Output应用非极大值抑制 (NMS)
            std::vector<Output> filtered_outputs = applyNMS(confident_outputs, 0.65f);

            // log version
            std::ofstream log_file("output.log", std::ios_base::out | std::ios_base::app);

            // Print all filtered Outputs
            for (int i = 0; i < static_cast<int>(filtered_outputs.size()); ++i) {
                const Output& output = filtered_outputs[i];
                log_file << "Filtered output " << i << ":\n";
                log_file << "  xywh: ";
                for (int j = 0; j < 4; ++j) {
                    log_file << output.xywh[j] << ' ';
                }
                log_file << "\n  confidence: " << output.confidence << "\n";
                log_file << "  kpts: ";
                for (int j = 0; j < 51; ++j) {
                    log_file << output.kpts[j] << ' ';
                }
                log_file << '\n';
            }  
            log_file.close();
        }
    }
    return GST_PAD_PROBE_OK;
}
```

# 6. 全部改成只有osd_sink_pad_buffer_probe并且加载显示

**没有展示的回调函数**
```cpp
// osd_sink_pad_buffer_probe
static GstPadProbeReturn osd_sink_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info,
                                                   gpointer u_data)
{
    GstBuffer *buf = (GstBuffer *)info->data;

    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);

    for (NvDsMetaList *l_frame = batch_meta->frame_meta_list; l_frame != NULL;
         l_frame = l_frame->next)
    {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *)(l_frame->data);

        for (NvDsMetaList *l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next)
        {
            NvDsUserMeta *user_meta = (NvDsUserMeta *) l_user->data;

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

            std::vector<Output> confident_outputs;
            for (const Output& output : outputs)
            {
                if (output.confidence >= 0.25f)
                {
                    confident_outputs.push_back(output);
                }
            }

            std::vector<Output> filtered_outputs = applyNMS(confident_outputs, 0.65f);
        }
    }

    frame_number++;
    update_frame_counter();
    return GST_PAD_PROBE_OK;
}
```

**带显示版本的回调函数(NvDsObjectMeta)**

目前怀疑没有显示的原因是因为使用的是NvDsObjectMeta, 后面准备使用NvDsDisplayMeta查看

```cpp
static GstPadProbeReturn osd_sink_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
    GstBuffer *buf = (GstBuffer *)info->data;
    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
    // 检查batch_meta是否为NULL
    if (batch_meta == NULL) {
        std::cout << "batch_meta is NULL" << std::endl;
    } else {
        std::cout << "batch_meta is not NULL" << std::endl;
    }

    for (NvDsMetaList *l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next)
    {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *)(l_frame->data);
        if (frame_meta == NULL) {
            std::cout << "frame_meta is NULL" << std::endl;
        } else {
            std::cout << "frame_meta is not NULL" << std::endl;
        }

        for (NvDsMetaList *l_user = frame_meta->frame_user_meta_list; l_user != NULL; l_user = l_user->next)
        {
            NvDsUserMeta *user_meta = (NvDsUserMeta *) l_user->data;
            if (user_meta == NULL) {
                std::cout << "user_meta is NULL" << std::endl;
            } else {
                std::cout << "user_meta is not NULL" << std::endl;
            }

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

            std::vector<Output> confident_outputs;
            for (const Output& output : outputs)
            {
                if (output.confidence >= 0.25f)
                {
                    confident_outputs.push_back(output);
                }
            }

            std::vector<Output> filtered_outputs = applyNMS(confident_outputs, 0.65f);
            for (const Output& output : filtered_outputs) {
                std::cout << "Output Information:\n";
                std::cout << "X: " << output.xywh[0] << "\n";
                std::cout << "Y: " << output.xywh[1] << "\n";
                std::cout << "Width: " << output.xywh[2] << "\n";
                std::cout << "Height: " << output.xywh[3] << "\n";
                NvOSD_RectParams rect_params;
                rect_params.left = static_cast<unsigned int>(output.xywh[0]);
                rect_params.top = static_cast<unsigned int>(output.xywh[1]);
                rect_params.width = static_cast<unsigned int>(output.xywh[2]);
                rect_params.height = static_cast<unsigned int>(output.xywh[3]);
                rect_params.border_color.red = 1.0;
                rect_params.border_color.green = 0.0;
                rect_params.border_color.blue = 0.0;
                rect_params.border_width = 5;
                rect_params.has_bg_color = 0;

                NvDsObjectMeta *obj_meta = nvds_acquire_obj_meta_from_pool(batch_meta);
                if(obj_meta == NULL) {
                    std::cout << "Failed to acquire metadata object from pool." << std::endl;
                } else {
                    std::cout << "Successfully acquired metadata object from pool." << std::endl;
                }

                obj_meta->rect_params = rect_params;
                nvds_add_obj_meta_to_frame(frame_meta, obj_meta, NULL);
                // Print out some frame meta information
                std::cout << "Frame number: " << frame_meta->frame_num << std::endl;
                std::cout << "Source id: " << frame_meta->source_id << std::endl;
                std::cout << "Object meta list length: " << g_list_length(frame_meta->obj_meta_list) << std::endl;
            }
        }
    }

    // frame_number++;
    // update_frame_counter();
    return GST_PAD_PROBE_OK;
}
```


**NvDsDisplayMeta版本的代码, 不贴合，下一步做归一化**
```cpp
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

            // GPU上的NMS
            // std::vector<Output> confident_outputs = getConfidentOutputs();
            // apply_nms(confident_outputs.data(), confident_outputs.size(), 0.65f);

            // 绘制
            for (const Output& output : filtered_outputs) 
            {
                // std::cout << "Output Information:\n";
                // std::cout << "X: " << output.xywh[0] << "\n";
                // std::cout << "Y: " << output.xywh[1] << "\n";
                // std::cout << "Width: " << output.xywh[2] << "\n";
                // std::cout << "Height: " << output.xywh[3] << "\n";

                NvDsDisplayMeta *display_meta = nvds_acquire_display_meta_from_pool(batch_meta);
                display_meta->num_rects = 1;
                NvOSD_RectParams *rect_params = &display_meta->rect_params[0];
                
                // if the xy is the top left corner
                // rect_params->left = static_cast<unsigned int>(output.xywh[0]);
                // rect_params->top = static_cast<unsigned int>(output.xywh[1]);

                // xy is the center 
                rect_params->left = static_cast<unsigned int>(output.xywh[0] - output.xywh[2] / 2.0);
                rect_params->top = static_cast<unsigned int>(output.xywh[1] - output.xywh[3] / 2.0);
                rect_params->width = static_cast<unsigned int>(output.xywh[2]);
                rect_params->height = static_cast<unsigned int>(output.xywh[3]);
                rect_params->border_width = 2;
                rect_params->border_color.red = 1.0;
                rect_params->border_color.green = 0.0;
                rect_params->border_color.blue = 0.0;
                rect_params->border_color.alpha = 1.0;

                nvds_add_display_meta_to_frame(frame_meta, display_meta);
                
                // Print out some frame meta information
                // std::cout << "Frame number: " << frame_meta->frame_num << std::endl;
                // std::cout << "Source id: " << frame_meta->source_id << std::endl;
            }
        }
    }

    frame_number++;
    update_frame_counter();
    return GST_PAD_PROBE_OK;
}
```


# 8. osd回调函数
```cpp
static GstPadProbeReturn osd_sink_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info,
                                                   gpointer u_data)
{
    GstBuffer *buf = (GstBuffer *)info->data;
    if (buf == NULL) {
        g_printerr("Buffer is NULL\n");
        return GST_PAD_PROBE_OK;
    }

    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);
    if (batch_meta == NULL) {
        g_printerr("Batch meta is NULL\n");
        return GST_PAD_PROBE_OK;
    }

    for (NvDsMetaList *l_frame = batch_meta->frame_meta_list; l_frame != NULL; l_frame = l_frame->next)
    {
        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *)(l_frame->data);
        if (frame_meta == NULL) {
            g_printerr("frame_meta is NULL\n");
            continue;
        }

        for (NvDsMetaList *l_obj = frame_meta->obj_meta_list; l_obj != NULL; l_obj = l_obj->next)
        {
            NvDsObjectMeta *obj_meta = (NvDsObjectMeta *)(l_obj->data);
            if (obj_meta == NULL) {
                g_printerr("obj_meta is NULL, skipping this object\n");
                continue;
            }

            // Update rectangle parameters for the bounding box
            obj_meta->rect_params.left = obj_meta->rect_params.left;
            obj_meta->rect_params.top = obj_meta->rect_params.top;
            obj_meta->rect_params.width = obj_meta->rect_params.width;
            obj_meta->rect_params.height = obj_meta->rect_params.height;
            obj_meta->rect_params.border_width = 2;
            obj_meta->rect_params.border_color = (NvOSD_ColorParams){ 1.0, 0.0, 0.0, 1.0 };  // Red color for the border

            // Update text parameters for the tracker ID
            obj_meta->text_params.display_text = g_strdup_printf("Student %d", obj_meta->object_id);  // Assuming object_id as the student ID
            obj_meta->text_params.x_offset = obj_meta->rect_params.left;
            obj_meta->text_params.y_offset = MAX(0, (int)obj_meta->rect_params.top - 20);  // Adjust according to your requirements
            obj_meta->text_params.font_params.font_name = "Serif";
            obj_meta->text_params.font_params.font_size = 12;
            obj_meta->text_params.font_params.font_color = (NvOSD_ColorParams){ 1.0, 1.0, 1.0, 1.0 };  // White color for the text
            obj_meta->text_params.set_bg_clr = 1;
            obj_meta->text_params.text_bg_clr = (NvOSD_ColorParams){ 0.0, 0.0, 0.0, 1.0 };  // Black color for the background
        }
    }
    frame_number++;
    update_frame_counter();
    return GST_PAD_PROBE_OK;
}
```

