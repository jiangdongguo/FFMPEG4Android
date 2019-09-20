// FFmpeg使用函数头文件
// Created by Jiangdg on 2019/9/17.
//

#ifndef FFMPEG4ANDROID_FFMPEG_SAVE_H
#define FFMPEG4ANDROID_FFMPEG_SAVE_H

#ifdef __cplusplus
extern "C"{
#endif

#include "native_save.h"
#include "ffmpeg.h"

typedef struct SaveStream{
    AVFormatContext *inputCtx;
    AVFormatContext *outputCtx;

    AVPacket *avPacket;
}FFmpegSaveStream;

void initFFmpeg();
int openInput(char *input_url);
int openOutput(char *out);
void closeInput();
void closeOutput();
int readAvPacketFromInput();
int writeAvPacketToOutput();
void releaseFFmpeg();

extern FFmpegSaveStream g_save;

#ifdef __cplusplus
};
#endif

#endif //FFMPEG4ANDROID_FFMPEG_SAVE_H
