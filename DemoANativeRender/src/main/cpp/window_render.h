// ANativeWindow调用
// Created by Jiangdg on 2019/9/25.
//

#ifndef FFMPEG4ANDROID_NATIVEWINDOW_RENDER_H
#define FFMPEG4ANDROID_NATIVEWINDOW_RENDER_H

#include "native_render.h"
#include "android/native_window.h"
#include "android/native_window_jni.h"
#include <cstring>


#ifdef __cplusplus
extern "C" {
#endif

struct RenderNativeWindow {
    ANativeWindow *nWindow;
    ANativeWindow_Buffer *outBuffer;
    int width;
    int height;
};

int create_native_window(JNIEnv *env, jobject _surface);
int set_buffers_geometry(int width, int height);
int render_window(int8_t * src_data , int src_stride);
void destory_native_window();


extern RenderNativeWindow g_nativewindow;

#ifdef __cplusplus
};
#endif

#endif //FFMPEG4ANDROID_NATIVEWINDOW_RENDER_H
