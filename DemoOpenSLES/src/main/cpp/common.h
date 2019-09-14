#include "android/log.h"

#define TAG "PlayAudio"
#define M_LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,"%s",__VA_ARGS__)
#define M_LOG_I(format,...) __android_log_print(ANDROID_LOG_INFO,TAG,format,__VA_ARGS__)
#define M_LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,"%s",__VA_ARGS__)
#define M_LOG_E(format, ...) __android_log_print(ANDROID_LOG_ERROR,TAG,format,__VA_ARGS__)