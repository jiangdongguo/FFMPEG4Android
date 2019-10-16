package com.jiangdg.natives;

/**
 * @Auther: Jiangdg
 * @Date: 2019/9/23 17:56
 * @Description: 解析MP4，提取h264、aac保存到本地文件
 */
public class DemuxerUtil {
    static {
        System.loadLibrary("demuxermp4");
    }

    public interface OnInitCallback {
        void onCallback(int code);

        int INIT_INPUT_FAILED = -1;  // 初始化输入流失败
        int INIT_OUTPUT_FAILED = -2; // 初始化输出文件失败
        int INIT_SUCCESS = 0; // 初始化成功
        int START_DEMUXER = 1;   // 开始处理
        int FINISH_DEMUXER = 2;  // 结束处理
    }

    /** 开始解封装
     *
     * @param url URL地址
     * @param h264Path 存储H264文件
     */
    public native static int nativeStartDemuxer(String url, String h264Path, String aacPath, OnInitCallback callback);
    public native static void nativeStopDemuxer();
}
