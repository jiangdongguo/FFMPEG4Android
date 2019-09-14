package com.jiangdg.opensles;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;

import com.jiangdg.natives.PlayAudioUtil;

/** 使用FFmpeg实现AAC->PCM
 *  使用OpenSL ES播放PCM
 * Created by jianddongguo on 2018/12/4.
 */

public class PlayAudioActivity extends AppCompatActivity {
    public static final String URL = "rtmp://media3.sinovision.net:1935/live/livestream";
    private Button mBtnPlayAudio;
    private boolean isPlaying = false;
    private PlayAudioUtil nativeFFmpeg;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_play_audio);

        nativeFFmpeg = PlayAudioUtil.getInstance();

        mBtnPlayAudio = (Button)findViewById(R.id.btn_play_audio);
        mBtnPlayAudio.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(! isPlaying) {
                    int ret = nativeFFmpeg.nativeStartPlayAudio(URL);
                    mBtnPlayAudio.setText("正在播放音频..."+ret);
                } else {
                    mBtnPlayAudio.setText("播放PCM音频");
                    nativeFFmpeg.nativeStopPlayAudio();
                }
                isPlaying = !isPlaying;
            }
        });

    }
}
