//
// Created by jiangdongguo on 2018/10/26.
//

#ifndef FFMPEG4ANDROID_SAVESTREAM_H
#define FFMPEG4ANDROID_SAVESTREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <android/log.h>
#include <ffmpeg.h>

#define LOG_TAG "ffmpeg"
#define LOG_I(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,"%s",__VA_ARGS__)
#define LOG_D(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,"%s",__VA_ARGS__)
#define LOG_W(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,"%s",__VA_ARGS__)
#define LOG_E(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,"%s",__VA_ARGS__)

void init();
int openInput(const char* url);
int openOutput(const char * outPath);
int readPacketFromSource();
int writePacket(AVPacket packet);
void release();

#ifdef __cplusplus
}
#endif

#endif //FFMPEG4ANDROID_SAVESTREAM_H
