package com.jiangdg.ffmpeg;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView tvInfo = (TextView)findViewById(R.id.tv_info);
        tvInfo.setText("角度："+getRotate());
    }

    private int getRotate() {
        return VideoFixUtils.getVideoAngle(Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator +"test_ffmpeg.mp4");
    }
}
