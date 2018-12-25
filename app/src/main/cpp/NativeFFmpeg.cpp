#include "NativeFFmpeg.h"
#include "lessons/SaveStream/thread_save_streams.h"
#include "lessons/PlayStream/threads_play_stream.h"

jobject globalSurfaceObj_video;
JavaVM * j_vm_video;
JavaVM *j_vm_stream;
jobject globalSurfaceObj_stream;
ThreadParams *threadParams;

//---------------------------------------------------------------------------------------
//------------------------------  FFmpeg+OpenSL ES示例  ---------------------------------
//---------------------------------------------------------------------------------------

char *tmpURL = NULL;

extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativeStartPlayAudio(JNIEnv *env, jobject instance,
                                                           jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    jsize urlLen = env->GetStringLength(url_);
    // 缓存url再传递，因为url_地址指向的数据可能会被GC回收
    // 导致下次传递出现乱码。注意：char *以'/0'结尾
    tmpURL = (char *) malloc((urlLen + 1) * sizeof(char));
    memset(tmpURL, 0, (urlLen + 1));
    memcpy(tmpURL, url, (urlLen + 1));

    pthread_t threadId_adecode;
    // 开启解码线程
    pthread_create(&threadId_adecode, NULL, decode_audio_thread, (void *) tmpURL);

    env->ReleaseStringUTFChars(url_, url);
    return 0;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativeStopPlayAudio(JNIEnv *env,jobject instances) {
    stopAllThread_audio();
    if (tmpURL) {
        free(tmpURL);
    }
}

//---------------------------------------------------------------------------------------
//------------------------------  FFmpeg+NativeWindow示例  ------------------------------
//---------------------------------------------------------------------------------------

extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativeStartPlayVideo(JNIEnv *env, jobject instance,
                                                           jstring url_, jobject surface) {
    const char *url = env->GetStringUTFChars(url_, 0);
    jsize urlLen = env->GetStringLength(url_);
    // 缓存url再传递，因为url_地址指向的数据可能会被GC回收
    // 导致下次传递出现乱码。注意：char *以'/0'结尾
    tmpURL = (char *) malloc((urlLen + 1) * sizeof(char));
    memset(tmpURL, 0, (urlLen + 1));
    memcpy(tmpURL, url, (urlLen + 1));

    env->GetJavaVM(&j_vm_video);
    globalSurfaceObj_video = env->NewGlobalRef(surface);

    pthread_t threadId_vdecode;
    // 开启解码线程
    pthread_create(&threadId_vdecode, NULL, decode_video_thread, (void *) tmpURL);

    env->ReleaseStringUTFChars(url_, url);
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativeStopPlayVideo(JNIEnv *env, jobject instance) {
    stop_threads_video();

    if (tmpURL) {
        free(tmpURL);
    }
}

//---------------------------------------------------------------------------------------
//-----------------------  FFmpeg+OpenSL ES+NativeWindow示例  ---------------------------
//---------------------------------------------------------------------------------------

extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativeStartPlayMedia(JNIEnv *env, jobject instance,
                                                           jstring url_, jobject surface) {
    const char *url = env->GetStringUTFChars(url_, 0);
    jsize urlLen = env->GetStringLength(url_);
    // 缓存url再传递，因为url_地址指向的数据可能会被GC回收
    // 导致下次传递出现乱码。注意：char *以'/0'结尾
    tmpURL = (char *) malloc((urlLen + 1) * sizeof(char));
    memset(tmpURL, 0, (urlLen + 1));
    memcpy(tmpURL, url, (urlLen + 1));

    env->GetJavaVM(&j_vm_stream);
    globalSurfaceObj_stream = env->NewGlobalRef(surface);

    pthread_t id_decode_media;
    pthread_create(&id_decode_media,NULL,decode_stream_thread,(void *)tmpURL);

    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativeStopPlayMedia(JNIEnv *env, jobject instance) {
    if (tmpURL) {
        free(tmpURL);
    }

    stopAllThread_stream();
}

//---------------------------------------------------------------------------------------
//----------------------------------  保存网络流示例  -------------------------------------
//---------------------------------------------------------------------------------------extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativeStartSaveStream(JNIEnv *env, jobject instance,
                                                            jstring url_, jstring outPath_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    const char *outPath = env->GetStringUTFChars(outPath_, 0);

    threadParams = (ThreadParams *) malloc(sizeof(ThreadParams));
    threadParams->inputUrl = url;
    threadParams->outputUrl = outPath;

    pthread_t id_save;
    pthread_create(&id_save,NULL,thread_save_stream,(void *)threadParams);

    env->ReleaseStringUTFChars(url_, url);
    env->ReleaseStringUTFChars(outPath_, outPath);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jiangdg_natives_NativeFFmpeg_nativeStopSaveStream(JNIEnv *env, jobject instance) {
    stop_save_thread();
    if(threadParams) {
        free(threadParams);
    }
}