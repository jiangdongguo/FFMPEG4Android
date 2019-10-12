// 调用ANativeWindow接口
// Created by Jiangdg on 2019/9/25.
//


#include "window_render.h"

RenderNativeWindow g_nativewindow;

int create_native_window(JNIEnv *env, jobject _surface) {
    g_nativewindow.nWindow = ANativeWindow_fromSurface(env, _surface);
    if(! g_nativewindow.nWindow) {
        RLOG_E("create native window failed");
        return -1;
    }
    ANativeWindow_acquire(g_nativewindow.nWindow);
    return 0;
}

int set_buffers_geometry(int width, int height) {
    if(! g_nativewindow.nWindow) {
        RLOG_E("set buffer geometry failed, as window is null");
        return -1;
    }
    g_nativewindow.width = width;
    g_nativewindow.height = height;
    return ANativeWindow_setBuffersGeometry(g_nativewindow.nWindow, width, height, WINDOW_FORMAT_RGBA_8888);
}

int render_window(uint8_t * src_data , int src_stride) {
    if(! g_nativewindow.nWindow || ! src_data || src_stride==0) {
        RLOG_E("render window failed,as window or src_data is null");
        return -1;
    }
    int ret = ANativeWindow_lock(g_nativewindow.nWindow, &g_nativewindow.outBuffer, NULL);
    if(ret < 0) {
        RLOG_E_("render window failed,err=%d", ret);
        return ret;
    }
    int8_t * dst_data = static_cast<int8_t *>(g_nativewindow.outBuffer.bits);
    int dst_stride = g_nativewindow.outBuffer.stride * 4;
    for(int h=0; h<g_nativewindow.height; h++) {
        memcpy(dst_data+h*dst_stride,src_data+h*src_stride, src_stride);
    }
    ANativeWindow_unlockAndPost(g_nativewindow.nWindow);
    return ret;
}

void destory_native_window() {
    if(g_nativewindow.nWindow) {
        ANativeWindow_release(g_nativewindow.nWindow);
    }
}