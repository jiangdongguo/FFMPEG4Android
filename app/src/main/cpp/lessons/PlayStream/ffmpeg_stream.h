// FFmpeg引擎使用头文件
//
// Created by jianddongguo on 2018/12/3.
//
#ifndef FFMPEG4ANDROID_FFMPEG_AUDIO_H
#define FFMPEG4ANDROID_FFMPEG_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ffmpeg.h"
#include "queue_stream.h"
#include <libavutil/imgutils.h>

typedef struct FFmpegAudio {
    AVFormatContext *pFmtCtx = NULL;  // 全局上下文
    AVCodecContext *pACodecCtx = NULL;// 编解码器上下文
    AVCodecContext *pVCodecCtx = NULL;
    AVCodec *pACodec = NULL;  // 音频编解码器
    AVCodec *pVCodec = NULL;  // 视频编解码器
    int index_audio = -1;      // 音频
    int index_video = -1;      // 视频
    PCMPacketQueue2 pcm_queue;  // PCM数据链表
    AVFrame *pAFrame;
    AVPacket *pPacket;
    SwrContext *swrContext = NULL;
    int quit = 0;               // 退出标志
    uint64_t channel_layout;
    int sample_rate;
    int channels;
    int profile;
    AVSampleFormat sample_fmt;

    SwsContext *mSwsContext = NULL; // 格式转换上下文
    RGBPacketQueue2 yuv_queue;  // YUV数据链表
    AVFrame *pYUVFrame = NULL;
    AVFrame *pRGBFrame = NULL;
    uint8_t *rgb_out_buffer = NULL; // RGB输出缓存

    int videoWidth = 0;
    int videoHeight = 0;
    AVPixelFormat rgbFormat;
} Decode_Stream;

int createFFmpegEngine_stream(const char *url);
int readAVPacket_stream();
int decodeAudio_stream(uint8_t **data);
int decodeVideo_stream(uint8_t **data);
void releaseFFmpegEngine_stream();

int getSampleFormat(AVSampleFormat sampleFormat);

// 声明一个全局变量
extern Decode_Stream global_stream;

#ifdef __cplusplus
}
#endif

#endif //FFMPEG4ANDROID_FFMPEG_AUDIO_H
