//
// Created by Jiangdg on 2019/9/16.
//

#ifndef FFMPEG4ANDROID_NATIVE_SAVE_H
#define FFMPEG4ANDROID_NATIVE_SAVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>
#include <android/log.h>

#define MTAG "SaveStream"
#define MLOG_I(...) __android_log_print(ANDROID_LOG_INFO, MTAG, "%s", __VA_ARGS__)
#define MLOG_I_(format, ...) __android_log_print(ANDROID_LOG_INFO, MTAG, format, __VA_ARGS__)
#define MLOG_E(...) __android_log_print(ANDROID_LOG_ERROR, MTAG, "%s", __VA_ARGS__)
#define MLOG_E_(format, ...) __android_log_print(ANDROID_LOG_ERROR, MTAG, format, __VA_ARGS__)

#ifdef __cplusplus
};
#endif

#endif //FFMPEG4ANDROID_NATIVE_SAVE_H
