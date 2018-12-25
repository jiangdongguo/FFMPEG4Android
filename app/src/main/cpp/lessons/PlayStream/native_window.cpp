// Native window驱动模块
// Created by jianddongguo on 2018/12/13.
//

#include "native_window.h"

// 定义变量
Render_Window_Stream global_nwindow_stream;

int createANativeWinidow_stream(JNIEnv* env, jobject surface) {
    global_nwindow_stream.mANativeWindow = ANativeWindow_fromSurface(env,surface);
    if(! global_nwindow_stream.mANativeWindow) {
        LOGE("##### create native window failed.");
        return -1;
    }
    LOGI("##### create native window success.");
    return 0;
}

void changeBuffersGeometry_stream(int32_t width, int32_t height) {
    if(! global_nwindow_stream.mANativeWindow) {
        LOGE("##### please create a native window before calling changeBuffersGeometry().");
        return;
    }
    global_nwindow_stream.width = width;
    global_nwindow_stream.height = height;
    ANativeWindow_setBuffersGeometry(global_nwindow_stream.mANativeWindow,width,height,WINDOW_FORMAT_RGBA_8888);
    LOGI("##### change Buffers Geometry success.");
}

void renderWindow_stream(uint8_t *data,int strideSize) {
    if(! global_nwindow_stream.mANativeWindow) {
        LOGE("##### please create a native window before calling renderWindow().");
        return;
    }
    // 逐行复制
    // 因为Window的stride与RGB的stride不一致
   int ret = ANativeWindow_lock(global_nwindow_stream.mANativeWindow,&global_nwindow_stream.mWindowBuffer,NULL);
   if(ret<0) {
       LOGE("##### lock window failed..");
       return;
   }
   uint8_t *dst = (uint8_t *)global_nwindow_stream.mWindowBuffer.bits;       // The actual bits
    int dstStride = global_nwindow_stream.mWindowBuffer.stride * 4;          // Window缓冲区一行在内存中所占字节数
    int srcStride = strideSize;                    // rgb图像缓冲区一行在内存中所占字节数
    for(int h=0 ; h<global_nwindow_stream.height ; h++) {
        memcpy(dst+h*dstStride,data+h*srcStride,strideSize);
    }
   ANativeWindow_unlockAndPost(global_nwindow_stream.mANativeWindow);
}

void destoryANativeWinidow_stream() {
    if(! global_nwindow_stream.mANativeWindow) {
        return;
    }
    ANativeWindow_release(global_nwindow_stream.mANativeWindow);
}