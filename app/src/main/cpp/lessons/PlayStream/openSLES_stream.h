//
// Created by jianddongguo on 2018/12/3.
//
#ifndef FFMPEG4ANDROID_OPENSLES_H
#define FFMPEG4ANDROID_OPENSLES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "android/log.h"
#include <stdint.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>

#define TAG "PlayAudio"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,"%s",__VA_ARGS__)
#define LOG_I(format,...) __android_log_print(ANDROID_LOG_INFO,TAG,format,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,"%s",__VA_ARGS__)
#define LOG_E(format, ...) __android_log_print(ANDROID_LOG_ERROR,TAG,format,__VA_ARGS__)

struct PCMData2 {
    char *pcm;
    int size;
};

struct PCMDataNode2 {
    PCMData2 pkt;
    struct PCMDataNode2 *next;
};

struct PCMDataQueue2 {
    PCMDataNode2 *first_pkt, *last_pkt;
    int nb_packets;

    pthread_mutex_t mutex;    // 互斥锁
};

typedef struct OpenSLES_Stream {
    // 引擎
    SLObjectItf pEngineObject = NULL;
    SLEngineItf pEngineItf = NULL;
    // 混音器
    SLObjectItf pOutputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    // 播放器
    SLObjectItf pPlayerObject = NULL;
    SLPlayItf pPlayerItf = NULL;
    SLVolumeItf pVolumeItf = NULL;
    SLAndroidSimpleBufferQueueItf pBufferItf = NULL;

    PCMDataQueue2 queue_play;
    int isExit = 0;
} Global_OpenSL_Stream;

int createOpenSLEngine_stream(int nbChannels,int sampleRate,int sampleFormat);
void destoryOpenSLEngine_stream();

void playQueueInit_stream(PCMDataQueue2 *pcmQueue);
int playQueuePut_stream(PCMDataQueue2 *pcmQueue, PCMData2 *pcmPkt);
int playQueueGet_stream(PCMDataQueue2 *pcmQueue, PCMData2 *pcmPkt);
int getQueueSize_stream(PCMDataQueue2 *pcmQueue);

extern Global_OpenSL_Stream global_openSL_stream;

#ifdef __cplusplus
}
#endif
#endif