// ffmpeg引擎核心代码
// Created by jianddongguo on 2018/12/3.
//

#include "ffmpeg_stream.h"

Decode_Stream global_stream;

int createFFmpegEngine_stream(const char *url) {
    global_stream.pPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    global_stream.pAFrame = av_frame_alloc();
    global_stream.pYUVFrame = av_frame_alloc();
    global_stream.pRGBFrame = av_frame_alloc();
    // 0.注册编解码器、muxers/demuxers等
    av_register_all();
    avformat_network_init();
    avcodec_register_all();

    // 1. 创建AVFormatContext，打开URL
    global_stream.pFmtCtx = avformat_alloc_context();
    if (global_stream.pFmtCtx == NULL) {
        return -1;
    }
    LOG_I("create ffmpeg for url = %s", url);

    AVDictionary *openOpts = NULL;                      // 优化
    av_dict_set(&openOpts, "stimeout", "15000000", 0);  // 15s超时连接断开
    av_dict_set(&openOpts, "buffer_size", "1024000", 0);// 减少码率变大导致花屏现象？
    av_dict_set(&openOpts,"rtsp_transport","tcp",0);    // 默认upd切换为tcp
    int ret = avformat_open_input(&global_stream.pFmtCtx, url, NULL, &openOpts);
    if (ret < 0) {
        LOGE("open input failed in PlayAudio,timesout.");
        releaseFFmpegEngine_stream();
        return -1;
    }

    // 2. 读取输入文件的数据包以获取流信息，比如码率、帧率等
    ret = avformat_find_stream_info(global_stream.pFmtCtx, NULL);
    if (ret < 0) {
        LOGE("find stream info failed in PlayAudio.");
        releaseFFmpegEngine_stream();
        return -1;
    }

    // 3. 获取音频轨道
    for (int i = 0; i < global_stream.pFmtCtx->nb_streams; i++) {
        if (global_stream.pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (global_stream.index_audio != -1) {
                continue;
            }
            global_stream.index_audio = i;
        } else if (global_stream.pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (global_stream.index_video != -1) {
                continue;
            }
            global_stream.index_video = i;
        }
    }
    if (global_stream.index_audio != -1 && global_stream.index_video == -1) {
        LOGE("url have no audio and video stream in PlayAudio.");
        releaseFFmpegEngine_stream();
        return -1;
    }
    // 4. 获取编码器上下文、打开音频解码器
    if (global_stream.index_audio != -1) {
        // --->根据AVCodecID查找相应的解码器
        AVCodecParameters *aCodecParameters = NULL;
        aCodecParameters = global_stream.pFmtCtx->streams[global_stream.index_audio]->codecpar;
        if (aCodecParameters == NULL) {
            LOGE("get audio codec's AVCodecParameters failed.");
            return -1;
        }
        global_stream.pACodec = avcodec_find_decoder(aCodecParameters->codec_id);
        if (!global_stream.pACodec) {
            LOG_E("do not find matched codec for %s", global_stream.pFmtCtx->audio_codec->name);
            releaseFFmpegEngine_stream();
            return -1;
        }
        // --->获取解码器对应的AVCodecContext，获取音频相关参数
        global_stream.pACodecCtx = avcodec_alloc_context3(global_stream.pACodec);
        if (!global_stream.pACodecCtx) {
            LOGE("alloc AVCodecContext failed..");
            return -1;
        }
        avcodec_parameters_to_context(global_stream.pACodecCtx, aCodecParameters);
        global_stream.channel_layout = global_stream.pACodecCtx->channel_layout;
        global_stream.sample_rate = global_stream.pACodecCtx->sample_rate;
        global_stream.channels = global_stream.pACodecCtx->channels;
        global_stream.sample_fmt = global_stream.pACodecCtx->sample_fmt;
        global_stream.profile = global_stream.pACodecCtx->profile;

        global_stream.swrContext = swr_alloc();                        // 设置参数
        swr_alloc_set_opts(global_stream.swrContext, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16,
                           global_stream.pACodecCtx->sample_rate,
                           global_stream.channel_layout,
                           global_stream.sample_fmt, global_stream.pACodecCtx->sample_rate, 0,
                           NULL);
        swr_init(global_stream.swrContext);

        // --->指定参数(如码率...)，开启解码器
        ret = avcodec_open2(global_stream.pACodecCtx, global_stream.pACodec, NULL);
        if (ret < 0) {
            LOGE("open %s audio codec failed.");
            releaseFFmpegEngine_stream();
            return -1;
        }
    }

    // 5. 获取编码器上下文、打开视频解码器
    if (global_stream.index_video != -1) {
        // --->根据AVCodecID查找相应的解码器
        AVCodecParameters *vCodecParameters = NULL;
        vCodecParameters = global_stream.pFmtCtx->streams[global_stream.index_video]->codecpar;
        if (vCodecParameters == NULL) {
            LOGE("get video codec's AVCodecParameters failed.");
            return -1;
        }
        global_stream.pVCodec = avcodec_find_decoder(vCodecParameters->codec_id);
        if (!global_stream.pVCodec) {
            LOGE("do not find matched codec");
            releaseFFmpegEngine_stream();
            return -1;
        }
        // --->获取解码器对应的AVCodecContext，获取视频相关参数
        global_stream.pVCodecCtx = avcodec_alloc_context3(global_stream.pVCodec);
        if (!global_stream.pVCodecCtx) {
            LOGE("alloc AVCodecContext failed..");
            return -1;
        }
        avcodec_parameters_to_context(global_stream.pVCodecCtx, vCodecParameters);

        // --->指定参数(如码率...)，开启解码器
        AVDictionary *opts = NULL;
        av_dict_set(&opts, "b", "2.5M", 0);
        ret = avcodec_open2(global_stream.pVCodecCtx, global_stream.pVCodec, &opts);
        if (ret < 0) {
            LOGE("open %s video codec failed.");
            releaseFFmpegEngine_stream();
            return -1;
        }

        // 初始化格式转码器
        int srcWidth = global_stream.pVCodecCtx->width;
        int srcHeight = global_stream.pVCodecCtx->height;
        int dstWidth = srcWidth;
        int dstHeight = srcHeight;
        AVPixelFormat srcFmt = global_stream.pVCodecCtx->pix_fmt;   // 原始格式
        AVPixelFormat dstFmt_rgb = AV_PIX_FMT_RGBA; // 目标格式为RGBA
        int flags = SWS_BICUBIC;                // 处理算法:Bicubic
        global_stream.mSwsContext = sws_getContext(srcWidth, srcHeight, srcFmt, dstWidth, dstHeight,
                                                   dstFmt_rgb, flags, NULL, NULL, NULL);

        global_stream.videoWidth = dstWidth;
        global_stream.videoHeight = dstHeight;
        global_stream.rgbFormat = dstFmt_rgb;
        // 计算存储一帧AV_PIX_FMT_RGBA格式图片所需空间(字节)
        //    分配内存,设置绑定数据源
        int rgbBufferSize = av_image_get_buffer_size(dstFmt_rgb, dstWidth, dstHeight, 1);
        global_stream.rgb_out_buffer = (uint8_t *) av_malloc(rgbBufferSize * sizeof(uint8_t));
        ret = av_image_fill_arrays(global_stream.pRGBFrame->data,     //最终将要被填充的数据指针
                                   global_stream.pRGBFrame->linesize, //图像数据指针每行大小
                                   global_stream.rgb_out_buffer,      //保存格式转换后rgb图像数据
                                   dstFmt_rgb,   // 目标图像格式
                                   dstWidth,
                                   dstHeight,
                                   1);
    }
    LOGI("#####create FFmpeg engine success!");
    return ret;
}

int readAVPacket_stream() {
    // pPacket为H264或aac/g711a
    int ret = av_read_frame(global_stream.pFmtCtx, global_stream.pPacket);
    if(ret == 0) {
        return global_stream.pPacket->stream_index;
    }
    return -1;
}

int decodeAudio_stream(uint8_t **data) {
    if (global_stream.pPacket->stream_index != global_stream.index_audio) {
        return -1;
    }

    // 支持音频压缩格式为AAC或G711a

    if(global_stream.pACodec->id == AV_CODEC_ID_AAC) {
        // 发送一个AVPacket(AAC)到解码器
        int ret = avcodec_send_packet(global_stream.pACodecCtx, global_stream.pPacket);    // AAC
        if (ret != 0) {
            return -1;
        }
        // 循环读取，获取一帧完整PCM音频数据
        while (avcodec_receive_frame(global_stream.pACodecCtx, global_stream.pAFrame) == 0) {
            LOG_D("读取一帧音频数据,frameSize=%d", global_stream.pAFrame->nb_samples);
            break;
        }
        // 重采样
        uint8_t *pcm_out_buffer = (uint8_t *) av_malloc(2 * global_stream.sample_rate);
        swr_convert(global_stream.swrContext, &pcm_out_buffer, global_stream.sample_rate * 2,
                    (const uint8_t **) global_stream.pAFrame->data,
                    global_stream.pAFrame->nb_samples);
        int pcm_outbuffer_size = av_samples_get_buffer_size(NULL, av_get_channel_layout_nb_channels(
                AV_CH_LAYOUT_STEREO),
                                                        global_stream.pAFrame->nb_samples,
                                                        AV_SAMPLE_FMT_S16P, 1);
        *data = (uint8_t *) malloc(pcm_outbuffer_size);     // PCM
        memset(*data, 0, pcm_outbuffer_size);
        memcpy(*data, pcm_out_buffer, pcm_outbuffer_size);
        free(pcm_out_buffer);
        // Wipe the AVFrame
        av_frame_unref(global_stream.pAFrame);
        // Wipe the AVPacket
        av_packet_unref(global_stream.pPacket);
        return pcm_outbuffer_size;
    } 
    return -1;
}

int decodeVideo_stream(uint8_t **data) {
    if (global_stream.pPacket->stream_index != global_stream.index_video) {
        return -1;
    }
    // 发送一个AVPacket(H.264)到解码器
    int ret = avcodec_send_packet(global_stream.pVCodecCtx, global_stream.pPacket);
    if (ret != 0) {
        return -1;
    }
    // 循环读取，获取一帧解码后的YUV数据，即pVFrame
    while (avcodec_receive_frame(global_stream.pVCodecCtx, global_stream.pYUVFrame) == 0) {
        break;
    }
    // YUV转RGB
    int srcSliceY = 0;
    int srcSliceH = global_stream.pVCodecCtx->height;
    sws_scale(global_stream.mSwsContext,
              global_stream.pYUVFrame->data,     // src数据源指针
              global_stream.pYUVFrame->linesize,  // src一行数据大小
              srcSliceY,
              srcSliceH,
              global_stream.pRGBFrame->data,
              global_stream.pRGBFrame->linesize);
    // Wipe the AVFrame
    av_frame_unref(global_stream.pYUVFrame);
    // Wipe the AVPacket
    av_packet_unref(global_stream.pPacket);
    av_init_packet(global_stream.pPacket);
    return  global_stream.pRGBFrame->linesize[0];
}

void releaseFFmpegEngine_stream() {
    // AVFormatContext
    if (global_stream.pFmtCtx) {
        avformat_close_input(&global_stream.pFmtCtx);
        avformat_free_context(global_stream.pFmtCtx);
    }
    // 音频
    if (global_stream.pACodecCtx) {
        avcodec_free_context(&global_stream.pACodecCtx);
    }
    if (global_stream.pPacket) {
        av_free(global_stream.pPacket);
    }
    if (global_stream.pAFrame) {
        av_frame_free(&global_stream.pAFrame);
    }
    // 视频
    if (global_stream.pVCodecCtx) {
        avcodec_free_context(&global_stream.pVCodecCtx);
    }

    if (global_stream.pRGBFrame) {
        av_frame_free(&global_stream.pRGBFrame);
    }
    if (global_stream.pRGBFrame) {
        av_frame_free(&global_stream.pRGBFrame);
    }
    if(global_stream.rgb_out_buffer) {
        free(global_stream.rgb_out_buffer);
    }

    avformat_network_deinit();
}

int getSampleFormat_stream(AVSampleFormat sampleFormat) {
    if (AV_SAMPLE_FMT_U8 == sampleFormat) {
        return 8;
    } else if (AV_SAMPLE_FMT_S16 == sampleFormat) {
        return 16;
    }
}


