// 三个线程，一个解码，一个播放PCM，一个渲染RGB
// Created by jianddongguo on 2018/12/4.
//

#include <unistd.h>
#include "threads_play_stream.h"
#include "native_window.h"

pthread_t threadId_open_opensl2;
void *play_audio_thread2(void *argv);
void *thread_open_opensl2(void *argv);
void *render_video_thread2(void *argv);

// 解码线程，将被主线程启动
void *decode_media_thread(void *argv) {
    global_stream.quit = 0;

    // 初始化FFmpeg引擎
    int ret = createFFmpegEngine_stream((const char *) argv);

    if (ret < 0) {
        LOGE("create FFmpeg Engine failed in decode_audio_thread");
        releaseFFmpegEngine_stream();
        return NULL;
    }

    // 初始化链表
    queue_pcm_init2(&global_stream.pcm_queue);
    queue_rgb_init2(&global_stream.yuv_queue);

    PCMPacket2 *pcmPkt = (PCMPacket2 *) malloc(sizeof(PCMPacket2));
    // 启动音频播放线程
    pthread_t id_play_audio;
    pthread_create(&id_play_audio, NULL, play_audio_thread2, NULL);
    // 启动视频渲染线程
    pthread_t id_play_video;
    pthread_create(&id_play_video, NULL, render_video_thread2, NULL);

    while (! global_stream.quit) {
        int stream_index = readAVPacket_stream();
        if (stream_index < 0) {
            global_stream.quit = 1;
            LOGW("#####------------->解析包失败。原因：解析完毕或网络异常");
            break;
        }
        if (stream_index == global_stream.index_audio) {

            // 解码播放
            uint8_t *data = NULL;
            int nb_samples = decodeAudio_stream(&data);

            // 插入到音频队列链表
            if (nb_samples > 0 && data != NULL) {
                pcmPkt->pcm = (char *) data;
                pcmPkt->size = nb_samples;
                queue_pcm_put2(&global_stream.pcm_queue, pcmPkt);
            }
        } else if (stream_index == global_stream.index_video) {

            // 解码渲染，YUV转RGB
            uint8_t *yuvData = NULL;
            int strideSize = decodeVideo_stream(&yuvData);
            // 将YUV数据插入到视频链表
            if (strideSize > 0) {
                RGBPacket2 *pkt = (RGBPacket2 *) malloc(sizeof(RGBPacket2));
                pkt->rgb = global_stream.rgb_out_buffer;
                pkt->size = strideSize;
                queue_rgb_put2(&global_stream.yuv_queue, pkt);
            }
        }
    }
    // 等待1s钟，确定消费者线程执行完毕
    LOGI("#####------------->start exiting decode media_thread");
    if (global_stream.quit) {
        sleep(1);
    }
    releaseFFmpegEngine_stream();
    if(pcmPkt) {
        free(pcmPkt);
    }
    LOGI("#####------------->end exiting decode media_thread");
    return NULL;
}

// 音频播放线程
void *play_audio_thread2(void *argv) {
    PCMPacket2 pcmPacket;
    PCMPacket2 *pkt = &pcmPacket;
    playQueueInit_stream(&global_openSL_stream.queue_play);

    pthread_create(&threadId_open_opensl2, NULL, thread_open_opensl2, NULL);

    for (;;) {
        if (global_stream.quit) {
            break;
        }
        if (queue_pcm_get2(&global_stream.pcm_queue, pkt) > 0) {
            // 写入数据
            PCMData2 *pcmData = (PCMData2 *) malloc(sizeof(PCMData2));
            pcmData->pcm = pkt->pcm;
            pcmData->size = pkt->size;
            playQueuePut_stream(&global_openSL_stream.queue_play, pcmData);
        }
    }
}

// 视频渲染线程
void *render_video_thread2(void *argv) {
    RGBPacket2 rgb;
    RGBPacket2 *pkt = &rgb;
    JNIEnv *env = NULL;
    // 绑定线程到JavaVM，并从JavaVM获取JNIEnv*
    if (j_vm_stream->GetEnv((void **) &env, JNI_VERSION_1_4) < 0) {
        if (j_vm_stream->AttachCurrentThread(&env, NULL) < 0) {
            LOGE("##### get JNIEnv * failed in thread render video.");
            return NULL;
        }
    }
    // 创建Native Window
    int ret = createANativeWinidow_stream(env, globalSurfaceObj_stream);
    if (ret < 0) {
        LOGE("create Native window failed in render_video_thread");
        if (j_vm_stream) {
            j_vm_stream->DetachCurrentThread();
        }
        return NULL;
    }
    // 配置Window属性
    changeBuffersGeometry_stream(global_stream.videoWidth, global_stream.videoHeight);

    for (;;) {
        if (global_stream.quit) {
            break;
        }
        // 渲染rgb
        if (queue_rgb_get2(&global_stream.yuv_queue, pkt) > 0) {
            renderWindow_stream(pkt->rgb, pkt->size);
        }
    }
    // 销毁Native Window
    destoryANativeWinidow_stream();
    if (j_vm_stream) {
        j_vm_stream->DetachCurrentThread();
    }
}

// 初始化openSL ES线程
void *thread_open_opensl2(void *argv) {
    int ret = createOpenSLEngine_stream(global_stream.channels, global_stream.sample_rate,
                                 getSampleFormat(global_stream.sample_fmt));
    if (ret < 0) {
        global_stream.quit = 1;
        LOGE("create OpenSL Engine failed in play_audio_thread");
        return NULL;
    }

    pthread_exit(&threadId_open_opensl2);
}

void stopAllThread_stream() {
    global_stream.quit = 1;

    global_openSL_stream.isExit = 1;
}
