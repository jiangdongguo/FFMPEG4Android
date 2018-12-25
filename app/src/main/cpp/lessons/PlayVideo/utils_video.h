// RGB链表
// Created by jianddongguo on 2018/12/5.
//
// 防止重复定义
#ifndef FFMPEG4ANDROID_UTILS_VIDEO_H
#define FFMPEG4ANDROID_UTILS_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>
#include "android/log.h"
#include <malloc.h>
#define TAG "PlayVideo"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,"%s",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,"%s",__VA_ARGS__)
#define LOG_E(format, ...) __android_log_print(ANDROID_LOG_ERROR,TAG,format,__VA_ARGS__)

struct RGBPacket {
    uint8_t *rgb;
    int size;
};

struct RGBPacketNode {
    RGBPacket pkt;
    struct RGBPacketNode *next;
};

struct RGBPacketQueue {
    RGBPacketNode *first_pkt, *last_pkt;
    int queueSize;
    int nb_packets;
    pthread_mutex_t mutex;    // 互斥锁
};

void queue_rgb_init(RGBPacketQueue *rgbQueue);
int queue_rgb_put(RGBPacketQueue *rgbQueue, RGBPacket *rgbPkt);
int queue_rgb_get(RGBPacketQueue *rgbQueue, RGBPacket *rgbPkt);
#ifdef __cplusplus
}
#endif

#endif //FFMPEG4ANDROID_UTILS_VIDEO_H