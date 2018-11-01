package com.jiangdg.ffmpeg.lessons.utils;

import android.content.Context;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**自定义SurfaceView控件，渲染RGB
 * Created by jiangdongguo on 2018/8/7.
 */

public class VideoSurface extends SurfaceView implements SurfaceHolder.Callback{

    public VideoSurface(Context context) {
        super(context);
    }

    public VideoSurface(Context context, AttributeSet attrs) {
        super(context, attrs);
        getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
//        nativeSetSurface(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }


    public interface OnFrameListener {
        void onFrame();
    }

//    static {
//        System.loadLibrary("decstream");
//    }

//    private native void nativeSetSurface(Surface surface);
//
//    public native void nativePauseSurface();
//    public native void nativeResumeSurface();
//    public native void nativeStopSurface();
//    public native void setOnFrameListener(OnFrameListener listener);
}
