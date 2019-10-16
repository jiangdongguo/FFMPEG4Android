// ffmpeg调用功能函数
// Created by Jiangdg on 2019/9/25.
//

#include "ffmpeg_demuxer.h"

FFmpegDexmuer g_demuxer;

int createDemuxerFFmpeg(char * url) {
    if(! url) {
        RLOG_E("createRenderFFmpeg failed,url can not be null");
        return -1;
    }
    // 初始化ffmpeg引擎
    av_register_all();
    avcodec_register_all();
    av_log_set_level(AV_LOG_VERBOSE);
    g_demuxer.avPacket = av_packet_alloc();
    av_init_packet(g_demuxer.avPacket);
    g_demuxer.id_video_stream = -1;
    g_demuxer.id_audio_stream = -1;

    // 打开输入URL
    g_demuxer.inputFormatCtx = avformat_alloc_context();
    if(! g_demuxer.inputFormatCtx) {
        releaseDemuxerFFmpeg();
        RLOG_E("avformat_alloc_context failed.");
        return -1;
    }
    int ret = avformat_open_input(&g_demuxer.inputFormatCtx, url, NULL, NULL);
    if(ret < 0) {
        releaseDemuxerFFmpeg();
        RLOG_E_("avformat_open_input failed,err=%d", ret);
        return -1;
    }
    ret = avformat_find_stream_info(g_demuxer.inputFormatCtx, NULL);
    if(ret < 0) {
        releaseDemuxerFFmpeg();
        RLOG_E_("avformat_find_stream_info failed,err=%d", ret);
        return -1;
    }
    // 获取音视频stream id
    for(int i=0; i<g_demuxer.inputFormatCtx->nb_streams; i++) {
        AVStream *avStream = g_demuxer.inputFormatCtx->streams[i];
        if(! avStream) {
            continue;
        }
        AVMediaType type = avStream ->codecpar->codec_type;
        if(g_demuxer.id_video_stream == -1 || g_demuxer.id_audio_stream == -1) {
            if(type == AVMEDIA_TYPE_VIDEO) {
                g_demuxer.id_video_stream = i;
            }
            if(type == AVMEDIA_TYPE_AUDIO) {
                g_demuxer.id_audio_stream = i;
            }
        }

    }

    // 初始化h264_mp4toannexb过滤器
    // 该过滤器用于将H264的封装格式由mp4模式转换为annexb模式
    const AVBitStreamFilter *avBitStreamFilter = av_bsf_get_by_name("h264_mp4toannexb");
    if(! avBitStreamFilter) {
        releaseDemuxerFFmpeg();
        RLOG_E("get AVBitStreamFilter failed");
        return -1;
    }
    ret = av_bsf_alloc(avBitStreamFilter, &g_demuxer.avBSFContext);
    if(ret < 0) {
        releaseDemuxerFFmpeg();
        RLOG_E_("av_bsf_alloc failed,err = %d", ret);
        return ret;
    }
    ret = avcodec_parameters_copy(g_demuxer.avBSFContext->par_in,g_demuxer.inputFormatCtx->streams[g_demuxer.id_video_stream] ->codecpar);
    if(ret < 0) {
        releaseDemuxerFFmpeg();
        RLOG_E_("copy codec params to filter failed,err = %d", ret);
        return ret;
    }
    ret = av_bsf_init(g_demuxer.avBSFContext);
    if(ret < 0) {
        releaseDemuxerFFmpeg();
        RLOG_E_("Prepare the filter failed,err = %d", ret);
        return ret;
    }
    return ret;
}

int readDataFromAVPacket() {
    int ret = -1;
    // 成功，返回AVPacket数据大小
    if(g_demuxer.avPacket) {
        ret = av_read_frame(g_demuxer.inputFormatCtx, g_demuxer.avPacket);
        if(ret == 0) {
            return g_demuxer.avPacket->size;
        }
    }
    return ret;
}

int handlePacketData(uint8_t *out, int size) {
    if(!g_demuxer.avPacket || !out) {
        return -1;
    }
    // h264封装格式转换：mp4模式->annexb模式
    int stream_index = g_demuxer.avPacket->stream_index;
    if(stream_index == getVideoStreamIndex()) {
        int ret = av_bsf_send_packet(g_demuxer.avBSFContext, g_demuxer.avPacket);
        if(ret < 0) {
            av_packet_unref(g_demuxer.avPacket);
            av_init_packet(g_demuxer.avPacket);
            return ret;
        }

        for(;;) {
            int flags = av_bsf_receive_packet(g_demuxer.avBSFContext, g_demuxer.avPacket);
            if(flags == EAGAIN) {
                continue;
            } else {
                break;
            }
        }
        memcpy(out, g_demuxer.avPacket->data, size);
    } else if(stream_index == getAudioStreamIndex()){
        memcpy(out, g_demuxer.avPacket->data, size);
    }
    av_packet_unref(g_demuxer.avPacket);
    av_init_packet(g_demuxer.avPacket);
    // 返回AVPacket的数据类型
    return stream_index;
}

void releaseDemuxerFFmpeg() {
    if(g_demuxer.inputFormatCtx) {
        avformat_close_input(&g_demuxer.inputFormatCtx);
        avformat_free_context(g_demuxer.inputFormatCtx);
    }
    if(g_demuxer.avPacket) {
        av_packet_free(&g_demuxer.avPacket);
        g_demuxer.avPacket = NULL;
    }
    if(g_demuxer.avBSFContext) {
        av_bsf_free(&g_demuxer.avBSFContext);
    }
    RLOG_I("release FFmpeg engine over!");
}

int getVideoStreamIndex() {
    return g_demuxer.id_video_stream;
}

int getAudioStreamIndex() {
    return g_demuxer.id_audio_stream;
}

int getAudioSampleRateIndex() {
    int rates[] = {96000, 88200, 64000,48000, 44100,
                   32000, 24000, 22050, 16000, 12000,
                   11025, 8000, 7350, -1, -1, -1};
    int sampe_rate = g_demuxer.inputFormatCtx->streams[getAudioStreamIndex()]->codecpar->sample_rate;
    for (int index = 0; index < 16; index++) {
        if(sampe_rate == rates[index]) {
            return index;
        }
    }
    return -1;
}

int getAudioProfile() {
    return g_demuxer.inputFormatCtx->streams[getAudioStreamIndex()]->codecpar->profile;
}

int getAudioChannels() {
    return g_demuxer.inputFormatCtx->streams[getAudioStreamIndex()]->codecpar->channels;
}