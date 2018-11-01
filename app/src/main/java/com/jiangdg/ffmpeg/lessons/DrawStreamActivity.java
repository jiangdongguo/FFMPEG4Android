package com.jiangdg.ffmpeg.lessons;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.widget.Toast;

import com.jiangdg.ffmpeg.R;
import com.jiangdg.natives.NativeFFmpeg;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class DrawStreamActivity extends AppCompatActivity implements SurfaceHolder.Callback {
//        private static final String URL_PATH = "rtsp://192.192.191.105:554/rtsp_live1";
    private static final String URL_PATH = "rtsp://184.72.239.149/vod/mp4:BigBuckBunny_115k.mov";
    //        private static final String URL_PATH = "rtsp://14.23.71.110:10554/152353460083592708.sdp";
    private SurfaceView mSurfaceView;
    private static final String PATH = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "result.264";
    private FileOutputStream os;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_draw_stream);
        mSurfaceView = (SurfaceView) findViewById(R.id.surface_view);
        mSurfaceView.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(final SurfaceHolder holder) {

        new Thread(new Runnable() {
            @Override
            public void run() {
                int result = NativeFFmpeg.openVideo(URL_PATH, holder.getSurface(), new NativeFFmpeg.OnStreamAcquireListener() {
                    @Override
                    public void onStreamAcquire(byte[] data, int len, int type) {
                        Log.d("DrawStreamActivity", "写入文件中....." + data.length);
                    }
                });

                if (result == -1) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(DrawStreamActivity.this, "解析rtsp流失败", Toast.LENGTH_SHORT).show();
                        }
                    });
                }
                Log.d("dddd", "读取结果:" + result);
            }
        }).start();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        NativeFFmpeg.stopVideo();
    }

    private void createFile() {
        File file = new File(PATH);
        if (file.exists()) {
            file.delete();
        }
        try {
            os = new FileOutputStream(file);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }

    private void writeFile(byte[] data) {
        if (os != null) {
            try {
                os.write(data);

            } catch (IOException e) {
                if (os != null) {
                    try {
                        os.close();
                        os = null;
                    } catch (IOException e1) {
                        e1.printStackTrace();
                    }
                }
                e.printStackTrace();
            }
        }
    }

    private void closeFile() {
        if (os != null) {
            try {
                os.close();
                os = null;
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
