//
// Created by Jiangdg on 2019/9/23.
//

#ifndef FFMPEG4ANDROID_NATIVE_RENDERD_H
#define FFMPEG4ANDROID_NATIVE_RENDERD_H
#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>
#include <android/log.h>

#define RTAG "RenderStream"
#define RLOG_I(...) __android_log_print(ANDROID_LOG_INFO, RTAG, "%s", __VA_ARGS__)
#define RLOG_I_(format, ...) __android_log_print(ANDROID_LOG_INFO, RTAG, format, __VA_ARGS__)
#define RLOG_E(...) __android_log_print(ANDROID_LOG_ERROR, RTAG, "%s", __VA_ARGS__)
#define RLOG_E_(format, ...) __android_log_print(ANDROID_LOG_ERROR, RTAG, format, __VA_ARGS__)

#ifdef __cplusplus
};
#endif
#endif //FFMPEG4ANDROID_NATIVE_RENDERD_H
