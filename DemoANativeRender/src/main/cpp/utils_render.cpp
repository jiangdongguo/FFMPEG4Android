// 链表操作函数
// Created by Jiangdg on 2019/9/25.
//

#include "utils_render.h"

void queue_rgb_init(RGBPacketQueue *queue) {
    memset(queue, 0 , sizeof(RGBPacketQueue));
    // 初始化互斥锁
    pthread_mutex_init(&queue->pthread_mutex, NULL);
}

int queue_rgb_put(RGBPacketQueue *queue, RGBPacket *rgbPacket) {
    if(! queue || !rgbPacket) {
        RLOG_E("put rgb to queue faile,queue or rgb packet is null");
        return -1;
    }
    pthread_mutex_lock(&queue->pthread_mutex);
    // 创建新的结点
    RGBPacketNode *pktNode = (RGBPacketNode *)malloc(sizeof(RGBPacketNode));
    if(! pktNode) {
        pthread_mutex_unlock(&queue->pthread_mutex);
        RLOG_E("create a node failed");
        return -1;
    }
    pktNode->pkt = *rgbPacket;
    pktNode->next = NULL;
    // 插入结点到链表
    if(queue->first_pkt == NULL) {
        queue->first_pkt = pktNode;
        queue->last_pkt = pktNode;
    } else {
        queue->last_pkt->next = pktNode;
    }
    queue->last_pkt = pktNode;

    queue->size_queue += rgbPacket->size;
    queue->nb_queue++;
    pthread_mutex_unlock(&queue->pthread_mutex);
    return 1;
}

int queue_rgb_get(RGBPacketQueue *queue, RGBPacket *rgbPacket) {
    RGBPacketNode *node;
    if(! queue || !rgbPacket) {
        RLOG_E("put rgb to queue faile,queue or rgb packet is null");
        return -1;
    }
    pthread_mutex_lock(&queue->pthread_mutex);
    // 从链表中取出头结点
    // 然后重新设置头指针指向
    node = queue->first_pkt;
    if(! node) {
        pthread_mutex_unlock(&queue->pthread_mutex);
        RLOG_E("first node is null");
        return -1;
    }
    queue->first_pkt = node->next;
    if(! queue->first_pkt) {
        queue->last_pkt = NULL;
    }

    queue->nb_queue--;
    queue->size_queue -= node->pkt.size;
    *rgbPacket = node->pkt;
    free(node);
    // 释放结点内存，释放互斥锁
    pthread_mutex_unlock(&queue->pthread_mutex);
    return 1;
}