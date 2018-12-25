// 链表
// Created by jianddongguo on 2018/12/5.
//

#include "utils_video.h"

void queue_rgb_init(RGBPacketQueue *rgbQueue) {
    // 初始化RGB链表
    memset(rgbQueue, 0, sizeof(RGBPacketQueue));
    // 初始化互斥锁
    pthread_mutex_init(&rgbQueue->mutex, NULL);
}

int queue_rgb_put(RGBPacketQueue *rgbQueue, RGBPacket *rgbPkt) {
    RGBPacketNode *rgbPktNode;
    if (rgbQueue == NULL || rgbPkt == NULL) {
        LOGE("RGBPacketQueue or RGBPacket is NULL in queue_RGB_put()");
        return -1;
    }
    // 构造节点
    rgbPktNode = (RGBPacketNode *) malloc(sizeof(RGBPacketNode));
    if (! rgbPktNode) {
        LOGE("create RGBPacketNode failed in queue_RGB_put()");
        return -1;
    }
    rgbPktNode->pkt = *rgbPkt;
    rgbPktNode->next = NULL;
    // 开启互斥锁，插入节点到链表
    pthread_mutex_lock(&rgbQueue->mutex);

    if (rgbQueue->first_pkt == NULL) {
        rgbQueue->first_pkt = rgbPktNode;
        rgbQueue->last_pkt = rgbPktNode;
    } else {
        rgbQueue->last_pkt->next = rgbPktNode;
    }
    rgbQueue->last_pkt = rgbPktNode;
    rgbQueue->nb_packets++;
    rgbQueue->queueSize += rgbPktNode->pkt.size;
    // 关闭互斥锁，释放资源
    pthread_mutex_unlock(&rgbQueue->mutex);
    LOGI("##### write data to RGB queue success.");
    return 1;
}

int queue_rgb_get(RGBPacketQueue *rgbQueue, RGBPacket *rgbPkt) {
    if (rgbQueue == NULL) {
        LOGE("RGBPacketQueue is NULL in queue_RGB_get()");
        return -1;
    }
    RGBPacketNode *rgbPacketNode;
    // 开启互斥锁，从链表头取一个节点
    pthread_mutex_lock(&rgbQueue->mutex);
    rgbPacketNode = rgbQueue->first_pkt;
    if (! rgbPacketNode) {
        pthread_mutex_unlock(&rgbQueue->mutex);
        return -1;
    }
    // 重新设置链表头，如果指向的下一个节点为NULL，说明链表中没有数据包了
    // 将last_pkt置空
    rgbQueue->first_pkt = rgbPacketNode->next;
    if (rgbQueue->first_pkt == NULL) {
        rgbQueue->last_pkt = NULL;
    }
    rgbQueue->nb_packets--;
    rgbQueue->queueSize -= rgbPacketNode->pkt.size;
    *rgbPkt = rgbPacketNode->pkt;
    // 关闭互斥锁
    pthread_mutex_unlock(&rgbQueue->mutex);
    // 释放节点内存
    free(rgbPacketNode);
    LOGI("##### get RGB data from queue success.");
    return 1;
}
