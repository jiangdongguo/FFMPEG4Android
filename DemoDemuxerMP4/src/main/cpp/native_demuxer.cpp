// 解码、渲染子线程
// ffmpeg错误码：https://my.oschina.net/u/3700450/blog/1545657
// Created by Jiangdg on 2019/9/23.
//

#include "native_demuxer.h"
#include "ffmpeg_demuxer.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#define NUM_METHODS(x) ((int)(sizeof(x)/ sizeof(x[0])))
int g_exit = 0;
JavaVM *g_jvm;
jobject global_cb_obj;
void *save_thread(void *args);

struct DemuxerThreadParams {
    char* url;
    char* h264path;
    char *aacpath;
};

char * jstringToString(JNIEnv *env, jstring j_str) {
    const  char* c_str = env->GetStringUTFChars(j_str, JNI_FALSE);
    jsize len = env->GetStringLength(j_str);
    char *ret = NULL;
    if(len > 0) {
        ret = (char *)malloc((len+1) * sizeof(char));
        memset(ret, 0, (len + 1));
        memcpy(ret, c_str, len);
        ret[len] = 0;
    }
    return ret;
}

jint startDemuxer(JNIEnv *env, jclass clz, jstring _url, jstring _h264path, jstring _aacpath, jobject callback) {
    g_exit = 0;
    if(!_url) {
        RLOG_E("url or surface can not be null");
        return -1;
    }
    char *c_url = jstringToString(env, _url);
    char *c_h264 = jstringToString(env, _h264path);
    char *c_aac = jstringToString(env, _aacpath);
    if(! c_url || ! c_h264 || ! c_aac) {
        RLOG_E("url or h264 or c_aac path can not be null");
        return NULL;
    }
    if(callback) {
        global_cb_obj = env->NewGlobalRef(callback);
    }
    DemuxerThreadParams *params = (DemuxerThreadParams *)malloc(sizeof(DemuxerThreadParams));
    params->url = c_url;
    params->h264path = c_h264;
    params->aacpath = c_aac;

    pthread_t id_save_thread = 0;
    pthread_create(&id_save_thread, NULL, save_thread, params);

    return 0;
}

void stopDemuxer(JNIEnv *env, jclass clz) {
    g_exit = 1;
}

void *save_thread(void * args) {
    // 主线程与子线程分离
    // 子线程结束后，资源自动回收
    pthread_detach(pthread_self());
    DemuxerThreadParams * params = (DemuxerThreadParams *)args;
    if(! params) {
        RLOG_E("pass parms to demuxer thread failed");
        return NULL;
    }
    // 将当前线程绑定到JavaVM，从JVM中获取JNIEnv*
    JNIEnv *env = NULL;
    jmethodID id_cb = NULL;
    if(g_jvm && global_cb_obj) {
        if(g_jvm->GetEnv(reinterpret_cast<void **>(env), JNI_VERSION_1_4) > 0) {
            RLOG_E("get JNIEnv from JVM failed.");
            return NULL;
        }
        if(JNI_OK != g_jvm->AttachCurrentThread(&env, NULL)) {
            RLOG_E("attach thread failed");
            return NULL;
        }
        jclass cb_cls = env->GetObjectClass(global_cb_obj);
        id_cb = env->GetMethodID(cb_cls, "onCallback", "(I)V");
    }

    // 打开输入流
    RLOG_I_("#### input url = %s", params->url);
    int ret = createDemuxerFFmpeg(params->url);
    if(ret < 0) {
        if(params) {
            free(params->url);
            free(params->h264path);
            free(params);
        }
        if(id_cb && g_jvm) {
            env->CallVoidMethod(global_cb_obj, id_cb, -1);
            env->DeleteGlobalRef(global_cb_obj);
            g_jvm->DetachCurrentThread();
        }
        return NULL;
    }
    // 打开文件
    RLOG_I_("#### h264 save path = %s", params->h264path);
    RLOG_I_("#### aac save path = %s", params->aacpath);
    FILE * h264file = fopen(params->h264path, "wb+");
    FILE * aacfile = fopen(params->aacpath, "wb+");
    if(h264file == NULL || aacfile == NULL) {
        RLOG_E("open save file failed");
        if(params) {
            free(params->url);
            free(params->h264path);
            free(params->aacpath);
            free(params);
        }
        releaseDemuxerFFmpeg();
        if(id_cb && g_jvm) {
            env->CallVoidMethod(global_cb_obj, id_cb, -2);
            env->DeleteGlobalRef(global_cb_obj);
            g_jvm->DetachCurrentThread();
        }
        return NULL;
    }
    int size = -1;
    int audio_profile = getAudioProfile();
    int rate_index = getAudioSampleRateIndex();
    int audio_channels = getAudioChannels();
    if(id_cb) {
        env->CallVoidMethod(global_cb_obj, id_cb, 0);
    }
    bool is_reading = false;
    while ((size = readDataFromAVPacket()) > 0) {
        if(g_exit) {
            break;
        }
        if(! is_reading) {
            is_reading = true;
            if(id_cb) {
                env->CallVoidMethod(global_cb_obj, id_cb, 1);
            }
        }
        uint8_t *out_buffer = (uint8_t *)malloc(size * sizeof(uint8_t));
        memset(out_buffer, 0, size * sizeof(uint8_t));
        int stream_index = handlePacketData(out_buffer, size);
        if(stream_index < 0) {
            continue;
        }
        if(stream_index == getVideoStreamIndex()) {
            fwrite(out_buffer, size,1, h264file);
            RLOG_I_("--->write a video data，size=%d", size);
        } else if(stream_index == getAudioStreamIndex()) {
            // 添加adts头部
            int adtslen = 7;
            uint8_t *ret = (uint8_t *)malloc(size * sizeof(uint8_t) + adtslen * sizeof(char));
            memset(ret, 0, size * sizeof(uint8_t) + adtslen * sizeof(char));
            char * adts = (char *)malloc(adtslen * sizeof(char));
            adts[0] = 0xFF;
            adts[1] = 0xF1;
            adts[2] = (((audio_profile - 1) << 6) + (rate_index << 2) + (audio_channels >> 2));
            adts[3] = (((audio_channels & 3) << 6) + (size >> 11));
            adts[4] = ((size & 0x7FF) >> 3);
            adts[5] = (((size & 7) << 5) + 0x1F);
            adts[6] = 0xFC;

            memcpy(ret, adts, adtslen);
            memcpy(ret+adtslen, out_buffer, size);
            fwrite(ret, size+adtslen, 1, aacfile);
            free(adts);
            free(ret);
            RLOG_I_("--->write a AUDIO data, header=%d, size=%d", adtslen, size);
        }
        free(out_buffer);
    }
    // 释放资源
    if(h264file) {
        fclose(h264file);
    }
    if(aacfile) {
        fclose(aacfile);
    }
    if(params) {
        free(params->url);
        free(params->h264path);
        free(params->aacpath);
        free(params);
    }
    releaseDemuxerFFmpeg();
    if(id_cb && g_jvm) {
        env->CallVoidMethod(global_cb_obj, id_cb, 2);
        env->DeleteGlobalRef(global_cb_obj);
        g_jvm->DetachCurrentThread();
    }
    RLOG_I("##### stop save success.");
    return NULL;
}

static JNINativeMethod g_methods[] = {
        {"nativeStartDemuxer", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Lcom/jiangdg/natives/DemuxerUtil$OnInitCallback;)I", (void *)startDemuxer},
        {"nativeStopDemuxer", "()V", (void *)stopDemuxer}
};

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* jvm, void* reserved) {
    g_jvm = jvm;

    // 获取jvm中的JNIEnv实例对象
    JNIEnv *env;
    if(JNI_OK != jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_4)) {
        RLOG_E("Get JNIEnv failed");
        return JNI_ERR;
    }
    // 注册native方法
    jclass clz = env->FindClass("com/jiangdg/natives/DemuxerUtil");
    int ret = env->RegisterNatives(clz, g_methods, NUM_METHODS(g_methods));
    if( ret < 0) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_4;
}


extern "C"
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* jvm, void* reserved) {
    if(jvm) {
        jvm->DestroyJavaVM();
    }
}





