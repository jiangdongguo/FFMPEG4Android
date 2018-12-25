// 链表
// Created by jianddongguo on 2018/12/5.
//

#include "queue_stream.h"

void queue_pcm_init2(PCMPacketQueue2 *pcmQueue) {
    // 初始化PCM链表
    memset(pcmQueue, 0, sizeof(PCMPacketQueue2));
    // 初始化互斥锁
    pthread_mutex_init(&pcmQueue->mutex, NULL);
}

int queue_pcm_put2(PCMPacketQueue2 *pcmQueue, PCMPacket2 *pcmPkt) {
    PCMPacketNode2 *pcmPktNode;
    if (pcmQueue == NULL || pcmPkt == NULL) {
        LOGE("PCMPacketQueue2 or PCMPacket is NULL in queue_pcm_put()");
        return -1;
    }
    // 构造节点
    pcmPktNode = (PCMPacketNode2 *) malloc(sizeof(PCMPacketNode2));
    if (! pcmPktNode) {
        LOGE("create PCMPacketNode2 failed in queue_pcm_put()");
        return -1;
    }
    pcmPktNode->pkt = *pcmPkt;
    pcmPktNode->next = NULL;
    // 开启互斥锁，插入节点到链表
    pthread_mutex_lock(&pcmQueue->mutex);

    if (pcmQueue->first_pkt == NULL) {
        pcmQueue->first_pkt = pcmPktNode;
        pcmQueue->last_pkt = pcmPktNode;
    } else {
        pcmQueue->last_pkt->next = pcmPktNode;
    }
    pcmQueue->last_pkt = pcmPktNode;
    pcmQueue->nb_packets++;
    pcmQueue->queueSize += pcmPktNode->pkt.size;
    // 关闭互斥锁，释放资源
    pthread_mutex_unlock(&pcmQueue->mutex);
    LOGI("##### write data to pcm queue success.");
    return 1;
}

int queue_pcm_get2(PCMPacketQueue2 *pcmQueue, PCMPacket2 *pcmPkt) {
    if (pcmQueue == NULL) {
        LOGE("PCMPacketQueue2 is NULL in queue_pcm_get()");
        return -1;
    }
    PCMPacketNode2 *PCMPacketNode2;
    // 开启互斥锁，从链表头取一个节点
    pthread_mutex_lock(&pcmQueue->mutex);
    PCMPacketNode2 = pcmQueue->first_pkt;
    if (! PCMPacketNode2) {
        pthread_mutex_unlock(&pcmQueue->mutex);
        return -1;
    }
    // 重新设置链表头，如果指向的下一个节点为NULL，说明链表中没有数据包了
    // 将last_pkt置空
    pcmQueue->first_pkt = PCMPacketNode2->next;
    if (pcmQueue->first_pkt == NULL) {
        pcmQueue->last_pkt = NULL;
    }
    pcmQueue->nb_packets--;
    pcmQueue->queueSize -= PCMPacketNode2->pkt.size;
    *pcmPkt = PCMPacketNode2->pkt;
    // 关闭互斥锁
    pthread_mutex_unlock(&pcmQueue->mutex);
    // 释放节点内存
    free(PCMPacketNode2);
    LOGI("##### get pcm data from queue success.");
    return 1;
}

void queue_rgb_init2(RGBPacketQueue2 *rgbQueue) {
    // 初始化RGB链表
    memset(rgbQueue, 0, sizeof(RGBPacketQueue2));
    // 初始化互斥锁
    pthread_mutex_init(&rgbQueue->mutex, NULL);
}

int queue_rgb_put2(RGBPacketQueue2 *rgbQueue, RGBPacket2 *rgbPkt) {
    RGBPacketNode2 *rgbPktNode;
    if (rgbQueue == NULL || rgbPkt == NULL) {
        LOGE("RGBPacketQueue or RGBPacket is NULL in queue_RGB_put()");
        return -1;
    }
    // 构造节点
    rgbPktNode = (RGBPacketNode2 *) malloc(sizeof(RGBPacketNode2));
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

int queue_rgb_get2(RGBPacketQueue2 *rgbQueue, RGBPacket2 *rgbPkt) {
    if (rgbQueue == NULL) {
        LOGE("RGBPacketQueue is NULL in queue_RGB_get()");
        return -1;
    }
    RGBPacketNode2 *rgbPacketNode;
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
