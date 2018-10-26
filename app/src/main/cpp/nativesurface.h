// 全局头文件
// Created by jiangdongguo on 2018/8/7.
//

#ifndef FFMPEG4ANDROID_NATIVESURFACE_H
#define FFMPEG4ANDROID_NATIVESURFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include "ffmpeg.h"
#include "libavdevice/avdevice.h"
#include <android/bitmap.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <libavutil/imgutils.h>

#define TAG "decstream"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

// Packet结构体
typedef struct PacketQueue {
    AVPacketList *first_pkg,*last_pkg;
    int nb_packet;
    int size;
    int abort_request;
    int serial;
    pthread_mutex_t *mutex;
}PacketQueue;

// 全局变量结构体
typedef struct GlobalContexts {
    AVCodecContext *vcodec_ctx;
    AVStream *vstream;
    AVStream *tstream;
    AVCodec *vcodec;
    PacketQueue video_queue;

    int quit;
    int pause;
} GlobalContext;

// packet操作函数
void packet_queue_init(PacketQueue *q);
int packet_queue_get(PacketQueue *q);
int packet_queue_put(PacketQueue *q,AVPacket *pkt);

// 子线程函数
void *open_stream_thread(void *argv);
void *video_decode_thread(void *argv);

// 原生绘制相关函数
int32_t setBufferGeometry(int32_t width,int32_t height);
void renderSurface(uint8_t *pixel);

extern GlobalContext global_context;

#ifdef __cplusplus
}
#endif

#endif //FFMPEG4ANDROID_NATIVESURFACE_H
