package com.jiangdg.natives;

/**使用ffmpeg+opensles在线播放音频数据
 * Created by jiangdongguo on 2019/9/14.
 */

public class PlayAudioUtil {
    static {
        System.loadLibrary("playaudio");
    }

    private static PlayAudioUtil instance;

    private PlayAudioUtil(){}

    public static PlayAudioUtil getInstance() {
        if(instance == null) {
            synchronized (PlayAudioUtil.class) {
                if(instance == null) {
                    instance = new PlayAudioUtil();
                }
            }
        }
        return instance;
    }

    public native int nativeStartPlayAudio(String url);
    public native void nativeStopPlayAudio();
}
