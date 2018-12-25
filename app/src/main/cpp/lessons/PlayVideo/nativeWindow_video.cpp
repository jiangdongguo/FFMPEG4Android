// Native window驱动模块
// Created by jianddongguo on 2018/12/13.
//

#include "nativeWindow_video.h"

// 定义变量
Render_Window global_nwindow;

int createANativeWinidow(JNIEnv* env, jobject surface) {
    global_nwindow.mANativeWindow = ANativeWindow_fromSurface(env,surface);
    if(! global_nwindow.mANativeWindow) {
        LOGE("##### create native window failed.");
        return -1;
    }
    return 0;
}

void changeBuffersGeometry(int32_t width, int32_t height) {
    if(! global_nwindow.mANativeWindow) {
        LOGE("##### please create a native window before calling changeBuffersGeometry().");
        return;
    }
    global_nwindow.width = width;
    global_nwindow.height = height;
    ANativeWindow_setBuffersGeometry(global_nwindow.mANativeWindow,width,height,WINDOW_FORMAT_RGBA_8888);
}

void renderWindow(uint8_t *data,int strideSize) {
    if(! global_nwindow.mANativeWindow) {
        LOGE("##### please create a native window before calling renderWindow().");
        return;
    }
    // 逐行复制
    // 因为Window的stride与RGB的stride不一致
   int ret = ANativeWindow_lock(global_nwindow.mANativeWindow,&global_nwindow.mWindowBuffer,NULL);
   if(ret<0) {
       LOGE("##### lock window failed..");
       return;
   }
   uint8_t *dst = (uint8_t *)global_nwindow.mWindowBuffer.bits;       // The actual bits
    int dstStride = global_nwindow.mWindowBuffer.stride * 4;          // Window缓冲区一行在内存中所占字节数
    int srcStride = strideSize;                    // rgb图像缓冲区一行在内存中所占字节数
    for(int h=0 ; h<global_nwindow.height ; h++) {
        memcpy(dst+h*dstStride,data+h*srcStride,strideSize);
    }
   ANativeWindow_unlockAndPost(global_nwindow.mANativeWindow);
}

void destoryANativeWinidow() {
    if(! global_nwindow.mANativeWindow) {
        return;
    }
    ANativeWindow_release(global_nwindow.mANativeWindow);
}