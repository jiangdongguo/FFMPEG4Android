//
// Created by Administrator on 2018/12/18.
//

#include "thread_save_streams.h"

void *thread_save_stream(void *argv){
    global_save.quit_save = 0;
    ThreadParams *params = (ThreadParams *)argv;
    const char *inputUrl = params->inputUrl;
    const char *outputUrl = params->outputUrl;
    if(!inputUrl || !outputUrl) {
        LOGE("inputUrl or outputUrl is NULL");
        return NULL;
    }
    LOG_I("inputUrl=%s，outputUrl=%",inputUrl,outputUrl);

    // 初始化FFmpeg引擎
    initFFmpeg();
    // 打开输入url
    if(openInputContext(inputUrl) < 0) {
        LOG_E("open input url=%s failed",inputUrl);
        return NULL;
    }
    // 打开输出url
    if(openOutputContext(outputUrl) < 0) {
        LOG_E("open output url=%s failed",inputUrl);
        return NULL;
    }
    // 循环读取、写入数据
    for(;;) {
        if(global_save.quit_save) {
            break;
        }
        AVPacket *avPacket = readAVPacketFromInput();
        if(! avPacket) {
            LOGI("read AVPacket From Input falied.");
            continue;
        }
        writeAVPacket(avPacket);
    }

    releaseFFmpeg();
}

void stop_save_thread() {
    global_save.quit_save = 1;
}