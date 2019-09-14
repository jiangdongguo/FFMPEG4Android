// ffmpeg引擎核心代码
// Created by jianddongguo on 2018/12/3.
//

#include "ffmpeg_audio.h"
#include "ffmpeg.h"

Decode_Audio global_dAudio;

int createFFmpegEngine(const char *url) {
    global_dAudio.pPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    global_dAudio.pAFrame = av_frame_alloc();
    // 0.注册编解码器、muxers/demuxers等
    av_register_all();
    avformat_network_init();
    avcodec_register_all();

    // 1. 创建AVFormatContext，打开URL
    global_dAudio.pFmtCtx = avformat_alloc_context();
    if (global_dAudio.pFmtCtx == NULL) {
        return -1;
    }
    M_LOG_I("create ffmpeg for url = %s", url);

    AVDictionary *openOpts = NULL;
    av_dict_set(&openOpts, "stimeout", "15000000", 0);  // 15s超时连接断开
    av_dict_set(&openOpts, "buffer_size", "1024000", 0);// 减少码率变大导致花屏现象
    int ret = avformat_open_input(&global_dAudio.pFmtCtx, url, NULL, &openOpts);
    if (ret < 0) {
        M_LOGE("open input failed in PlayAudio,timesout or other errors");
        releaseFFmpegEngine();
        return ret;
    }

    // 2. 读取输入文件的数据包以获取流信息，比如码率、帧率等
    ret = avformat_find_stream_info(global_dAudio.pFmtCtx, NULL);
    if (ret < 0) {
        M_LOGE("find stream info failed in PlayAudio.");
        releaseFFmpegEngine();
        return -1;
    }

    // 3. 获取音频轨道
    for (int i = 0; i < global_dAudio.pFmtCtx->nb_streams; i++) {
        if (global_dAudio.pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            global_dAudio.index_audio = i;
            break;
        }
    }
    // 4. 获取编码器上下文、打开解码器
    if (global_dAudio.index_audio == -1) {
        M_LOGE("url have no audio stream in PlayAudio.");
        releaseFFmpegEngine();
        return -1;
    }
    // --->根据AVCodecID查找相应的解码器
    AVCodecParameters *avCodecParameters = NULL;
    avCodecParameters = global_dAudio.pFmtCtx->streams[global_dAudio.index_audio]->codecpar;
    if (avCodecParameters == NULL) {
        M_LOGE("get audio codec's AVCodecParameters failed.");
        return -1;
    }
    global_dAudio.pACodec = avcodec_find_decoder(avCodecParameters->codec_id);
    if (!global_dAudio.pACodec) {
        M_LOG_E("do not find matched codec for %s", global_dAudio.pFmtCtx->audio_codec->name);
        releaseFFmpegEngine();
        return -1;
    }
    // --->获取解码器对应的AVCodecContext，获取音频相关参数
    global_dAudio.pACodecCtx = avcodec_alloc_context3(global_dAudio.pACodec);
    if (!global_dAudio.pACodecCtx) {
        M_LOGE("alloc AVCodecContext failed..");
        return -1;
    }
    avcodec_parameters_to_context(global_dAudio.pACodecCtx, avCodecParameters);
    global_dAudio.channel_layout = global_dAudio.pACodecCtx->channel_layout;
    global_dAudio.sample_rate = global_dAudio.pACodecCtx->sample_rate;
    global_dAudio.channels = global_dAudio.pACodecCtx->channels;
    global_dAudio.sample_fmt = global_dAudio.pACodecCtx->sample_fmt;

    global_dAudio.swrContext = swr_alloc();                        // 设置参数
    swr_alloc_set_opts(global_dAudio.swrContext, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, global_dAudio.pACodecCtx->sample_rate,
                       global_dAudio.channel_layout,
                       global_dAudio.sample_fmt, global_dAudio.pACodecCtx->sample_rate, 0, NULL);
    swr_init(global_dAudio.swrContext);

    // --->指定参数(如码率...)，开启解码器
//    AVDictionary *opts = NULL;
//    av_dict_set(&opts, "b", "2.5M", 0);
    ret = avcodec_open2(global_dAudio.pACodecCtx, global_dAudio.pACodec, NULL);
    if (ret < 0) {
        M_LOG_E("open %s codec failed.", global_dAudio.pFmtCtx->audio_codec->name);
        releaseFFmpegEngine();
        return -1;
    }
    M_LOGI("#####create FFmpeg engine success!");
    return ret;
}

int readAVPacket() {
    if (global_dAudio.pACodecCtx == NULL) {
        return -1;
    }
    return av_read_frame(global_dAudio.pFmtCtx, global_dAudio.pPacket);
}

int decodeAudio(uint8_t **data) {
    if (global_dAudio.pPacket->stream_index != global_dAudio.index_audio) {
        return -1;
    }
    // 发送一个AVPacket(AAC)到解码器
    int ret = avcodec_send_packet(global_dAudio.pACodecCtx, global_dAudio.pPacket);
    if (ret != 0) {
        return -1;
    }
    // 循环读取，获取一帧完整PCM音频数据
    while (avcodec_receive_frame(global_dAudio.pACodecCtx, global_dAudio.pAFrame) == 0) {
        M_LOG_I("读取一帧音频数据,frameSize=%d", global_dAudio.pAFrame->nb_samples);
        break;
    }
    // 重采样
    uint8_t *a_out_buffer = (uint8_t *) av_malloc(2 * global_dAudio.sample_rate);
    swr_convert(global_dAudio.swrContext, &a_out_buffer, global_dAudio.sample_rate * 2,
                (const uint8_t **) global_dAudio.pAFrame->data,
                global_dAudio.pAFrame->nb_samples);
    int outbuffer_size = av_samples_get_buffer_size(NULL, av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO),
                                                    global_dAudio.pAFrame->nb_samples,
                                                    AV_SAMPLE_FMT_S16P, 1);
    *data = (uint8_t *)malloc(outbuffer_size);
    memset(*data,0,outbuffer_size);
    memcpy(*data,a_out_buffer,outbuffer_size);
    free(a_out_buffer);
    // Wipe the AVFrame
    av_frame_unref(global_dAudio.pAFrame);
    // Wipe the AVPacket
    av_packet_unref(global_dAudio.pPacket);
    return outbuffer_size;
}

void releaseFFmpegEngine() {
    // 关闭所有流，释放AVFormatContext
    if (global_dAudio.pFmtCtx) {
        avformat_close_input(&global_dAudio.pFmtCtx);
        avformat_free_context(global_dAudio.pFmtCtx);
    }
    // 释放AVCodecContext
    if (global_dAudio.pACodecCtx) {
        avcodec_free_context(&global_dAudio.pACodecCtx);
    }
    if (global_dAudio.pPacket) {
        av_free(global_dAudio.pPacket);
    }
    if (global_dAudio.pAFrame) {
        av_frame_free(&global_dAudio.pAFrame);
    }
    if(global_dAudio.swrContext) {
        swr_free(&global_dAudio.swrContext);
    }
    avformat_network_deinit();
}

int getSampleFormat(AVSampleFormat sampleFormat) {
    if (AV_SAMPLE_FMT_U8 == sampleFormat) {
        return 8;
    } else if (AV_SAMPLE_FMT_S16 == sampleFormat) {
        return 16;
    }
    return 32;
}

