// 调用ffpmeg
//  错误码对照表 https://my.oschina.net/u/3700450/blog/1545657
// Created by Jiangdg on 2019/9/17.
//

#include "ffmpeg_save.h"

FFmpegSaveStream g_save;

/**
 *  注册所有muxer、demuxer、protocol
 *  和编解码器
 */
void initFFmpeg() {
    av_register_all();
    avcodec_register_all();
    av_log_set_level(AV_LOG_VERBOSE);
    g_save.avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
}

/**
 *  打开网络流
 *
 *  @params url url地址
 *  @return >=0 成功
 */
int openInput(char *input_url){
    MLOG_I_("#### open url = %s", input_url);
    if(! input_url) {
        MLOG_E("#### input url is null in openInput function.");
        return -100;
    }
    g_save.inputCtx = avformat_alloc_context();
    if(! g_save.inputCtx) {
        MLOG_E("#### alloc input AVFormatContext failed.");
        return -99;
    }
    AVDictionary *opts = NULL;
    av_dict_set(&opts, "rtsp_transport","tcp", 0); //设置tcp or udp，默认一般优先tcp再尝试udp
    av_dict_set(&opts, "stimeout", "3000000", 0);  //设置超时3秒
    int ret = avformat_open_input(&g_save.inputCtx, input_url, NULL, &opts);
    if(ret < 0) {
        MLOG_E_("#### open input url failed,err=%d(timesout?)", ret);
        return ret;
    }
    ret = avformat_find_stream_info(g_save.inputCtx, NULL);
    if(ret < 0) {
        MLOG_E_("#### find stream failed,err=%d", ret);
        return ret;
    }
    return ret;
}

/**
 *  打开输出文件
 *
 *  @params out out输出文件路径
 *  @return >=0 成功
 */
int openOutput(char *out){
    MLOG_I_("#### open output file = %s", out);
    int ret = avformat_alloc_output_context2(&g_save.outputCtx, NULL, "mpegts", out);
    if(ret < 0) {
        MLOG_E_("#### Allocate an AVFormatContext for an output format failed,err=%d", ret);
        closeOutput();
        return ret;
    }
    // 创建并初始化一个AVIOContext,用于访问url资源
    // app需要给存储权限，否则ret=-13
    ret = avio_open2(&g_save.outputCtx->pb, out, AVIO_FLAG_WRITE, NULL, NULL);
    if(ret < 0) {
        MLOG_E_("#### Create and initialize a AVIOContext failed,err=%d", ret);
        closeOutput();
        return ret;
    }
    // 根据inputCtx，为输出文件创建流
    // 获取每个流的编码器信息，为输出流复制一份
    int num_streams = g_save.inputCtx->nb_streams;
    for(int i = 0; i < num_streams; i++)
    {
        AVStream * stream = avformat_new_stream(g_save.outputCtx, g_save.inputCtx->streams[i]->codec->codec);
        ret = avcodec_copy_context(stream->codec, g_save.inputCtx->streams[i]->codec);
        if(ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "copy coddec context failed");
        }
    }
//    for(int i=0; i<num_streams; i++) {
//        AVCodecParameters *inAVCodecParameters = g_save.inputCtx->streams[i]->codecpar;
//        AVCodec *inputCodec = avcodec_find_encoder(inAVCodecParameters->codec_id);
//        AVStream *outStream = avformat_new_stream(g_save.outputCtx, inputCodec);
//        if(! outStream) {
//            closeOutput();
//            MLOG_E("#### create streams for output failed");
//            return -99;
//        }
//        AVCodec *outputCodec = avcodec_find_encoder(outStream->codecpar->codec_id);
//        AVCodecContext *outCodecCtx = avcodec_alloc_context3(outputCodec);
//        ret = avcodec_parameters_to_context(outCodecCtx, inAVCodecParameters);
//        if(ret < 0) {
//            closeOutput();
//            MLOG_E_("#### copy input parms to outCodecCtx failed,err=%d", ret);
//            return ret;
//        }
//    }
    // 为流的private data分配空间
    // 并将stream header写到输出文件中
    ret = avformat_write_header(g_save.outputCtx, NULL);
    if(ret < 0) {
        MLOG_E_("#### write the stream header to"
                " an output media file failed,err=%d", ret);
        return ret;
    }
    return ret;
}

void closeInput() {
    if(g_save.inputCtx) {
        avformat_close_input(&g_save.inputCtx);
        avformat_free_context(g_save.inputCtx);
    }
}

void closeOutput() {
    if(g_save.outputCtx) {
        for(int i = 0 ; i < g_save.outputCtx->nb_streams; i++) {
            AVStream * avStream = g_save.outputCtx->streams[i];
            if(avStream) {
                AVCodecContext *codecContext = avStream->codec;
                avcodec_close(codecContext);
            }
        }
        avformat_close_input(&g_save.outputCtx);
        avformat_free_context(g_save.outputCtx);
    }
}

int readAvPacketFromInput(){
    if(! g_save.avPacket) {
        return -99;
    }
    av_init_packet(g_save.avPacket);
    int ret = av_read_frame(g_save.inputCtx, g_save.avPacket);
    if(ret < 0) {
        MLOG_I("#### read frame error or end of file");
        return ret;
    }
    MLOG_I("----->read a frame");
    return ret;
}

int writeAvPacketToOutput() {
    int ret = -99;
    if(! g_save.avPacket) {
        return ret;
    }
    AVStream *inputStream = g_save.inputCtx->streams[g_save.avPacket->stream_index];
    AVStream *outputStream = g_save.outputCtx->streams[g_save.avPacket->stream_index];
    if(inputStream && outputStream) {
        // 处理同步
        av_packet_rescale_ts(g_save.avPacket, inputStream->time_base, outputStream->time_base);
        // 写入数据到输出文件
        ret = av_interleaved_write_frame(g_save.outputCtx, g_save.avPacket);
        if(ret < 0) {
            MLOG_E_("#### write a packet to an output media file failed,err=%d", ret);
            return ret;
        }
    }
    MLOG_I("----->write a frame");
    return ret;
}

void releaseFFmpeg(){
    closeOutput();
    closeInput();
    if(g_save.avPacket) {
        av_packet_unref(g_save.avPacket);
    }
}
