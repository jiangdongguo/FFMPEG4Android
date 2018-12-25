// 三个线程，一个解码，一个播放PCM，一个渲染RGB
// Created by jianddongguo on 2018/12/14.
//

#ifndef FFMPEG4ANDROID_THREADSPLAYAUDIO_H
#define FFMPEG4ANDROID_THREADSPLAYAUDIO_H

#ifdef __cplusplus
extern "C" {
#endif
#include <jni.h>
#include "ffmpeg_stream.h"
#include "openSLES_stream.h"
#include "queue_stream.h"

extern JavaVM *j_vm_stream;
extern jobject globalSurfaceObj_stream;
void *decode_stream_thread(void *argv);

void stopAllThread_stream();

#ifdef __cplusplus
}
#endif

#endif
