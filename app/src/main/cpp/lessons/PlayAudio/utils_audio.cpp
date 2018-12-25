// 链表
// Created by jianddongguo on 2018/12/5.
//

#include "utils_audio.h"

void queue_pcm_init(PCMPacketQueue *pcmQueue) {
    // 初始化PCM链表
    memset(pcmQueue, 0, sizeof(PCMPacketQueue));
    // 初始化互斥锁
    pthread_mutex_init(&pcmQueue->mutex, NULL);
}

int queue_pcm_put(PCMPacketQueue *pcmQueue, PCMPacket *pcmPkt) {
    PCMPacketNode *pcmPktNode;
    if (pcmQueue == NULL || pcmPkt == NULL) {
        LOGE("PCMPacketQueue or PCMPacket is NULL in queue_pcm_put()");
        return -1;
    }
    // 构造节点
    pcmPktNode = (PCMPacketNode *) malloc(sizeof(PCMPacketNode));
    if (! pcmPktNode) {
        LOGE("create PCMPacketNode failed in queue_pcm_put()");
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

int queue_pcm_get(PCMPacketQueue *pcmQueue, PCMPacket *pcmPkt) {
    if (pcmQueue == NULL) {
        LOGE("PCMPacketQueue is NULL in queue_pcm_get()");
        return -1;
    }
    PCMPacketNode *pcmPacketNode;
    // 开启互斥锁，从链表头取一个节点
    pthread_mutex_lock(&pcmQueue->mutex);
    pcmPacketNode = pcmQueue->first_pkt;
    if (! pcmPacketNode) {
        pthread_mutex_unlock(&pcmQueue->mutex);
        return -1;
    }
    // 重新设置链表头，如果指向的下一个节点为NULL，说明链表中没有数据包了
    // 将last_pkt置空
    pcmQueue->first_pkt = pcmPacketNode->next;
    if (pcmQueue->first_pkt == NULL) {
        pcmQueue->last_pkt = NULL;
    }
    pcmQueue->nb_packets--;
    pcmQueue->queueSize -= pcmPacketNode->pkt.size;
    *pcmPkt = pcmPacketNode->pkt;
    // 关闭互斥锁
    pthread_mutex_unlock(&pcmQueue->mutex);
    // 释放节点内存
    free(pcmPacketNode);
    LOGI("##### get pcm data from queue success.");
    return 1;
}
