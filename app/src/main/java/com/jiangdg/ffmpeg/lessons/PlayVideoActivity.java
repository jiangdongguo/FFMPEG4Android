package com.jiangdg.ffmpeg.lessons;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.jiangdg.ffmpeg.R;
import com.jiangdg.natives.NativeFFmpeg;

public class PlayVideoActivity extends AppCompatActivity implements SurfaceHolder.Callback {
    public static final String URL = "rtsp://184.72.239.149/vod/mp4:BigBuckBunny_115k.mov";
    private SurfaceView mSurfaceView;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_play_video);

        mSurfaceView = findViewById(R.id.surface_play_video);
        mSurfaceView.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        NativeFFmpeg.getInstance().nativeStartPlayVideo(URL,holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        NativeFFmpeg.getInstance().nativeStopPlayVideo();
    }
}
