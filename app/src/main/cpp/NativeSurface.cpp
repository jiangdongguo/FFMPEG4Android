#include "NativeSurface.h"
#include "lessons/SaveStream.h"

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 28, 1)
#define av_frame_alloc  avcodec_alloc_frame
#endif
bool stop;
//    char input_str[500] = {0};
//    sprintf(input_str, "%s", url);

extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_openVideo(JNIEnv *env, jclass type, jstring url_,
                                                jobject surface, jobject listener) {

    stop = false;
    int videoIndex = -1;
    int ret = -1, videoWidth = 0, videoHeight = 0;
    const char *inputUrl = env->GetStringUTFChars(url_, 0);
    AVFrame *pAvFrame = av_frame_alloc();
    AVFrame *pRGBAFrame = av_frame_alloc();
    AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket));

    jclass jcls = env->GetObjectClass(listener);
    jmethodID jID = env->GetMethodID(jcls, "onStreamAcquire", "([BII)V");

    //-----------------------------------------------------------------
    //---------------1.初始化FFmpeg引擎/打开输入文件inputUrl---------------
    //-----------------------------------------------------------------

    //(1) 初始化FFmpeg引擎
    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    avdevice_register_all();
    //(2) 创建与输入文件对应的inputContext
    // 并保存打开输入文件获取到的数据
    AVFormatContext *inputContext = avformat_alloc_context();
    ret = avformat_open_input(&inputContext, inputUrl, NULL, NULL);
    if (ret < 0) {
        return ret;
    }
    //(3) 获取输入文件的流信息和确定音频轨道序号
    ret = avformat_find_stream_info(inputContext, NULL);
    if (ret < 0) {
        return ret;
    }
    unsigned int nb_streams = inputContext->nb_streams;
    for (unsigned int i = 0; i < nb_streams; i++) {
        if (inputContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            break;
        }
    }
    if (videoIndex == -1) {
        LOGE("find video index failed");
        return -1;
    }
    //(4) 匹配解码器，如果成功则使用给定的avCodec初始化AVCodecContext
    AVCodecContext *pCodecCtx = inputContext->streams[videoIndex]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        LOGE("find matched decoder failed by called avcodec_find_decoder()");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) != 0) {
        LOGE("open codec failed by called avcodec_open2()");
        return -1;
    }
    videoWidth = pCodecCtx->width;
    videoHeight = pCodecCtx->height;

    //-----------------------------------------------------------------
    //------------------------2.初始化ANativeWindow----------------------
    //-----------------------------------------------------------------

    ANativeWindow_Buffer windowBuffer;

    //(1) 获取Surface对象相对于原生Window的引用
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (0 == nativeWindow) {
        LOGE("初始化Native Window失败");
        return -1;
    }

    //(2) 设置原生Window缓冲区的几何形状
    //    包括大小、像素格式(通常与Surfaceview初始化格式一致)
    ret = ANativeWindow_setBuffersGeometry(nativeWindow, videoWidth, videoHeight, WINDOW_FORMAT_RGBA_8888);
    if(ret < 0) {
        ANativeWindow_release(nativeWindow);
        return ret;
    }

    //-----------------------------------------------------------------
    //---------------------3.逐帧解码，YUV转RGB后渲染---------------------
    //-----------------------------------------------------------------

    //(1) 初始化像素格式转换上下文
    AVPixelFormat srcFormat = pCodecCtx->pix_fmt;
    AVPixelFormat dstFormat = AV_PIX_FMT_RGBA;
    SwsContext *pImgCovertCtx = sws_getContext(
            videoWidth,   // 原始宽度
            videoHeight,  // 原始高度
            srcFormat,    // 原始格式
            videoWidth,   // 目标宽度
            pCodecCtx->height,   // 目标高度
            dstFormat,    // 目标格式，RGB
            SWS_BICUBIC,  // 改变尺寸方式
            NULL,
            NULL,
            NULL);

    // (2) 计算存储一帧AV_PIX_FMT_RGBA格式图片所需空间(字节)
    //     分配内存,设置绑定数据源
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, videoWidth, videoHeight, 1);
    uint8_t *v_out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

    av_image_fill_arrays(pRGBAFrame->data,    // 要填充的dst_data指针
                         pRGBAFrame->linesize,// 要填充的图像行大小
                         v_out_buffer,   // 数据源，将在解码后被填充
                         AV_PIX_FMT_RGBA,// 图像像素格式
                         videoWidth,     // 图像宽度
                         videoHeight,    // 图像高度
                         1);

    //(2) 逐帧解码，并将解码得到的YUV转换为RGB进行渲染
    while (! stop)
    {
        if (av_read_frame(inputContext, pPacket) >= 0) {

            LOGI("---------->获取packet中的数据(H.264)，返回给Java层");

            if ((pPacket)->stream_index != videoIndex) {
                continue;
            }
            uint8_t *data = pPacket->data;
            int len = pPacket->size;
            jbyteArray h264Arr = env->NewByteArray(len);
            if (h264Arr != NULL) {
                env->SetByteArrayRegion(h264Arr, 0, len, (jbyte *) data);
                env->CallVoidMethod(listener, jID, h264Arr, len, 1);
            }

            LOGI("---------->解码H.264，存储到pAvFrame...");

            int frameFinished = 0;
            int code = avcodec_decode_video2(pCodecCtx, pAvFrame, &frameFinished, pPacket);

            if (code > 0 && frameFinished != 0) {
                LOGI("---------->将YUV转换为RGB");
                sws_scale(
                        pImgCovertCtx,                           // 格式转换上下文
                        (const uint8_t *const *) pAvFrame->data, // src image数据
                        pAvFrame->linesize,     // src image行大小
                        0,
                        pCodecCtx->height,
                        pRGBAFrame->data,       // dst image
                        pRGBAFrame->linesize);  // dst image行大小

                // 锁定窗口
                // Window的stride核帧的stride不一致
                // 因此需要逐行复制
                LOGI("---------->渲染一帧数据，像素格式为RGBA");
                if (ANativeWindow_lock(nativeWindow, &windowBuffer, NULL) >= 0) {
                    // 获取到屏幕的实际位数
                    uint8_t *dst = (uint8_t *) windowBuffer.bits;

                    // Window缓冲区中一行占用内存的像素数，1像素占4字节
                    int dstStride = windowBuffer.stride * 4;
                    int srcStride = pRGBAFrame->linesize[0];

                    for (int h = 0; h < videoHeight; h++) {
                        memcpy(dst + h * dstStride,
                               v_out_buffer + h * srcStride,
                               srcStride);
                    }
                    // 解锁先前锁定的窗口，将新的缓存区内容显示在界面上
                    ANativeWindow_unlockAndPost(nativeWindow);
                    LOGI("---------->完成渲染到屏幕");
                }
            }
            // 是否数组引用,否则会报“local reference table overflow (max=512)”
            env->DeleteLocalRef(h264Arr);
        }
        // Wipe the packet
        av_packet_unref(pPacket);
    }

    //-----------------------------------------------------------------
    //-----------------------------4.释放资源----------------------------
    //-----------------------------------------------------------------

    env->ReleaseStringUTFChars(url_, inputUrl);
    sws_freeContext(pImgCovertCtx);
    av_free(pPacket);
    av_free(pRGBAFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&inputContext);
    return videoIndex;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_stopVideo(JNIEnv *env, jclass type) {
    stop = true;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_init(JNIEnv *env, jclass type) {
    stop = false;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_release(JNIEnv *env, jclass type) {
    stop = true;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_saveStreamFile(JNIEnv *env, jclass type_, jstring inputUrl_,
                                                     jint type, jstring outputPath_) {
    const char *inputUrl = env->GetStringUTFChars(inputUrl_, 0);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);
    initFFmpeg();

    int ret = openInput(inputUrl);
    if (ret >= 0) {
        ret = openOutput(outputPath);
        if (ret >= 0) {
            while (!stop) {
                AVPacket *avPacket = readPacketFromSource();
                if (avPacket) {
                    ret = writePacket(avPacket);
                    if (ret >= 0) {
                        LOGI("writePacket Success!");
                    } else {
                        LOGI("writePacket Failed.");
                    }
                }
            }
        } else {
            LOGE("open output file failed.");
        }
    } else {
        LOGE("open input url failed.");
    }

    releaseFFmpeg();
    env->ReleaseStringUTFChars(inputUrl_, inputUrl);
    env->ReleaseStringUTFChars(outputPath_, outputPath);
    return ret;
}