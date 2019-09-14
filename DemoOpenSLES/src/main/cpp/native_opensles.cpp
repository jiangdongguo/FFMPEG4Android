#include "native_opensles.h"

//---------------------------------------------------------------------------------------
//------------------------------  FFmpeg+OpenSL ES示例  ---------------------------------
//---------------------------------------------------------------------------------------

char *tmpURL = NULL;

extern "C"
JNIEXPORT jint JNICALL
Java_com_jiangdg_natives_PlayAudioUtil_nativeStartPlayAudio(JNIEnv *env, jobject instance,
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
Java_com_jiangdg_natives_PlayAudioUtil_nativeStopPlayAudio(JNIEnv *env,jobject instances) {
    stopAllThread_audio();
    if (tmpURL) {
        free(tmpURL);
    }
}