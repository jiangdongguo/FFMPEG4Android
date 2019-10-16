//
// Created by Jiangdg on 2019/9/25.
//

#ifndef FFMPEG4ANDROID_FFMPEG_RENDER_H
#define FFMPEG4ANDROID_FFMPEG_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "native_demuxer.h"
#include "include/ffmpeg.h"

struct FFmpegDexmuer {
    AVFormatContext *inputFormatCtx  = NULL;

    AVPacket *avPacket  = NULL;
    int id_video_stream = -1;
    int id_audio_stream = -1;
    AVBSFContext *avBSFContext = NULL;
};

int createDemuxerFFmpeg(char * url);
int readDataFromAVPacket();
int handlePacketData(uint8_t *out, int size);
void releaseDemuxerFFmpeg();
int getVideoStreamIndex();
int getAudioStreamIndex();
int getAudioSampleRateIndex();
int getAudioProfile();
int getAudioChannels();

extern FFmpegDexmuer g_demuxer;

#ifdef __cplusplus
};
#endif
#endif //FFMPEG4ANDROID_FFMPEG_RENDER_H
