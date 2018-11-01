//
// Created by jiangdongguo on 2018/10/26.
// 将网络流(如rtsp、rtmp等)保存为文件(mp4、ts、flv等等)
//

#include "SaveStream.h"

AVFormatContext *inputContext;
AVFormatContext *outputContext;
AVPacket *packet;

void closeOutput() {
    if (outputContext) {
        for (int i = 0; i < outputContext->nb_streams; ++i) {
            avcodec_close(outputContext->streams[i]->codec);
        }
        avformat_free_context(outputContext);
    }
}

void closeInput() {
    if (inputContext != NULL) {
        avformat_close_input(&inputContext);
    }
}

void initFFmpeg() {
    // 注册所有编解码器、muxer和demuxer等
    av_register_all();
    // 初始化filter system
    avfilter_register_all();
    // 网络组件全局初始化
    avformat_network_init();
    // 设置log等级
    av_log_set_level(AV_LOG_ERROR);
    // 创建一个AVPacket，并开辟一段空间
    packet = (AVPacket *) av_malloc(sizeof(AVPacket));
}

int openInput(const char *inputUrl) {
    // 第一步：创建广义输入文件的AVFormatContext，即为其分配内存
    inputContext = avformat_alloc_context();
    // 第二步：打开输入文件，将文件头部信息存储到AVFormatContext
    int ret = avformat_open_input(&inputContext, inputUrl, NULL, NULL);
    if (ret < 0) {
        LOGE("打开输入文件失败");
        closeInput();
        return ret;
    }
    // 第三步：读取输入文件的数据包以获取流信息，比如码率、帧率等
    ret = avformat_find_stream_info(inputContext, NULL);
    if (ret < 0) {
        LOGE("查询输入文件流信息失败");
        closeInput();
        return ret;
    }
    LOGI("打开文件成功");
    return ret;
}

int openOutput(const char *outUrl) {
    // 第一步:为广义输出文件分配一个AVFortmatContext结构体
    int ret = avformat_alloc_output_context2(&outputContext, NULL, "mpegts", outUrl);
    if (ret < 0) {
        LOGE("为输出上下文(outputContext)分配内存失败");
        return ret;
    }
    // 第二步：创建并初始化一个AVIOContext，以访问outUrl指定的资源
    ret = avio_open2(&outputContext->pb, outUrl, AVIO_FLAG_WRITE, NULL, NULL);
    if (ret < 0) {
        LOGE("打开avio失败");
        closeOutput();
        return ret;
    }
    // 第三步：根据inputContext的流信息，创建新流添加到outputContext
    int inputStreamNb = inputContext->nb_streams;
    for (int i = 0; i < inputStreamNb; i++) {
        AVCodecContext *inputCodecContext = inputContext->streams[i]->codec;
        const AVCodec *inputCodec = inputCodecContext->codec;

        AVStream *avStream = avformat_new_stream(outputContext, inputCodec);
        AVCodecContext *outputCodecContext = avStream->codec;
        ret = avcodec_copy_context(outputCodecContext, inputCodecContext);
        if (ret < 0) {
            LOGE("copy codec context failed");
            closeOutput();
            return ret;
        }
    }
    // 第四步：为流的私有数据分配内存，将流头部信息写入到输出文件中
    ret = avformat_write_header(outputContext, NULL);
    if (ret < 0) {
        LOGE("format write header failed");
        closeOutput();
        return ret;
    }
    return ret;
}

AVPacket *readPacketFromSource() {
    if(packet == NULL) {
        return NULL;
    }
    av_init_packet(packet);
    // 获取inputContext一帧数据，存储到AVPacket
    int ret = av_read_frame(inputContext, packet);
    if (ret < 0) {
        return NULL;
    }
    return packet;
}

int writePacket(AVPacket *packet) {
    // 同步时间
    AVStream *inputStream = inputContext->streams[packet->stream_index];
    AVStream *outputStream = outputContext->streams[packet->stream_index];
    av_packet_rescale_ts(packet, inputStream->time_base, outputStream->time_base);
    // 写一个数据包到输出媒体文件
    return av_interleaved_write_frame(outputContext, packet);
}

void releaseFFmpeg() {
    closeInput();
    closeOutput();
    // 释放内存
    av_free(packet);
    avformat_network_deinit();
}
