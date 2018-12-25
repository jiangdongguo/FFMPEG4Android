//
// Created by jianddongguo on 2018/12/13.
//

#ifndef FFMPEG4ANDROID_THREADSPLAYVIDEO_H
#define FFMPEG4ANDROID_THREADSPLAYVIDEO_H


#ifdef __cplusplus
extern "C" {
#endif

#include "ffmpeg_video.h"
#include "utils_video.h"
#include <jni.h>

void *decode_video_thread(void *argv);
void stop_threads_video();

// 声明全局
extern jobject globalSurfaceObj_video;
extern JavaVM * j_vm_video;

#ifdef __cplusplus
}
#endif

#endif //FFMPEG4ANDROID_THREADSPLAYVIDEO_H
