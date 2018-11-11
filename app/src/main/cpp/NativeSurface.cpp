#include "NativeSurface.h"
#include "lessons/SaveStream.h"

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 28, 1)
#define av_frame_alloc  avcodec_alloc_frame
#endif
bool stop;
//    char input_str[500] = {0};
//    sprintf(input_str, "%s", url);
const int sampleRates[16] = {96000, 88200,64000, 48000,44100,32000,
                       24000,22050,16000,12000,11025,8000,7350,
                       -1,-1,-1};

int getSampleRateIndex(int sRate) {
    int i,index = 0;
    for(i=0; i<16; i++) {
        if(sRate == sampleRates[i]) {
            index = i;
            break;
        }
    }
    return index;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_openVideo(JNIEnv *env, jclass type, jstring url_,
                                                jobject surface, jobject listener) {

    stop = false;
    int videoIndex = -1,audioIndex = -1;
    int ret = -1, videoWidth = 0, videoHeight = 0;
    const char *inputUrl = env->GetStringUTFChars(url_, 0);
    AVFrame *pAvFrame = av_frame_alloc();
    AVFrame *pRGBAFrame = av_frame_alloc();
    AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket));

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
        if (inputContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO ) {
            if(videoIndex != -1) {
                continue;
            }
            videoIndex = i;
        } else if(inputContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            if(audioIndex != -1) {
                continue;
            }
            audioIndex = i;
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
    jclass jcls = env->GetObjectClass(listener);
    jmethodID jID = env->GetMethodID(jcls, "onStreamAcquire", "([BIJI)V");
    char *pADTS = (char *)malloc(sizeof(char) * 7);
    int chanCfg = inputContext->streams[audioIndex]->codec->channels;
    int profile = inputContext->streams[audioIndex]->codec->profile;

    int freqIdx = getSampleRateIndex(inputContext->streams[audioIndex]->codec->sample_rate);
    LOG_I("channels = %d,profile = %d,sampleRate = %d",chanCfg,profile,freqIdx);

    while (! stop)
    {
        if (av_read_frame(inputContext, pPacket) >= 0) {

            LOGI("---------->获取packet中的数据(H.264)，返回给Java层");

            uint8_t *data = pPacket->data;
            int len = pPacket->size;

            if(videoIndex == pPacket-> stream_index) {
                jbyteArray encodeData = env->NewByteArray(len);
                if (encodeData != NULL) {
                    env->SetByteArrayRegion(encodeData, 0, len, (jbyte *) data);
                    env->CallVoidMethod(listener, jID, encodeData, len, pPacket->pts, 1);
                }
                env->DeleteLocalRef(encodeData);
            } else if(audioIndex == pPacket->stream_index){
                int newLength = len + 7;
                pADTS[0] = (char)0xFF;
                pADTS[1] = (char)0xF1;
                pADTS[2] = (char)(((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
                pADTS[3] = (char) (((chanCfg & 3) << 6) + (newLength >> 11));
                pADTS[4] = (char) ((newLength & 0x7FF) >> 3);
                pADTS[5] = (char) (((newLength & 7) << 5) + 0x1F);
                pADTS[6] = (char)0xFC;
                char *newData = (char *)malloc(sizeof(char) * newLength);
                memset(newData,0,newLength);
                // ADTS头部
                memcpy(newData,pADTS,7);
                // AAC ES流
                memcpy(newData+7,data,len);

//                LOG_I("----------------->pADTS0=%x",pADTS[0]);
//                LOG_I("----------------->pADTS1=%x",pADTS[1]);
//                LOG_I("----------------->pADTS2=%x",pADTS[2]);
//                LOG_I("----------------->pADTS3=%x",pADTS[3]);
//                LOG_I("----------------->pADTS4=%x",pADTS[4]);
//                LOG_I("----------------->pADTS5=%x",pADTS[5]);
//                LOG_I("----------------->pADTS6=%x",pADTS[6]);
                jbyteArray encodeData = env->NewByteArray(newLength);
                if (encodeData != NULL) {
                    env->SetByteArrayRegion(encodeData, 0, newLength, (jbyte *) newData);
                    env->CallVoidMethod(listener, jID, encodeData, newLength, pPacket->pts, 0);
                }
                env->DeleteLocalRef(encodeData);

                free(newData);
            }


            LOGI("---------->解码H.264，存储到pAvFrame...");
            if (videoIndex != pPacket->stream_index) {
                continue;
            }
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
        }
        // Wipe the packet
        av_packet_unref(pPacket);
    }

    free(pADTS);
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

extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativeInit(JNIEnv *env, jobject instance) {




}extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativeRelease(JNIEnv *env, jobject instance) {




}extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_openInputURL(JNIEnv *env, jobject instance,
                                                   jstring inputUrl_) {
    const char *inputUrl = env->GetStringUTFChars(inputUrl_, 0);




    env->ReleaseStringUTFChars(inputUrl_, inputUrl);
}extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_saveStream(JNIEnv *env, jobject instance, jint type,
                                                 jstring outputPath_) {
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);




    env->ReleaseStringUTFChars(outputPath_, outputPath);
}extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativePausePlayer(JNIEnv *env, jobject instance) {




}extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativeResumePlayer(JNIEnv *env, jobject instance) {




}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativeStopPlayer(JNIEnv *env, jobject instance) {




}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_setSurface(JNIEnv *env, jobject instance, jobject surface) {


}