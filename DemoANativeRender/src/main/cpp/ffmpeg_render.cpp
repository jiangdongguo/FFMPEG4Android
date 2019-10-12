// ffmpeg调用功能函数
// Created by Jiangdg on 2019/9/25.
//

#include "ffmpeg_render.h"

FFmpegRender g_render;

int createRenderFFmpeg(char * url) {
    if(! url) {
        RLOG_E("createRenderFFmpeg failed,url can not be null");
        return -1;
    }
    RLOG_I_("---->input url = %s", url);
    // 初始化ffmpeg引擎
    av_register_all();
    avcodec_register_all();
    av_log_set_level(AV_LOG_VERBOSE);
    g_render.avPacket = av_packet_alloc();
    g_render.yuvFrame = av_frame_alloc();
    g_render.rgbFrame = av_frame_alloc();
    av_init_packet(g_render.avPacket);

    // 打开输入URL
    g_render.inputFormatCtx = avformat_alloc_context();
    if(! g_render.inputFormatCtx) {
        releaseRenderFFmpeg();
        RLOG_E("avformat_alloc_context failed.");
        return -1;
    }
    int ret = avformat_open_input(&g_render.inputFormatCtx, url, NULL, NULL);
    if(ret < 0) {
        releaseRenderFFmpeg();
        RLOG_E_("avformat_open_input failed,err=%d", ret);
        return -1;
    }
    ret = avformat_find_stream_info(g_render.inputFormatCtx, NULL);
    if(ret < 0) {
        releaseRenderFFmpeg();
        RLOG_E_("avformat_find_stream_info failed,err=%d", ret);
        return -1;
    }
    // 获取音视频stream id
    for(int i=0; i<g_render.inputFormatCtx->nb_streams; i++) {
        AVStream *avStream = g_render.inputFormatCtx->streams[i];
        if(! avStream) {
            continue;
        }
        AVMediaType type = avStream ->codecpar->codec_type;
        if(type == AVMEDIA_TYPE_VIDEO) {
            g_render.id_video_stream = i;
            break;
        }
    }
    // 获取解码器和创建上下文，打开编码器
    // 注意要调用avcodec_parameters_to_context进行参数copy
    AVCodecParameters *vCodecParams = g_render.inputFormatCtx->streams[g_render.id_video_stream]->codecpar;
    if(! vCodecParams) {
        releaseRenderFFmpeg();
        RLOG_E("get video codec parameters failed.");
        return -1;
    }
    AVCodec *vCodec = avcodec_find_decoder(vCodecParams->codec_id);
    g_render.vCodecCtx = avcodec_alloc_context3(vCodec);
    if(! g_render.vCodecCtx) {
        releaseRenderFFmpeg();
        RLOG_E("get video codec failed.");
        return -1;
    }
    ret = avcodec_parameters_to_context(g_render.vCodecCtx, vCodecParams);
    if(ret < 0) {
        releaseRenderFFmpeg();
        RLOG_E("fill the codec context failed.");
        return ret;
    }
    ret = avcodec_open2(g_render.vCodecCtx, g_render.vCodec, NULL);
    if(ret < 0) {
        releaseRenderFFmpeg();
        RLOG_E_("avcode_open2 failed,err=%d", ret);
        return ret;
    }
    // 初始化YUV->RGB转换器
    g_render.video_width = g_render.vCodecCtx->width;
    g_render.video_height = g_render.vCodecCtx->height;
    int srcWidth = g_render.video_width;
    int srcHeight = g_render.video_height;
    if(srcHeight==0 || srcWidth == 0) {
        releaseRenderFFmpeg();
        RLOG_E("get codec context info failed");
        return -1;
    }
    AVPixelFormat srcFormat = g_render.vCodecCtx->pix_fmt;
    int dstWidth = srcWidth;
    int dstHeight = srcHeight;
    AVPixelFormat dstFormat = AV_PIX_FMT_RGBA;
    int algorithm = SWS_BICUBIC;
    g_render.swsContext = sws_getContext(srcWidth, srcHeight,
                                         srcFormat,
                                         dstWidth, dstHeight,
                                         dstFormat,
                                         algorithm,
                                         NULL, NULL, NULL);
    if(! g_render.swsContext) {
        releaseRenderFFmpeg();
        RLOG_E("init SWSContext failed.");
        return -1;
    }
    // 计算一帧ARGB格式数据所占内存大小和行宽
    // 并赋值给rgbFrame的data和linesize字段
    int rgbBufferSize = av_image_get_buffer_size(dstFormat, dstWidth, dstHeight, 1);
    g_render.rgb_buffer = (uint8_t *) av_malloc(rgbBufferSize * sizeof(uint8_t));
    ret = av_image_fill_arrays(g_render.rgbFrame->data,     //最终将要被填充的数据指针
                               g_render.rgbFrame->linesize, //图像数据指针每行大小
                               g_render.rgb_buffer,      //保存格式转换后rgb图像数据
                               dstFormat,   // 目标图像格式
                               dstWidth,
                               dstHeight,
                               1);
    if(ret < 0) {
        releaseRenderFFmpeg();
        RLOG_E_("av_image_alloc failed,err = %d", ret);
        return ret;
    }
    RLOG_I("---->init FFmpeg engine success!");
    return ret;
}

int readH264DataFromAVPacket() {
    int ret = -1;
    if(g_render.avPacket) {
        ret = av_read_frame(g_render.inputFormatCtx, g_render.avPacket);
    }
    // ret = 0,成功
    return ret;
}

int decodeH264Data() {
    // 将H264送到编码器解码
    if(!g_render.vCodecCtx || !g_render.avPacket) {
        return -1;
    }
    int ret = avcodec_send_packet(g_render.vCodecCtx, g_render.avPacket);
    if(ret != 0) {
        return -1;
    }
    // 获取解码后的YUV，将其转换为RGB
    // 对于视频而言，一个AVPacket存储一帧图像
    ret = avcodec_receive_frame(g_render.vCodecCtx, g_render.yuvFrame);
    if(ret != 0) {
        return -1;
    }
    // 解码一帧数据，开始转换颜色格式
    int srcSliceY = 0;
    int srcSliceH = g_render.vCodecCtx->height;
    ret = sws_scale(g_render.swsContext,
                    g_render.yuvFrame->data,
                    g_render.yuvFrame->linesize,
                    srcSliceY, srcSliceH,
                    g_render.rgbFrame->data,
                    g_render.rgbFrame->linesize);
    if(ret < 0) {
        return -1;
    }
    av_packet_unref(g_render.avPacket);
    av_init_packet(g_render.avPacket);
    av_frame_unref(g_render.yuvFrame);
    return ret;
}

void releaseRenderFFmpeg() {
    if(g_render.inputFormatCtx) {
        avformat_close_input(&g_render.inputFormatCtx);
        avformat_free_context(g_render.inputFormatCtx);
    }
    if(g_render.vCodecCtx) {
        avcodec_close(g_render.vCodecCtx);
        avcodec_free_context(&g_render.vCodecCtx);
    }
    if(g_render.avPacket) {
        av_packet_free(&g_render.avPacket);
    }
    if(g_render.swsContext) {
        sws_freeContext(g_render.swsContext);
    }
    if(g_render.rgbFrame) {
        av_freep(&g_render.rgbFrame[0]);
        av_frame_free(&g_render.rgbFrame);
    }
    if(g_render.yuvFrame) {
        av_frame_free(&g_render.yuvFrame);
    }
    RLOG_I("release FFmpeg engine over!");
}

int getVideoWidth() {
    return g_render.video_width;
}

int getVideoHeight() {
    return g_render.video_height;
}