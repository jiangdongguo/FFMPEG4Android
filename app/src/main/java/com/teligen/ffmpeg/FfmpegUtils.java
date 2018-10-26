package com.teligen.ffmpeg;

import android.view.Surface;

/**使用ffmpeg解析rtsp数据流
 * Created by jiangdongguo on 2018/7/27.
 */

public class FfmpegUtils {
    static {
        System.loadLibrary("decstream");
//        System.loadLibrary("avcodec-57");
//        System.loadLibrary("avdevice-57");
//        System.loadLibrary("avfilter-6");
//        System.loadLibrary("avformat-57");
//        System.loadLibrary("avutil-55");
//        System.loadLibrary("swscale-4");
//        System.loadLibrary("swresample-2");
//        System.loadLibrary("postproc-54");
    }

    public interface OnStreamAcquireListener {
        void onStreamAcquire(byte[] data,int len,int type);
    }

    public static native int openVideo(String url,Surface surface,OnStreamAcquireListener listener);
    public static native void stopVideo();
}
