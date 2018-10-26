#include "nativesurface.h"

bool stop;

ANativeWindow *mANativeWindow;

extern "C"
JNIEXPORT jint JNICALL
Java_com_teligen_ffmpeg_FfmpegUtils_openVideo(JNIEnv *env, jclass type, jstring url_,
                                              jobject surface,jobject listener) {
    // 控制标志
    // 获取接口的引用
    jclass jcls = env->GetObjectClass(listener);
    jmethodID jID = env->GetMethodID(jcls,"onStreamAcquire","([BII)V");

    stop = false;
    const char *url = env->GetStringUTFChars(url_, 0);
    LOGI("----初始化窗口渲染引擎------");
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 28, 1)
#define av_frame_alloc  avcodec_alloc_frame
#endif
    AVFrame *pAvFrame = av_frame_alloc();

    char input_str[500] = {0};
    sprintf(input_str, "%s", url);
    // 初始化低层窗口渲染引擎
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (0 == nativeWindow) {
        LOGE("初始化Native Window失败");
        return -1;
    }

    //-----------------------初始化ffmpeg引擎---------------------
    LOGI("----初始化ffmpeg引擎------");
    avcodec_register_all();
    // 注册库中所有可用的文件格式和编码器
    av_register_all();
    avformat_network_init();
    avdevice_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //-----------------------打开流多(流)媒体文件---------------------
    LOGI("----打开流多(流)媒体文件------");
    // 打开输入流并读取封装格式信息，存储到pFormatCtx
    if (avformat_open_input(&pFormatCtx, input_str, NULL, NULL) < 0) {
        return -1;
    }
    avformat_find_stream_info(pFormatCtx, NULL);
    // 视频轨道
    int videoIndex = -1;

    unsigned int nb_streams = pFormatCtx->nb_streams;
    for (unsigned int i = 0; i < nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            break;
        }
    }

    if (videoIndex == -1) {
        LOGE("获取视频轨道失败");
        return -1;
    }
    // 获取视频流编解码器
    AVCodecContext *pCodecCtx = pFormatCtx->streams[videoIndex]->codec;
    // 解码器对应的结构体
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec == NULL) {
        LOGE("无法找到匹配的解码器");
        return -1;
    }
    // 启动解码器
    if(avcodec_open2(pCodecCtx, pCodec, NULL) != 0) {
        LOGE("无法找到匹配的解码器");
        return -1;
    }
    // 将解码出来的帧格式，转码为RGBA
    SwsContext *pImgCovertCtx = sws_getContext(
            pCodecCtx->width,   // 原始宽度
            pCodecCtx->height,  // 原始高度
            pCodecCtx->pix_fmt, // 原始格式
            pCodecCtx->width,   // 目标宽度
            pCodecCtx->height,   // 目标高度
            AV_PIX_FMT_RGBA,    // 目标格式
            SWS_BICUBIC,        // 改变尺寸方式
            NULL,
            NULL,
            NULL);

    ANativeWindow_Buffer windowBuffer;
    // 获取视频帧的宽高
    int width = pCodecCtx->width;
    int height = pCodecCtx->height;
    // 返回存储widthxheight大小AV_PIX_FMT_RGBA格式图片所需的数据量的字节数
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, width, height, 1);
    // 申请分配一段内存
    uint8_t *v_out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

    AVFrame *pFrameRGBA = av_frame_alloc();
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, v_out_buffer, AV_PIX_FMT_RGBA, width,
                         height, 1);
    // 改变窗口缓存区的格式和大小
    // width、height 用于控制缓存区的像素数量
    // format 缓存区格式，通常与Surfaceview初始化格式一致
    if (0 > ANativeWindow_setBuffersGeometry(nativeWindow, width, height, WINDOW_FORMAT_RGBA_8888)) {
        ANativeWindow_release(nativeWindow);
        return -1;
    }

    //-----------------------读取数据包---------------------
    LOGI("----开始读取数据包------");
    // 存储一帧压缩编码数据
    AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    LOGI("----为AVPacket分配内存------");
    int count = 0;

    // 循环读取/解码视频帧
    while (! stop) {
        LOGI("----读取一帧存储到pPacket------");
        if (av_read_frame(pFormatCtx, pPacket) >= 0) {
            // 非视频包，不解码
            if ((pPacket)->stream_index != videoIndex) {
                continue;
            }
            uint8_t * data = pPacket->data;
            int len = pPacket->size;
            jbyteArray h264Arr = env->NewByteArray(len);
            if(h264Arr != NULL) {
                env->SetByteArrayRegion(h264Arr,0,len,(jbyte *)data);
                env->CallVoidMethod(listener,jID,h264Arr,len,1);
            }

            // 开始解码
            int frameFinished = 0;
            LOGI("----解码pPacket------");
            int code = avcodec_decode_video2(pCodecCtx, pAvFrame, &frameFinished, pPacket);
            if (code >0 && frameFinished != 0) {
                count++;
                // yuv转换为rgb
                LOGI("---------->获取解码数据，将YUV转换为RGB");
                sws_scale(
                        pImgCovertCtx,
                        (const uint8_t *const *) pAvFrame->data,
                        pAvFrame->linesize,     // 数据块大小
                        0,
                        pCodecCtx->height,
                        pFrameRGBA->data,
                        pFrameRGBA->linesize);

                // 锁定窗口
                // Window的stride核帧的stride不一致
                // 因此需要逐行复制
                LOGI("---------->开始渲染一帧");
                if (ANativeWindow_lock(nativeWindow, &windowBuffer, NULL) >= 0) {
                    // 获取到屏幕的实际位数
                    uint8_t *dst = (uint8_t *) windowBuffer.bits;

                    // Window缓冲区中一行占用内存的像素数，1像素占4字节
                    int dstStride = windowBuffer.stride * 4;
                    int srcStride = pFrameRGBA->linesize[0];

                    for (int h = 0; h < height; h++) {
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
        av_packet_unref(pPacket);
    }
    LOGI("----释放所有资源------");
    env->ReleaseStringUTFChars(url_, url);
    // 释放与ffmpeg相关的资源
    sws_freeContext(pImgCovertCtx);
    av_free(pPacket);
    av_free(pFrameRGBA);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    return videoIndex;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_teligen_ffmpeg_FfmpegUtils_stopVideo(JNIEnv *env, jclass type) {
    stop = true;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_teligen_ffmpeg_VideoSurface_nativeSetSurface(JNIEnv *env, jobject instance,
                                                      jobject surface) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_teligen_ffmpeg_VideoSurface_nativePauseSurface(JNIEnv *env, jobject instance) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_teligen_ffmpeg_VideoSurface_nativeResumeSurface(JNIEnv *env, jobject instance) {


}

extern "C"
JNIEXPORT void JNICALL
Java_com_teligen_ffmpeg_VideoSurface_nativeStopSurface(JNIEnv *env, jobject instance) {


}

extern "C"
JNIEXPORT void JNICALL
Java_com_teligen_ffmpeg_VideoSurface_setOnFrameListener(JNIEnv *env, jobject instance,
                                                        jobject listener) {


}

int32_t setBufferGeometry(int32_t width,int32_t height){

}

void renderSurface(uint8_t *pixel){

}