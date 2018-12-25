// PCM数据链表
// Created by jianddongguo on 2018/12/5.
//
// 防止重复定义
#ifndef FFMPEG4ANDROID_UTILS_AUDIO_H
#define FFMPEG4ANDROID_UTILS_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>
#include "android/log.h"
#include <malloc.h>
#define TAG "PlayAudio"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,"%s",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,"%s",__VA_ARGS__)
#define LOG_E(format, ...) __android_log_print(ANDROID_LOG_ERROR,TAG,format,__VA_ARGS__)

// PCM数据包结构体
struct PCMPacket {
    char *pcm;
    int size;
};

// PCM数据链表节点
struct PCMPacketNode {
    PCMPacket pkt;
    struct PCMPacketNode *next;
};

// PCM数据链表
struct PCMPacketQueue {
    PCMPacketNode *first_pkt, *last_pkt;
    int queueSize;
    int nb_packets;
    pthread_mutex_t mutex;    // 互斥锁
};

void queue_pcm_init(PCMPacketQueue *pcmQueue);
int queue_pcm_put(PCMPacketQueue *pcmQueue, PCMPacket *pcmPkt);
int queue_pcm_get(PCMPacketQueue *pcmQueue, PCMPacket *pcmPkt);
#ifdef __cplusplus
}
#endif

#endif //FFMPEG4ANDROID_UTILS_AUDIO_H