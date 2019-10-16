// 链表
// Created by Jiangdg on 2019/9/25.
//

#ifndef FFMPEG4ANDROID_UTILS_RENDER_H
#define FFMPEG4ANDROID_UTILS_RENDER_H

#include "native_demuxer.h"
#include "pthread.h"

struct RGBPacket {
    uint8_t *data;
    int size;
};

struct RGBPacketNode{
    RGBPacket pkt;
    struct RGBPacketNode *next;
};

struct RGBPacketQueue {
    RGBPacketNode *first_pkt, *last_pkt;
    int size_queue; // 链表大小
    int nb_queue;   // 链表中结点个数

    pthread_mutex_t pthread_mutex; // 互斥锁
};

void queue_rgb_init(RGBPacketQueue *queue);
int queue_rgb_put(RGBPacketQueue *queue, RGBPacket *rgbPacket);
int queue_rgb_get(RGBPacketQueue *queue, RGBPacket *rgbPacket);

#endif //FFMPEG4ANDROID_UTILS_RENDER_H
