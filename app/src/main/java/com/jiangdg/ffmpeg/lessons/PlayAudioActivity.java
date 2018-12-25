package com.jiangdg.ffmpeg.lessons;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;

import com.jiangdg.ffmpeg.R;
import com.jiangdg.natives.NativeFFmpeg;

/** 使用FFmpeg实现AAC->PCM
 *  使用OpenSL ES播放PCM
 * Created by jianddongguo on 2018/12/4.
 */

public class PlayAudioActivity extends AppCompatActivity {
    public static final String URL = "rtsp://184.72.239.149/vod/mp4:BigBuckBunny_115k.mov";
    private Button mBtnPlayAudio;
    private boolean isPlaying = false;
    private NativeFFmpeg nativeFFmpeg;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_play_audio);

        nativeFFmpeg = NativeFFmpeg.getInstance();

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
