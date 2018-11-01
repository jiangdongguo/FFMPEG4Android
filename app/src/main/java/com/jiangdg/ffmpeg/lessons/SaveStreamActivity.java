package com.jiangdg.ffmpeg.lessons;

import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.jiangdg.ffmpeg.R;
import com.jiangdg.natives.NativeFFmpeg;

/** 保存网络流(rtsp/rtmp)。注：需要给予存储权限
 * Created by jiangdongguo on 2018/10/29.
 */

public class SaveStreamActivity extends AppCompatActivity {
    public static final String INPUT_URL = "rtsp://184.72.239.149/vod/mp4:BigBuckBunny_115k.mov";
    public static final String OUTPUT_PATH = Environment.getExternalStorageDirectory().getAbsolutePath()+"/666666.mp4";
    private boolean isStop = false;
    private int i;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_save_stream);
        final Button btnSave =  findViewById(R.id.btn_save_stream);


        btnSave.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(! isStop) {
                    btnSave.setText("停止保存网络流");
                    // 初始化FFmpeg引擎
                    NativeFFmpeg.init();

                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            NativeFFmpeg.saveStreamFile(INPUT_URL,0,OUTPUT_PATH);
                        }
                    }).start();
                } else {
                    // 释放FFmpeg引擎
                    NativeFFmpeg.release();
                    btnSave.setText("保存网络流");
                }
                isStop = ! isStop;
            }
        });

    }

}
