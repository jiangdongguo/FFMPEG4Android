// 保存网络流
// Created by Administrator on 2018/12/18.
//

#ifndef FFMPEG4ANDROID_FFMPEG_SAVE_H
#define FFMPEG4ANDROID_FFMPEG_SAVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ffmpeg.h"

typedef struct FFmpeg4SaveStream{
    AVFormatContext *mInputFmt;
    AVFormatContext *mOutputFmt;
    AVPacket *pAvPacket;

    int quit_save = 0;
}Save_Stream;

void initFFmpeg();
int openInputContext(const char * inputUrl);
void closeInputContext();
AVPacket * readAVPacketFromInput();
int writeAVPacket(AVPacket *pkt);
int openOutputContext(const char * outputUrl);
void closeOutputContext();
void releaseFFmpeg();

extern Save_Stream global_save;

#ifdef __cplusplus
}
#endif

#endif //FFMPEG4ANDROID_FFMPEG_SAVE_H
