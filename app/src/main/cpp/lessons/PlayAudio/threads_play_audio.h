// 两个线程，一个解码，一个播放PCM
// Created by jianddongguo on 2018/12/4.
//

#ifndef FFMPEG4ANDROID_THREADSPLAYAUDIO_H
#define FFMPEG4ANDROID_THREADSPLAYAUDIO_H

#ifdef __cplusplus
extern "C" {
#endif
#include <jni.h>
#include "ffmpeg_audio.h"
#include "openSLES_audio.h"
#include "utils_audio.h"

extern JavaVM * j_vm;
extern  jobject globalNativeFFmpegObj;
void *decode_audio_thread(void *argv);

void stopAllThread_audio();

#ifdef __cplusplus
}
#endif

#endif
