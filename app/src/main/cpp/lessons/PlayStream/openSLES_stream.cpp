// OpenSL ES引擎驱动函数集
// Created by jianddongguo on 2018/12/3.
//

#include <unistd.h>
#include "openSLES_stream.h"

SLuint32 getSamplesPerSec_stream(SLuint32 sampleRate);

SLuint32 getBitsPerSample_stream(SLuint32 sampleFormat);

SLuint32 getChannelMask_stream(SLuint32 channels);

Global_OpenSL_Stream global_openSL_stream;

void pcmBufferCallBack_stream(SLAndroidSimpleBufferQueueItf bf, void *context);

int createOpenSLEngine_stream(int nbChannels, int sampleRate, int sampleFormat) {
    destoryOpenSLEngine_stream();

    // 1. 使用SLOjectItf创建实现引擎接口，并利用该接口初始化具体的引擎对象
    SLresult slRet = slCreateEngine(&global_openSL_stream.pEngineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != slRet) {
        LOGE("create engine failed");
        return -1;
    }
    slRet = (*global_openSL_stream.pEngineObject)->Realize(global_openSL_stream.pEngineObject, SL_BOOLEAN_FALSE);
    slRet = (*global_openSL_stream.pEngineObject)->GetInterface(global_openSL_stream.pEngineObject, SL_IID_ENGINE,
                                                         &global_openSL_stream.pEngineItf);
    // 2. 创建混音器
    SLInterfaceID effect[1] = {SL_IID_ENVIRONMENTALREVERB};     // 环境回响音效
    SLboolean boolValue[1] = {SL_BOOLEAN_FALSE};  // 非强制性开启音响逻辑
    slRet = (*global_openSL_stream.pEngineItf)->CreateOutputMix(global_openSL_stream.pEngineItf,
                                                         &global_openSL_stream.pOutputMixObject, 1, effect,
                                                         boolValue);
    if (SL_RESULT_SUCCESS != slRet) {
        LOGE("create output mix failed");
        return -1;
    }
    slRet = (*global_openSL_stream.pOutputMixObject)->Realize(global_openSL_stream.pOutputMixObject,
                                                       SL_BOOLEAN_FALSE);
    slRet = (*global_openSL_stream.pOutputMixObject)->GetInterface(global_openSL_stream.pOutputMixObject,
                                                            SL_IID_ENVIRONMENTALREVERB,
                                                            &global_openSL_stream.outputMixEnvironmentalReverb);
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    if (SL_RESULT_SUCCESS == slRet) {
        slRet = (*global_openSL_stream.outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                global_openSL_stream.outputMixEnvironmentalReverb, &reverbSettings);
    }

    // 3. 设置播放器参数，创建播放器
    // --->配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};

    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,              // 播放PCM格式数据
            (SLuint32) nbChannels,                     // 通道数量，立体声
            getSamplesPerSec_stream((SLuint32) sampleRate),   // 采样率
            getBitsPerSample_stream((SLuint32) sampleFormat), // 采样深度
            getBitsPerSample_stream((SLuint32) sampleFormat),
            getChannelMask_stream((SLuint32) nbChannels),  // 前作前右
            SL_BYTEORDER_LITTLEENDIAN       // 结束标志
    };
    SLDataSource pAudioSrc = {&android_queue, &format_pcm};
    // --->配置aduioSnk信息
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, global_openSL_stream.pOutputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};
    SLuint32 numInterfaces_audio = 3;
    const SLInterfaceID ids_audio[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean requireds_audio[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    // --->创建播放器，获取各类接口
    slRet = (*global_openSL_stream.pEngineItf)->CreateAudioPlayer(
            global_openSL_stream.pEngineItf,                                 // 创建音频播放器
            &global_openSL_stream.pPlayerObject,
            &pAudioSrc,
            &audioSnk,
            numInterfaces_audio,
            ids_audio,
            requireds_audio);
    slRet = (*global_openSL_stream.pPlayerObject)->Realize(global_openSL_stream.pPlayerObject,
                                                    SL_BOOLEAN_FALSE);                                   // 实现音频播放器
    slRet = (*global_openSL_stream.pPlayerObject)->GetInterface(global_openSL_stream.pPlayerObject, SL_IID_PLAY,
                                                         &global_openSL_stream.pPlayerItf);         // 获取播放器接口
    slRet = (*global_openSL_stream.pPlayerObject)->GetInterface(global_openSL_stream.pPlayerObject, SL_IID_VOLUME,
                                                         &global_openSL_stream.pVolumeItf);      // 获取音量接口
    // 4. 设置缓冲队列和回掉函数
    slRet = (*global_openSL_stream.pPlayerObject)->GetInterface(global_openSL_stream.pPlayerObject,
                                                         SL_IID_BUFFERQUEUE,
                                                         &global_openSL_stream.pBufferItf);  // 获取缓冲区接口
    if (SL_RESULT_SUCCESS != slRet) {
        LOGE("create audio player failed");
        return -1;
    }
    (*global_openSL_stream.pBufferItf)->RegisterCallback(global_openSL_stream.pBufferItf, pcmBufferCallBack_stream,
                                                  NULL);   // 注册回调接口

    //    获取播放状态接口
    (*global_openSL_stream.pPlayerItf)->SetPlayState(global_openSL_stream.pPlayerItf, SL_PLAYSTATE_PLAYING);
//    主动调用回调函数开始工作
    pcmBufferCallBack_stream(global_openSL_stream.pBufferItf, NULL);
    LOGI("create OpenSL ES Engine");
    return 0;
}

void pcmBufferCallBack_stream(SLAndroidSimpleBufferQueueItf bf, void *context) {
    PCMData2 pcmData;
    PCMData2 *data = &pcmData;

    SLresult result = NULL;
    for(;;){
        if (!global_openSL_stream.isExit && playQueueGet_stream(&global_openSL_stream.queue_play, data) > 0) {
            LOGI("start to play......");
            result = (*global_openSL_stream.pBufferItf)->Enqueue(global_openSL_stream.pBufferItf, data->pcm,
                                                          data->size);
            if (SL_RESULT_SUCCESS == result) {
                LOGI("end to play......");
                break;
            }
        }

        if(global_openSL_stream.isExit) {
            LOGI("stop to play......");
            break;
        }
    }
}

void destoryOpenSLEngine_stream() {
    if (global_openSL_stream.pPlayerObject) {
        (*global_openSL_stream.pPlayerItf)->SetPlayState(global_openSL_stream.pPlayerItf, SL_PLAYSTATE_STOPPED);
        (*global_openSL_stream.pPlayerObject)->Destroy(global_openSL_stream.pPlayerObject);
        global_openSL_stream.pPlayerObject = NULL;
        global_openSL_stream.pPlayerItf = NULL;
        global_openSL_stream.pVolumeItf = NULL;
    }
    if (global_openSL_stream.pOutputMixObject) {
        (*global_openSL_stream.pOutputMixObject)->Destroy(global_openSL_stream.pOutputMixObject);
        global_openSL_stream.pOutputMixObject = NULL;
        global_openSL_stream.pBufferItf = NULL;
        global_openSL_stream.outputMixEnvironmentalReverb = NULL;
    }
    if (global_openSL_stream.pEngineObject) {
        (*global_openSL_stream.pEngineObject)->Destroy(global_openSL_stream.pEngineObject);
        global_openSL_stream.pEngineObject = NULL;
        global_openSL_stream.pEngineItf = NULL;
    }

    global_openSL_stream.isExit = 0;
    LOGI("##### destory OpenSL ES Engine");
}

SLuint32 getSamplesPerSec_stream(SLuint32 sampleRate) {
    if (sampleRate == 8000) {
        return SL_SAMPLINGRATE_8;
    } else if (sampleRate == 12000) {
        return SL_SAMPLINGRATE_12;
    } else if (sampleRate == 16000) {
        return SL_SAMPLINGRATE_16;
    } else {
        return SL_SAMPLINGRATE_44_1;
    }
}

SLuint32 getBitsPerSample_stream(SLuint32 sampleFormat) {
    if (sampleFormat == 8) {
        return SL_PCMSAMPLEFORMAT_FIXED_8;
    } else {
        return SL_PCMSAMPLEFORMAT_FIXED_16;
    }
}

SLuint32 getChannelMask_stream(SLuint32 channels) {
    if (channels == 2) {
        return SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    }
    return NULL;
}

void playQueueInit_stream(PCMDataQueue2 *pcmQueue) {
    // 初始化PCM链表
    memset(pcmQueue, 0, sizeof(PCMDataQueue2));
    // 互斥锁初始化
    pthread_mutex_init(&pcmQueue->mutex, NULL);
}

int playQueuePut_stream(PCMDataQueue2 *pcmQueue, PCMData2 *pcmPkt) {
    PCMDataNode2 *pcmPktNode;
    if (pcmQueue == NULL || pcmPkt == NULL) {
        LOGE("PCMPacketQueue or PCMPacket is NULL in queue_pcm_put()");
        return -1;
    }
    // 开启互斥锁，插入节点到链表
    pthread_mutex_lock(&pcmQueue->mutex);

    // 构造节点
    pcmPktNode = (PCMDataNode2 *) malloc(sizeof(PCMDataNode2));
    if (!pcmPktNode) {
        LOGE("create PCMPacketNode failed in queue_pcm_put()");
        return -1;
    }
    pcmPktNode->pkt = *pcmPkt;
    pcmPktNode->next = NULL;

    if (pcmQueue->first_pkt == NULL) {
        pcmQueue->first_pkt = pcmPktNode;
        pcmQueue->last_pkt = pcmPktNode;
    } else {
        pcmQueue->last_pkt->next = pcmPktNode;
    }
    pcmQueue->last_pkt = pcmPktNode;
    pcmQueue->nb_packets++;
    LOGI("---> write data to play.");
    // 关闭互斥锁，释放资源
    pthread_mutex_unlock(&pcmQueue->mutex);
    return 1;
}

int getQueueSize_stream(PCMDataQueue2 *pcmQueue) {
    return pcmQueue->nb_packets;
}

int playQueueGet_stream(PCMDataQueue2 *pcmQueue, PCMData2 *pcmPkt) {
    if (pcmQueue == NULL) {
        LOGE("PCMPacketQueue is NULL in queue_pcm_get()");
        return -1;
    }
    PCMDataNode2 *pcmPacketNode;
    // 开启互斥锁，从链表头取一个节点
    pthread_mutex_lock(&pcmQueue->mutex);
    pcmPacketNode = pcmQueue->first_pkt;
    if (!pcmPacketNode) {
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
    *pcmPkt = pcmPacketNode->pkt;

    // 关闭互斥锁
    pthread_mutex_unlock(&pcmQueue->mutex);
    // 释放节点内存
    free(pcmPacketNode);
    LOGI("----> get pcm data to play.");
    return 1;
}


