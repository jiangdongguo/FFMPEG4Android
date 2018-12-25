// FFmpeg引擎使用头文件
//
// Created by jianddongguo on 2018/12/3.
//
#ifndef FFMPEG4ANDROID_FFMPEG_VIDEO_H
#define FFMPEG4ANDROID_FFMPEG_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ffmpeg.h"
#include "utils_video.h"
#include <libavutil/imgutils.h>

typedef struct FFmpeg4Video {
    AVFormatContext *pVFmtCtx = NULL;  // 全局上下文
    AVCodecContext *pVCodecCtx = NULL;// 编解码器上下文
    AVCodec *pVCodec = NULL;  // 编解码器
    SwsContext *mSwsContext = NULL; // 格式转换上下文
    int index_video = -1;      // 视频
    RGBPacketQueue yuv_queue;  // YUV数据链表
    AVFrame *pYUVFrame = NULL;
    AVFrame *pRGBFrame = NULL;
    AVPacket *pVPacket = NULL;
    uint8_t *rgb_out_buffer = NULL; // RGB输出缓存
    int quit = 0;               // 退出标志

    int videoWidth = 0;
    int videoHeight = 0;
    AVPixelFormat rgbFormat;
} Decode_Video;

int createFFmpeg4Video(const char *url);
int readH264Packet();
int decodeVideo(uint8_t **data);
void releaseFFmpeg4Video();

// 声明一个全局变量
extern Decode_Video global_dVideo;

#ifdef __cplusplus
}
#endif

#endif //FFMPEG4ANDROID_FFMPEG_VIDEO_H
