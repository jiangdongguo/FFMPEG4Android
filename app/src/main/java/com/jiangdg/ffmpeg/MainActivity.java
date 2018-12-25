package com.jiangdg.ffmpeg;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

import com.jiangdg.ffmpeg.lessons.PlayAudioActivity;
import com.jiangdg.ffmpeg.lessons.PlayMediaActivity;
import com.jiangdg.ffmpeg.lessons.PlayVideoActivity;
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

    public void onPlayAudio(View view) {
        startActivity(new Intent(MainActivity.this, PlayAudioActivity.class));
    }

    public void onPlayVideo(View view) {
        startActivity(new Intent(MainActivity.this, PlayVideoActivity.class));
    }

    public void onPlayMedia(View view) {
        startActivity(new Intent(MainActivity.this, PlayMediaActivity.class));
    }

    public void onSaveStream(View view) {
        startActivity(new Intent(MainActivity.this, SaveStreamActivity.class));
    }
}
