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
    AVFormatContext *inputFormatCtx  = NULL;

    AVPacket *avPacket  = NULL;
    AVCodec *vCodec  = NULL;
    AVCodecContext *vCodecCtx  = NULL;
    SwsContext *swsContext = NULL;
    AVFrame *yuvFrame  = NULL;
    AVFrame *rgbFrame  = NULL;
    uint8_t * rgb_buffer;
    int id_video_stream;
    int video_height;
    int video_width;
};

int createRenderFFmpeg(char * url);
int readH264DataFromAVPacket();
int decodeH264Data();
void releaseRenderFFmpeg();
int getVideoWidth();
int getVideoHeight();


extern FFmpegRender g_render;

#ifdef __cplusplus
};
#endif
#endif //FFMPEG4ANDROID_FFMPEG_RENDER_H
