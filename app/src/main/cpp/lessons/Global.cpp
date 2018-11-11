// 全局化函数
// Created by jiangdongguo on 2018/11/5.
//

#include "Global.h"

//-----------------------FFmpeg初始化/释放资源-----------------------

void ffmpeg_init() {
    // 注册所有编解码器、muxer和demuxer等
    av_register_all();
    // 初始化filter system
    avfilter_register_all();
    // 网络组件全局初始化
    avformat_network_init();
    // 设置log等级
    av_log_set_level(AV_LOG_ERROR);
}

void ffmpeg_deinit() {
    avformat_network_deinit();
}

//-----------------------操作AVPacket数据函数集-----------------------

void packet_queue_init(PacketQueue *queue) {
    memset(queue, 0, sizeof(queue));
    // 互斥锁初始化
    pthread_mutex_init(queue->mutex, NULL);
}

int packet_queue_put(PacketQueue *queue, AVPacket *avPkt) {
    AVPacketList *pktListNode;
    if (queue == NULL || avPkt == NULL) {
        LOGW("packet_queue_put failed: queue or avPacket is NULL");
        return -1;
    }
    AVPacket *tmpPkt = (AVPacket *) av_malloc(sizeof(AVPacket));;
    av_init_packet(tmpPkt);
    if (av_packet_ref(tmpPkt, avPkt) < 0) {
        LOGW("packet_queue_put failed: av_packet_ref() failed");
        return -1;
    }
    // 根据avPkt构造新的node
    pktListNode = (AVPacketList *) av_malloc(sizeof(AVPacketList));
    if (pktListNode == NULL) {
        LOGW("packet_queue_put failed: pktListNode is NULL");
        return -1;
    }
    pktListNode->pkt = *avPkt;
    pktListNode->next = NULL;

    // 开启互斥锁
    pthread_mutex_lock(queue->mutex);

    // 插入节点到链表
    if (queue->last_pkt != NULL) {
        queue->last_pkt->next = pktListNode;
    } else {
        queue->first_pkt = pktListNode;
    }
    queue->last_pkt = pktListNode;
    queue->nb_packets++;
    queue->size += pktListNode->pkt.size;

    // 关闭互斥锁
    pthread_mutex_unlock(queue->mutex);
    av_packet_unref(tmpPkt);
    return 0;
}

int packet_queue_get(PacketQueue *queue, AVPacket *avPacket) {
    AVPacketList *pktListNode;
    if (mGlobalCtx.quit) {
        LOGI("packet_queue_get: quit");
        return -1;
    }
    // 开启互斥锁
    pthread_mutex_lock(queue->mutex);

    // 取出节点
    pktListNode = queue->first_pkt;
    if (pktListNode == NULL) {
        LOGI("packet_queue_get: get Node from queue failed");
        return 0;
    }
    queue->first_pkt = pktListNode->next;
    if (!queue->first_pkt) {
        queue->last_pkt = NULL;
    }

    queue->nb_packets--;
    queue->size -= pktListNode->pkt.size;
    *avPacket = pktListNode->pkt;
    free(pktListNode);
    // 关闭互斥锁
    pthread_mutex_unlock(queue->mutex);
    return 1;
}