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
struct PCMPacket2 {
    char *pcm;
    int size;
};

// PCM数据链表节点
struct PCMPacketNode2 {
    PCMPacket2 pkt;
    struct PCMPacketNode2 *next;
};

// PCM数据链表
struct PCMPacketQueue2 {
    PCMPacketNode2 *first_pkt, *last_pkt;
    int queueSize;
    int nb_packets;
    pthread_mutex_t mutex;    // 互斥锁
};

void queue_pcm_init2(PCMPacketQueue2 *pcmQueue);
int queue_pcm_put2(PCMPacketQueue2 *pcmQueue, PCMPacket2 *pcmPkt);
int queue_pcm_get2(PCMPacketQueue2 *pcmQueue, PCMPacket2 *pcmPkt);

struct RGBPacket2 {
    uint8_t *rgb;
    int size;
};

struct RGBPacketNode2 {
    RGBPacket2 pkt;
    struct RGBPacketNode2 *next;
};

struct RGBPacketQueue2 {
    RGBPacketNode2 *first_pkt, *last_pkt;
    int queueSize;
    int nb_packets;
    pthread_mutex_t mutex;    // 互斥锁
};

void queue_rgb_init2(RGBPacketQueue2 *rgbQueue);
int queue_rgb_put2(RGBPacketQueue2 *rgbQueue, RGBPacket2 *rgbPkt);
int queue_rgb_get2(RGBPacketQueue2 *rgbQueue, RGBPacket2 *rgbPkt);

#ifdef __cplusplus
}
#endif

#endif //FFMPEG4ANDROID_UTILS_AUDIO_H