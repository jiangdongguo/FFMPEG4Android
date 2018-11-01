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

void initFFmpeg();
int openInput(const char* url);
int openOutput(const char * outPath);
AVPacket * readPacketFromSource();
int writePacket(AVPacket* packet);
void releaseFFmpeg();

#ifdef __cplusplus
}
#endif

#endif //FFMPEG4ANDROID_SAVESTREAM_H
