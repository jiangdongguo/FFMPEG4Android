// 全局头文件
// Created by jiangdongguo on 2018/11/5.
//

#ifndef FFMPEG4ANDROID_GLOBAL_H
#define FFMPEG4ANDROID_GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif
#include "ffmpeg.h"
#include <android/log.h>

// PacketQueue结构体
typedef struct PacketQueue {
    AVPacketList *first_pkt,*last_pkt;
    int nb_packets;         // 缓存中Packet的数量
    int size;               // 队列大小
    pthread_mutex_t *mutex; // 互斥锁
} PacketQueue;

// GlobalContext结构体
typedef struct GlobalContext {
    int quit;   // quit=1 结束
    int pause;  // quit=1 暂停
} GlobalContext;


GlobalContext mGlobalCtx;

// 全局函数
void ffmpeg_init();
void ffmpeg_deinit();
void packet_queue_init(PacketQueue *queue);
int packet_queue_put(PacketQueue *q,AVPacket *avPacket);
int packet_queue_get(PacketQueue *q,AVPacket *avPacket);

extern GlobalContext mGlobalCtx;
#ifdef __cplusplus
}
#endif

#define TAG "FFmpeg4Android"
#define LOG_I(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

#endif //FFMPEG4ANDROID_GLOBAL_H
