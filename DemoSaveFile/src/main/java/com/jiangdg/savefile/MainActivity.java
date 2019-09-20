package com.jiangdg.savefile;

import android.media.MediaPlayer;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.jiangdg.natives.SaveStreamUtil;

import java.io.File;
import java.io.IOException;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback {
    private static final String URL = "rtmp://media3.sinovision.net:1935/live/livestream";

    private boolean isRecording;
    private SurfaceView mSurfaceView;
    private MediaPlayer mMediaPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mSurfaceView = findViewById(R.id.surface_play);
        mSurfaceView.getHolder().addCallback(this);
        final Button mBtnSave = findViewById(R.id.btn_save_stream);
        final TextView mTvTip = findViewById(R.id.tv_tip);
        mBtnSave.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(! isRecording) {
                    String outPath = Environment.getExternalStorageDirectory().getAbsolutePath()+File.separator
                            +"SaveFile"+System.currentTimeMillis()+".mp4";
                    final File file = new File(outPath);
                    SaveStreamUtil.nativeStart(URL, file.getAbsolutePath(), new SaveStreamUtil.OnInitCallBack() {
                        @Override
                        public void onResult(final int code) {
                            MainActivity.this.runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    switch (code) {
                                        case SaveStreamUtil.OnInitCallBack.INIT_INPUT_FAILED:
                                            mTvTip.setText("初始化输入流失败");
                                            if(file.exists()) {
                                                file.delete();
                                            }
                                            break;
                                        case SaveStreamUtil.OnInitCallBack.INIT_OUTPUT_FAILED:
                                            mTvTip.setText("初始化输出文件失败");
                                            if(file.exists()) {
                                                file.delete();
                                            }
                                            break;
                                        case SaveStreamUtil.OnInitCallBack.INIT_SUCCESS:
                                            mTvTip.setText("初始化成功");
                                            break;
                                        case SaveStreamUtil.OnInitCallBack.START_SAVE:
                                            mTvTip.setText("正在处理...");
                                            isRecording = true;
                                            mBtnSave.setText("停止录制");
                                            break;
                                        case SaveStreamUtil.OnInitCallBack.FINISH_SAVE:
                                            mTvTip.setText("处理完毕，文件地址："+file.getAbsolutePath());
                                            isRecording = false;
                                            mBtnSave.setText("开始录制");
                                            startPlayMp4(file.getAbsolutePath());
                                            break;
                                    }
                                }
                            });
                        }
                    });
                } else {
                    SaveStreamUtil.nativeStop();
                }

            }
        });
    }

    private void startPlayMp4(String filePath) {
        mMediaPlayer = new MediaPlayer();
        try {
            mMediaPlayer.setDisplay(mSurfaceView.getHolder());
            mMediaPlayer.setDataSource(filePath);
            mMediaPlayer.prepare();
            mMediaPlayer.start();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    private void stopPlayMp4() {
        if(mMediaPlayer != null) {
            mMediaPlayer.stop();
            mMediaPlayer.setDisplay(null);
            mMediaPlayer.release();
            mMediaPlayer = null;
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        stopPlayMp4();
    }
}
