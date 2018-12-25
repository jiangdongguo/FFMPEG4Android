// Native window驱动模块
// Created by jianddongguo on 2018/12/3.
//

#ifndef FFMPEG4ANDROID_NATIVEWINDOW_H
#define FFMPEG4ANDROID_NATIVEWINDOW_H

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <string.h>
#include "android/log.h"
#define TAG "PlayVideo"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,"%s",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,"%s",__VA_ARGS__)
#define LOG_E(format, ...) __android_log_print(ANDROID_LOG_ERROR,TAG,format,__VA_ARGS__)


typedef struct Window_Stream{
    ANativeWindow *mANativeWindow;
    ANativeWindow_Buffer mWindowBuffer;

    int32_t width = 0;
    int32_t height = 0;
}Render_Window_Stream;

int createANativeWinidow_stream(JNIEnv* env, jobject surface);
void changeBuffersGeometry_stream(int32_t width, int32_t height);
void renderWindow_stream(uint8_t *data,int strideSize);
void destoryANativeWinidow_stream();

// 声明全局变量
extern Render_Window_Stream global_nwindow_stream;

#endif //FFMPEG4ANDROID_NATIVEWINDOW_H
