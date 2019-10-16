package com.jiangdg.dmp4;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.jiangdg.natives.DemuxerUtil;

public class MainActivity extends AppCompatActivity{
    private static final String URL = "rtmp://media3.sinovision.net:1935/live/livestream";
    private static final String ROOT_PATH = Environment.getExternalStorageDirectory().getAbsolutePath();
//    private static final String URL = ROOT_PATH+"/1.mp4";
    private TextView mTvTip;
    private Button mBtnDemuxer;
    private boolean isDemuxing;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mTvTip = findViewById(R.id.tv_tip);
        mBtnDemuxer = findViewById(R.id.btn_demuxer);
        mBtnDemuxer.setText("开始解析MP4");
    }

    public void onDemuxerBtn(View view) {
        if(! isDemuxing) {
            isDemuxing = true;
            // 写文件注意要给APP权限
            String h264Path = ROOT_PATH + "/result666.264";
            String aacPath = ROOT_PATH + "/result888.aac";
            DemuxerUtil.nativeStartDemuxer(URL, h264Path, aacPath, new DemuxerUtil.OnInitCallback() {
                @Override
                public void onCallback(final int code) {
                    MainActivity.this.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if(code < 0) {
                                mTvTip.setText("初始化失败");
                                mBtnDemuxer.setText("开始解析MP4");
                                isDemuxing = false;
                            } else if(code == START_DEMUXER) {
                                mTvTip.setText("正在解析中...");
                            } else if(code == FINISH_DEMUXER){
                                mTvTip.setText("解析完毕");
                                mBtnDemuxer.setText("开始解析MP4");
                                isDemuxing = false;
                            }
                        }
                    });
                }
            });
            mBtnDemuxer.setText("停止解析MP4");
        } else {
            isDemuxing = false;
            DemuxerUtil.nativeStopDemuxer();
            mTvTip.setText("准备开始");
            mBtnDemuxer.setText("开始解析MP4");
        }
    }
}
