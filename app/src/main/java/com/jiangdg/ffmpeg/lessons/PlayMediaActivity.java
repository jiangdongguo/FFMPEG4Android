package com.jiangdg.ffmpeg.lessons;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.jiangdg.ffmpeg.R;
import com.jiangdg.natives.NativeFFmpeg;


public class PlayMediaActivity extends AppCompatActivity implements SurfaceHolder.Callback {
    private static final String URL_PATH = "rtsp://184.72.239.149/vod/mp4:BigBuckBunny_115k.mov";
    private SurfaceView mSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_draw_stream);
        mSurfaceView = (SurfaceView) findViewById(R.id.surfaceview_playmedia);
        mSurfaceView.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(final SurfaceHolder holder) {
        NativeFFmpeg.getInstance().nativeStartPlayMedia(URL_PATH,holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        NativeFFmpeg.getInstance().nativeStopPlayMedia();
    }
}
