package com.jiangdg.natives;

/**
 * @Auther: Jiangdg
 * @Date: 2019/9/16 17:36
 * @Description: 使用ffmpeg保存网络流到文件
 */
public class SaveStreamUtil {
    static {
        System.loadLibrary("savefile");
    }

    public interface OnInitCallBack {
        // C层调用
        void onResult(int code);

        int INIT_INPUT_FAILED = -1;  // 初始化输入流失败
        int INIT_OUTPUT_FAILED = -2; // 初始化输出文件失败
        int INIT_SUCCESS = 0; // 初始化成功
        int START_SAVE = 1;   // 开始处理
        int FINISH_SAVE = 2;  // 结束处理
    }

    /** 启动录制子线程
     *
     * @param url url地址
     * @param outPath 输出文件路径
     * @param callBack 结果回调接口
     * @return
     */
    public native static int nativeStart(String url, String outPath, OnInitCallBack callBack);
    public native static int nativeStop();
}
