//
// Created by Jiangdg on 2019/9/25.
//

#ifndef FFMPEG4ANDROID_FFMPEG_RENDER_H
#define FFMPEG4ANDROID_FFMPEG_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "native_render.h"
#include "include/ffmpeg.h"
#include <libavutil/imgutils.h>

struct FFmpegRender {
    AVFormatContext *inputFormatCtx;
    AVFormatContext *outputFormatCtx;

    AVPacket *avPacket;
    AVCodec *vCodec;
    AVCodecContext *vCodecCtx;
    SwsContext *swsContext;
    AVFrame *yuvFrame;
    AVFrame *rgbFrame;
    int id_video_stream;
};

int createRenderFFmpeg(char * url);
int readH264DataFromAVPacket();
int decodeH264Data();
void releaseRenderFFmpeg();


extern FFmpegRender g_render;

#ifdef __cplusplus
};
#endif
#endif //FFMPEG4ANDROID_FFMPEG_RENDER_H
