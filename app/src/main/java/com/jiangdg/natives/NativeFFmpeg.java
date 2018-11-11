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
        void onStreamAcquire(byte[] data,int len,long pts,int type);
    }

    public static native int openVideo(String url,Surface surface,OnStreamAcquireListener listener);
    public static native void stopVideo();

    // Save Stream，保存网络流
    public static native void init();
    public static native int saveStreamFile(String inputUrl,int type,String outputPath);
    public static native void release();

    // 初始化/释放资源
    public native int nativeInit();
    public native int nativeRelease();
    public native int openInputURL(String inputUrl);

    //------------------------------ Lesson01：保存网络流到文件--------------------------

    // saveStream
    // @param inputUrl 输入url路径
    // @parms type 保存文件格式
    // @parms outputPath 文件输出路径
    public native int saveStream(int type,String outputPath);

    //------------------------------ Lesson02：网络流转发，渲染Surface--------------------------
    public native int nativePausePlayer();
    public native int nativeResumePlayer();
    public native int nativeStopPlayer();
    public native int setSurface(Surface surface);
}
