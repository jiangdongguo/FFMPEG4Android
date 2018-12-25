package com.jiangdg.natives;

import android.view.Surface;

/**使用ffmpeg解析rtsp数据流
 * Created by jiangdongguo on 2018/7/27.
 */

public class NativeFFmpeg {
    static {
        System.loadLibrary("avstream");
    }

    private static NativeFFmpeg instance;

    private NativeFFmpeg(){}

    public static NativeFFmpeg getInstance() {
        if(instance == null) {
            synchronized (NativeFFmpeg.class) {
                if(instance == null) {
                    instance = new NativeFFmpeg();
                }
            }
        }
        return instance;
    }

    //------------------------------ Lsn01：FFmpeg+OpenSL ES播放音频--------------------------
    public native int nativeStartPlayAudio(String url);
    public native void nativeStopPlayAudio();

    //------------------------------ Lsn02：FFmpeg+NativeWindow播放视频(无音频)----------------
    public native int nativeStartPlayVideo(String url,Surface surface);
    public native void nativeStopPlayVideo();

    //------------------------------ Lsn03：FFmpeg+NativeWindow播放视频(无音频)----------------
    public native int nativeStartPlayMedia(String url,Surface surface);
    public native void nativeStopPlayMedia();

    //------------------------------ Lsn04：保存网络流----------------
    public native int nativeStartSaveStream(String url,String outPath);
    public native void nativeStopSaveStream();
}
