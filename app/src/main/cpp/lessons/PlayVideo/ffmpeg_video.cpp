// ffmpeg引擎核心代码
// Created by jianddongguo on 2018/12/13.
//

#include "ffmpeg_video.h"

// 定义全局变量
Decode_Video global_dVideo;

int createFFmpeg4Video(const char *url) {
    global_dVideo.pYUVFrame = av_frame_alloc();
    global_dVideo.pRGBFrame = av_frame_alloc();
    global_dVideo.pVPacket = (AVPacket *) av_malloc(sizeof(AVPacket));

    // 0.注册编解码器、muxers/demuxers等
    av_register_all();
    avformat_network_init();
    avcodec_register_all();

    // 1. 创建AVFormatContext，打开URL
    global_dVideo.pVFmtCtx = avformat_alloc_context();
    if (global_dVideo.pVFmtCtx == NULL) {
        return -1;
    }
    LOG_I("##### create ffmpeg for decode video, url = %s", url);

    AVDictionary *openOpts = NULL;
    av_dict_set(&openOpts, "stimeout", "15000000", 0);  // 15s超时连接断开
    av_dict_set(&openOpts, "buffer_size", "1024000", 0);// 减少码率变大导致花屏现象
    int ret = avformat_open_input(&global_dVideo.pVFmtCtx, url, NULL, &openOpts);
    if (ret < 0) {
        LOGE("open input failed in PlayVideo,timesout.");
        releaseFFmpeg4Video();
        return -1;
    }

    // 2. 读取输入文件的数据包以获取流信息，比如码率、帧率等
    ret = avformat_find_stream_info(global_dVideo.pVFmtCtx, NULL);
    if (ret < 0) {
        LOGE("find stream info failed in PlayVideo.");
        releaseFFmpeg4Video();
        return -1;
    }

    // 3. 获取视频轨道
    for (int i = 0; i < global_dVideo.pVFmtCtx->nb_streams; i++) {
        if (global_dVideo.pVFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            global_dVideo.index_video = i;
            break;
        }
    }
    // 4. 获取编码器上下文、打开解码器
    if (global_dVideo.index_video == -1) {
        LOGE("url have no video stream in PlayVideo.");
        releaseFFmpeg4Video();
        return -1;
    }
    // --->根据AVCodecID查找相应的解码器
    AVCodecParameters *avCodecParameters = NULL;
    avCodecParameters = global_dVideo.pVFmtCtx->streams[global_dVideo.index_video]->codecpar;
    if (avCodecParameters == NULL) {
        LOGE("get video codec's AVCodecParameters failed.");
        return -1;
    }
    global_dVideo.pVCodec = avcodec_find_decoder(avCodecParameters->codec_id);
    if (!global_dVideo.pVCodec) {
        LOG_E("do not find matched codec for %s", global_dVideo.pVFmtCtx->video_codec->name);
        releaseFFmpeg4Video();
        return -1;
    }
    // --->获取解码器对应的AVCodecContext，获取视频相关参数
    global_dVideo.pVCodecCtx = avcodec_alloc_context3(global_dVideo.pVCodec);
    if (!global_dVideo.pVCodecCtx) {
        LOGE("alloc AVCodecContext failed..");
        return -1;
    }
    avcodec_parameters_to_context(global_dVideo.pVCodecCtx, avCodecParameters);

    // --->指定参数(如码率...)，开启解码器
    AVDictionary *opts = NULL;
    av_dict_set(&opts, "b", "2.5M", 0);
    ret = avcodec_open2(global_dVideo.pVCodecCtx, global_dVideo.pVCodec, NULL);
    if (ret < 0) {
        LOG_E("open %s codec failed.", global_dVideo.pVFmtCtx->video_codec->name);
        releaseFFmpeg4Video();
        return -1;
    }

    // 5. 初始化格式转码器
    int srcWidth = global_dVideo.pVCodecCtx->width;
    int srcHeight = global_dVideo.pVCodecCtx->height;
    int dstWidth = srcWidth;
    int dstHeight = srcHeight;
    AVPixelFormat srcFmt = global_dVideo.pVCodecCtx->pix_fmt;   // 原始格式
    AVPixelFormat dstFmt_rgb = AV_PIX_FMT_RGBA; // 目标格式为RGBA
    int flags = SWS_BICUBIC;                // 处理算法:Bicubic
    global_dVideo.mSwsContext = sws_getContext(srcWidth,srcHeight,srcFmt,dstWidth,dstHeight,dstFmt_rgb,flags,NULL,NULL,NULL);

    global_dVideo.videoWidth = dstWidth;
    global_dVideo.videoHeight = dstHeight;
    global_dVideo.rgbFormat = dstFmt_rgb;
    // 6. 计算存储一帧AV_PIX_FMT_RGBA格式图片所需空间(字节)
    //    分配内存,设置绑定数据源
    int rgbBufferSize = av_image_get_buffer_size(dstFmt_rgb,dstWidth,dstHeight,1);
    global_dVideo.rgb_out_buffer = (uint8_t *)av_malloc(rgbBufferSize * sizeof(uint8_t));
    ret = av_image_fill_arrays(global_dVideo.pRGBFrame->data,     //最终将要被填充的数据指针
                         global_dVideo.pRGBFrame->linesize, //图像数据指针每行大小
                         global_dVideo.rgb_out_buffer,      //保存格式转换后rgb图像数据
                         dstFmt_rgb,   // 目标图像格式
                         dstWidth,
                         dstHeight,
                         1);
    if (ret < 0) {
        LOGE("av_image_fill_arrays failed in createFFmpeg()");
        releaseFFmpeg4Video();
        return -1;
    }
    LOG_I("#####create FFmpeg engine success,result = %d!",ret);
    return ret;
}

int readH264Packet() {
    if (global_dVideo.pVCodecCtx == NULL) {
        return -1;
    }
    return av_read_frame(global_dVideo.pVFmtCtx, global_dVideo.pVPacket);
}

int decodeVideo(uint8_t **data) {
    if (global_dVideo.pVPacket->stream_index != global_dVideo.index_video) {
        return -1;
    }
    // 发送一个AVPacket(H.264)到解码器
    int ret = avcodec_send_packet(global_dVideo.pVCodecCtx, global_dVideo.pVPacket);
    if (ret != 0) {
        return -1;
    }
    // 循环读取，获取一帧解码后的YUV数据，即pVFrame
    while (avcodec_receive_frame(global_dVideo.pVCodecCtx, global_dVideo.pYUVFrame) == 0) {
        LOGD("读取一帧视频数据");
        break;
    }
    // YUV转RGB
    int srcSliceY = 0;
    int srcSliceH = global_dVideo.pVCodecCtx->height;
    sws_scale(global_dVideo.mSwsContext,
              global_dVideo.pYUVFrame->data,     // src数据源指针
              global_dVideo.pYUVFrame->linesize,  // src一行数据大小
              srcSliceY,
              srcSliceH,
              global_dVideo.pRGBFrame->data,
              global_dVideo.pRGBFrame->linesize);

    // Wipe the AVFrame
    av_frame_unref(global_dVideo.pYUVFrame);
    // Wipe the AVPacket
    av_packet_unref(global_dVideo.pVPacket);
    av_init_packet(global_dVideo.pVPacket);
    return  global_dVideo.pRGBFrame->linesize[0];
}

void releaseFFmpeg4Video() {
    // 关闭所有流，释放AVFormatContext
    if (global_dVideo.pVFmtCtx) {
        avformat_close_input(&global_dVideo.pVFmtCtx);
        avformat_free_context(global_dVideo.pVFmtCtx);
    }
    // 释放AVCodecContext
    if (global_dVideo.pVCodecCtx) {
        avcodec_free_context(&global_dVideo.pVCodecCtx);
    }
    if (global_dVideo.pVPacket) {
        av_free(global_dVideo.pVPacket);
    }
    if (global_dVideo.pRGBFrame) {
        av_frame_free(&global_dVideo.pRGBFrame);
    }
    if (global_dVideo.pRGBFrame) {
        av_frame_free(&global_dVideo.pRGBFrame);
    }
    if(global_dVideo.rgb_out_buffer) {
        free(global_dVideo.rgb_out_buffer);
    }
    avformat_network_deinit();
}

