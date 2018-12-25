//
// Created by Administrator on 2018/12/18.
//

#ifndef FFMPEG4ANDROID_THREADSAVESTREAMS_H
#define FFMPEG4ANDROID_THREADSAVESTREAMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ffmpeg_save.h"

struct ThreadParams {
    const char *inputUrl;
    const char *outputUrl;
};

void *thread_save_stream(void *argv);
void stop_save_thread();

extern ThreadParams *threadParams;

#ifdef __cplusplus
}
#endif

#endif //FFMPEG4ANDROID_THREADSAVESTREAMS_H
