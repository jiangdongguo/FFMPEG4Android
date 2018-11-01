package com.jiangdg.natives;

import android.view.Surface;

/**使用ffmpeg解析rtsp数据流
 * Created by jiangdongguo on 2018/7/27.
 */

public class NativeFFmpeg {
    static {
        System.loadLibrary("avstream");
    }

    public interface OnStreamAcquireListener {
        void onStreamAcquire(byte[] data,int len,int type);
    }

    public static native int openVideo(String url,Surface surface,OnStreamAcquireListener listener);
    public static native void stopVideo();

    // Save Stream，保存网络流
    public static native void init();
    public static native int saveStreamFile(String inputUrl,int type,String outputPath);
    public static native void release();
}
