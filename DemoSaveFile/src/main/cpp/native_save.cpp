// natives方法底层实现
// Created by Jiangdg on 2019/9/16.
//

#include "native_save.h"
#include <pthread.h>
#include "ffmpeg_save.h"

struct ThreadParams {
    char *url;
    char *out;
};

ThreadParams *params;
pthread_t id_save_thread;
char *c_url;
char *c_out;
int g_quit;

static JavaVM *g_jvm;
jobject g_callbackobj;
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
void *save_thread(void *args);

char * jstring_to_string(JNIEnv *env, jstring j_str) {
    const char * c_str  = env->GetStringUTFChars(j_str, JNI_FALSE);
    jsize len = env->GetStringLength(j_str);
    char * ret = (char *) malloc((len+1) * sizeof(char));
    // char * 默认末尾有'/0'
    if(ret) {
        memset(ret, 0, (len+1));
        memcpy(ret, c_str, len);
        ret[len] = 0;
    }
    env->ReleaseStringUTFChars(j_str, c_str);
    return ret;
}

static jint save_start(JNIEnv *env, jobject thiz, jstring _url, jstring _out, jobject callback)
{
    g_quit = 0;
    if(!_url || !_out) {
        MLOG_E("#### save_start: url or output path can not be null");
        return -1;
    }
    c_url = jstring_to_string(env, _url);
    c_out = jstring_to_string(env, _out);

    g_callbackobj = env->NewGlobalRef(callback);

    // 启动子线程
    // sizeof(params)得到的是指针变量大小，固定占4字节
    params = (ThreadParams *)malloc(sizeof(ThreadParams));
    params->url = c_url;
    params->out = c_out;
    pthread_create(&id_save_thread, NULL, save_thread, params);

    return 0;
}

static int save_stop(JNIEnv *env, jobject thiz)
{
    g_quit = 1;
    return 0;
}

// 子线程函数入口
// 将当前线程绑定到JavaVM，从JVM中获取JNIEnv*
// 并得到回调接口方法
void *save_thread(void *args) {
    pthread_detach(pthread_self());
    JNIEnv *env = NULL;
    jmethodID methodId = NULL;
    if(g_jvm) {
        if(g_jvm->GetEnv(reinterpret_cast<void **>(env), JNI_VERSION_1_4)>0) {
            MLOG_E("Get JINEnv object failed.");
            return NULL;
        }
        if(JNI_OK != g_jvm->AttachCurrentThread(&env, NULL)) {
            MLOG_E("Get JINEnv object failed.");
            return NULL;
        }
        jclass cbClz = env->GetObjectClass(g_callbackobj);
        methodId = env->GetMethodID(cbClz, "onResult", "(I)V");
    }

    initFFmpeg();
    ThreadParams *params = (ThreadParams *)args;
    if(! params) {
        MLOG_E("#### get thread parms failed in save_thread.");
        if(env) {
            env->DeleteGlobalRef(g_callbackobj);
            g_jvm->DetachCurrentThread();
        }
        return NULL;
    }
    // 打开输入流
    int ret = openInput(params->url);
    if(ret < 0) {
        MLOG_E_("#### open input url failed,err=%d", ret);
        if(g_jvm && methodId) {
            env->CallVoidMethod(g_callbackobj, methodId, -1);
            env->DeleteGlobalRef(g_callbackobj);
            g_jvm->DetachCurrentThread();
        }
        closeInput();
        return NULL;
    }
    // 打开输出文件
    ret = openOutput(params->out);
    if(ret < 0) {
        MLOG_E_("#### open out file failed,err=%d", ret);
        if(g_jvm && methodId) {
            env->CallVoidMethod(g_callbackobj, methodId, -2);
            env->DeleteGlobalRef(g_callbackobj);
            g_jvm->DetachCurrentThread();
        }
        closeOutput();
        return NULL;
    }
    if(methodId) {
        env->CallVoidMethod(g_callbackobj, methodId, 0);
    }
    // 循环读取
    bool is_reading = false;
    while (! g_quit) {
        if(readAvPacketFromInput() == 0) {
            writeAvPacketToOutput();
            MLOG_I("##### write a packet data");
        }
        if(! is_reading) {
            is_reading = true;
            env->CallVoidMethod(g_callbackobj, methodId, 1);
        }
    }
    // 释放各种资源
    releaseFFmpeg();
    if(params) {
        free(params);
    }
    if(c_url) {
        free(c_url);
    }
    if(c_out) {
        free(c_out);
    }
    if(g_jvm) {
        env->CallVoidMethod(g_callbackobj, methodId, 2);
        env->DeleteGlobalRef(g_callbackobj);
        g_jvm->DetachCurrentThread();
    }
    MLOG_I("save stream success.");
    // void * 必须要返回NULL
    // 否则会报libc: Fatal signal 5 (SIGTRAP)错误
    return NULL;
}

static JNINativeMethod g_methods[] = {
        {"nativeStart","(Ljava/lang/String;Ljava/lang/String;Lcom/jiangdg/natives/SaveStreamUtil$OnInitCallBack;)I", (void *)save_start},
        {"nativeStop", "()I", (void *)save_stop}
};

extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    JNIEnv *env;
    // 缓存JavaVM，获取JNIEnv实例
    g_jvm = jvm;
    if(jvm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
        MLOG_E("##### get JNIEnv object failed.");
        return JNI_ERR;
    }
    // 获取Java Native类
    jclass clazz = env->FindClass("com/jiangdg/natives/SaveStreamUtil");
    // 注册Natives方法，NELEM获得方法的数量
    if(env->RegisterNatives(clazz, g_methods, NELEM(g_methods)) < 0) {
        MLOG_E("##### register natives failed.");
        return JNI_ERR;
    }
    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{
    if(g_jvm) {
        g_jvm->DestroyJavaVM();
    }
}
