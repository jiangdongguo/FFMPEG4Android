//
// Created by Administrator on 2018/12/18.
//

#include "ffmpeg_save.h"

Save_Stream global_save;

void initFFmpeg() {
    av_register_all();
    avfilter_register_all();  // 注册所有编解码器、muxer/demuxer等
    avformat_network_init();  // 初始化网络
    av_log_set_level(AV_LOG_ERROR);       // 设置日志等级
    global_save.pAvPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
}

int openInputContext(const char *inputUrl) {
    // 1. 为广义输入文件创建AVFormatContext
    global_save.mInputFmt = avformat_alloc_context();
    if (!global_save.mInputFmt) {
        LOGE("alloc input format context failed.");
        closeInputContext();
        return -1;
    }

    // 2. 打开输入文件，填充AVFormatContext数据
    int ret = avformat_open_input(&global_save.mInputFmt, inputUrl, NULL, NULL);
    if (ret < 0) {
        LOGE("open input failed.");
        closeInputContext();
        return -1;
    }
    // 3. 获取流信息
    ret = avformat_find_stream_info(global_save.mInputFmt, NULL);
    if (ret < 0) {
        LOGE("find stream info failed in openInputContext()");

        return -1;
    }
    LOGI("open input format success!");
    return 0;
}

int openOutputContext(const char *outputUrl) {
    // 1. 为广义输出文件分配一个AVFortmatContext结构体
    int ret = avformat_alloc_output_context2(&global_save.mOutputFmt, NULL, "mpegts", outputUrl);
    if (ret < 0) {
        LOGE("为输出上下文(outputContext)分配内存失败");
        return ret;
    }
    // 2. 创建并初始化一个AVIOContext，以访问outUrl指定的资源
    ret = avio_open2(&global_save.mOutputFmt->pb, outputUrl, AVIO_FLAG_WRITE, NULL, NULL);

    if (ret < 0) {
        LOGE("打开avio失败");
        closeOutputContext();
        return ret;
    }

    // 3. 根据inputContext的流信息，创建新流添加到outputContext
    int inputStreamNb = global_save.mOutputFmt->nb_streams;
    for (int i = 0; i < inputStreamNb; i++) {

        AVCodecParameters *avCodecParams = global_save.mOutputFmt->streams[i]->codecpar;
        AVCodec *inputCodec = avcodec_find_encoder(avCodecParams->codec_id);

        AVStream *avStream = avformat_new_stream(global_save.mOutputFmt, inputCodec);
        AVCodec *outputCodec = avcodec_find_encoder(avStream->codecpar->codec_id);
        AVCodecContext *outputCodecContext = avcodec_alloc_context3(outputCodec);

        ret = avcodec_parameters_to_context(outputCodecContext,avCodecParams);
        if (ret < 0) {
            LOGE("copy codec context failed");
            closeOutputContext();
            return ret;
        }
    }
    // 4. 为流的私有数据分配内存，将流头部信息写入到输出文件中
    ret = avformat_write_header(global_save.mOutputFmt, NULL);
    if (ret < 0) {
        LOGE("format write header failed");
        closeOutputContext();
        return ret;
    }
    return ret;
}

void closeInputContext() {
    if (global_save.mInputFmt) {
        avformat_close_input(&global_save.mInputFmt);
        avformat_free_context(global_save.mInputFmt);
    }
}

AVPacket *readAVPacketFromInput() {
    if (!global_save.pAvPacket) {
        return NULL;
    }
    av_packet_unref(global_save.pAvPacket);
    av_init_packet(global_save.pAvPacket);
    int ret = av_read_frame(global_save.mInputFmt, global_save.pAvPacket);
    if (ret < 0) {
        return NULL;
    }
    LOGI("read a AVPacket From Input");
    return global_save.pAvPacket;
}

int writeAVPacket(AVPacket *pkt) {
    if (!pkt) {
        LOGI("write a AVPacket to output failed.");
        return -1;
    }
    // 获取input、output的AVStream
    AVStream *inputAVStream = global_save.mInputFmt->streams[pkt->stream_index];
    AVStream *outputAVStream = global_save.mOutputFmt->streams[pkt->stream_index];
    // 同步时间
    av_packet_rescale_ts(pkt, inputAVStream->time_base, outputAVStream->time_base);
    // 写入数据到output
    int ret = av_interleaved_write_frame(global_save.mOutputFmt,pkt);
    if(ret < 0) {
        LOGI("write a AVPacket to output failed.");
        return -1;
    }
    LOGI("write a AVPacket to output");
    return ret;
}

void closeOutputContext() {
    if (global_save.mOutputFmt) {
        avformat_close_input(&global_save.mOutputFmt);
        avformat_free_context(global_save.mOutputFmt);
    }
}

void releaseFFmpeg() {
    closeInputContext();
    closeOutputContext();
    avformat_network_deinit();
    if (global_save.pAvPacket) {
        free(global_save.pAvPacket);
    }
}

