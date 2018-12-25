// 两个线程，一个解码，一个播放PCM
// Created by jianddongguo on 2018/12/4.
//

#include "threads_play_audio.h"

pthread_t threadId_open_opensl;

void *play_audio_thread(void *argv);
void *thread_open_opensl(void * argv);

// 解码线程
void *decode_audio_thread(void *argv) {
    global_dAudio.quit = 0;

    int ret = createFFmpegEngine((const char *) argv);
    if (ret < 0) {
        LOGE("create FFmpeg Engine failed in decode_audio_thread");
        return NULL;
    }
    // 初始化链表
    queue_pcm_init(&global_dAudio.pcm_queue);

    PCMPacket *pcmPkt = (PCMPacket *) malloc(sizeof(PCMPacket));
    // 启动音频播放线程
    pthread_t threadId_play;
    pthread_create(&threadId_play, NULL, play_audio_thread, NULL);

    while (readAVPacket() >= 0) {
        // 线程终止标志
        if (global_dAudio.quit) {
            break;
        }
        // 解码
        uint8_t *data = NULL;
        int nb_samples = decodeAudio(&data);
        // 插入到队列
        if (nb_samples > 0 && data != NULL) {
            pcmPkt->pcm = (char *) data;
            pcmPkt->size = nb_samples;
            queue_pcm_put(&global_dAudio.pcm_queue, pcmPkt);
        }
    }
    releaseFFmpegEngine();
    free(pcmPkt);
    return NULL;
}

// 音频播放线程
void *play_audio_thread(void *argv) {
    PCMPacket pcmPacket;
    PCMPacket *pkt = &pcmPacket;
    playQueueInit(&global_openSL.queue_play);

    pthread_create(&threadId_open_opensl,NULL,thread_open_opensl,NULL);

    for (;;) {
        if (global_dAudio.quit) {
            break;
        }
        if (queue_pcm_get(&global_dAudio.pcm_queue, pkt) > 0) {
            // 写入数据
            PCMData *pcmData = (PCMData *) malloc(sizeof(PCMData));
            pcmData->pcm = pkt->pcm;
            pcmData->size = pkt->size;
            playQueuePut(&global_openSL.queue_play,pcmData);
        }
    }
}

void stopAllThread_audio() {
    global_dAudio.quit = 1;

    global_openSL.isExit = 1;
}

void * thread_open_opensl(void * argv) {
    int ret = createOpenSLEngine(global_dAudio.channels, global_dAudio.sample_rate,getSampleFormat(global_dAudio.sample_fmt));
    if (ret < 0) {
        global_dAudio.quit = 1;
        LOGE("create OpenSL Engine failed in play_audio_thread");
        return NULL;
    }

    pthread_exit(&threadId_open_opensl);
}
