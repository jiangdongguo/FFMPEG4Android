package com.jiangdg.natives;

import android.view.Surface;

/**
 * @Auther: Jiangdg
 * @Date: 2019/9/23 17:56
 * @Description: 视频流渲染工具类
 */
public class RenderUtil {
    static {
        System.loadLibrary("rendstream");
    }

    interface OnInitCallback {
        void onCallback(int code);
    }

    /** 开始渲染视频流
     *
     * @param url URL地址
     * @param surface 渲染控件
     */
    public native static int nativeStartRender(String url, Surface surface);
    public native static void nativeStopRender();
}
