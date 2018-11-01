package com.jiangdg.ffmpeg;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

import com.jiangdg.ffmpeg.lessons.DrawStreamActivity;
import com.jiangdg.ffmpeg.lessons.SaveStreamActivity;

/**
 * Created by jiangdongguo on 2018/10/29.
 */

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void onSavingStream(View view) {
        startActivity(new Intent(MainActivity.this, SaveStreamActivity.class));
    }

    public void onDrawingStream(View view) {
        startActivity(new Intent(MainActivity.this, DrawStreamActivity.class));
    }
}
