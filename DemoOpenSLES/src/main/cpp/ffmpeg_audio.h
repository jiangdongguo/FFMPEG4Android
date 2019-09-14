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
#include "utils_audio.h"
#include "common.h"


typedef struct FFmpeg4Audio {
    AVFormatContext *pFmtCtx = NULL;  // 全局上下文
    AVCodecContext *pACodecCtx = NULL;// 编解码器上下文
    AVCodec *pACodec = NULL;  // 编解码器
    int index_audio = -1;      // 音频
    PCMPacketQueue pcm_queue;  // PCM数据链表
    AVFrame *pAFrame;
    AVPacket *pPacket;
    SwrContext *swrContext = NULL;
    int quit = 0;               // 退出标志
    // 音频参数
    uint64_t channel_layout;
    int sample_rate;
    int channels;
    AVSampleFormat sample_fmt;
} Decode_Audio;

int createFFmpegEngine(const char *url);
int readAVPacket();
int decodeAudio(uint8_t **data);
void releaseFFmpegEngine();

int getSampleFormat(AVSampleFormat sampleFormat);

// 声明一个全局变量
extern Decode_Audio global_dAudio;

#ifdef __cplusplus
}
#endif

#endif //FFMPEG4ANDROID_FFMPEG_AUDIO_H
