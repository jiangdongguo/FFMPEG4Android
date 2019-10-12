package com.jiangdg.nativewindow;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.jiangdg.natives.RenderUtil;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback {
    private static final String URL = "rtmp://media3.sinovision.net:1935/live/livestream";
    private SurfaceView mSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mSurfaceView = findViewById(R.id.surfaceview_render);
        mSurfaceView.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        RenderUtil.nativeStartRender(URL, holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        RenderUtil.nativeStopRender();
    }
}
