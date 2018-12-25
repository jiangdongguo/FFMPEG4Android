// 解码线程、渲染线程
// Created by jianddongguo on 2018/12/13.
//

#include <unistd.h>
#include "threads_play_video.h"
#include "nativeWindow_video.h"

void *render_video_thread(void *argv);

void *decode_video_thread(void *argv) {
    global_dVideo.quit = 0;

    // 初始化FFmpeg引擎
    int ret = createFFmpeg4Video((const char *) argv);
    LOG_I("ret = %d", ret);
    if (ret < 0) {
        LOGI("create FFmpeg failed in decode_video_thread()");
        return NULL;
    }

    // 初始化链表
    queue_rgb_init(&global_dVideo.yuv_queue);
    // 启动渲染线程
    pthread_t threadId_render;
    pthread_create(&threadId_render, NULL, render_video_thread, NULL);

    while (readH264Packet() == 0) {
        if (global_dVideo.quit) {
            break;
        }
        // 解码，YUV转RGB
        uint8_t *yuvData = NULL;
        int strideSize = decodeVideo(&yuvData);
        // 将YUV数据插入到链表
        if (strideSize > 0) {
            RGBPacket *pkt = (RGBPacket *) malloc(sizeof(RGBPacket));
            pkt->rgb = global_dVideo.rgb_out_buffer;
            pkt->size = strideSize;
            queue_rgb_put(&global_dVideo.yuv_queue, pkt);
        }
    }
    // 睡眠1s,等待渲染线程关闭
    sleep(1);
    // 释放FFmpeg引擎
    releaseFFmpeg4Video();
}

void *render_video_thread(void *argv) {
    RGBPacket rgb;
    RGBPacket *pkt = &rgb;
    JNIEnv *env = NULL;
    // 绑定线程到JavaVM，并从JavaVM获取JNIEnv*
    if(j_vm_video->GetEnv((void **)&env,JNI_VERSION_1_4) < 0) {
       if(j_vm_video->AttachCurrentThread(&env,NULL) < 0) {
           LOGE("##### get JNIEnv * failed in thread render video.");
           return NULL;
       }
    }
    // 创建Native Window
    int ret = createANativeWinidow(env,globalSurfaceObj_video);
    if(ret < 0) {
        LOGE("create Native window failed in render_video_thread");
        if(j_vm_video) {
            j_vm_video->DetachCurrentThread();
        }
        return NULL;
    }
    // 配置Window属性
    changeBuffersGeometry(global_dVideo.videoWidth,global_dVideo.videoHeight);

    for(;;) {
        if (global_dVideo.quit) {
            break;
        }
        // 渲染rgb
        if(queue_rgb_get(&global_dVideo.yuv_queue,pkt) > 0) {
            renderWindow(pkt->rgb,pkt->size);
        }
    }
    // 销毁Native Window
    destoryANativeWinidow();
    if(j_vm_video) {
        j_vm_video->DetachCurrentThread();
    }
}

void stop_threads_video() {
    global_dVideo.quit = 1;
}



