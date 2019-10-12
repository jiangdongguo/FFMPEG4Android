// 解码、渲染子线程
// Created by Jiangdg on 2019/9/23.
//

#include "native_render.h"
#include "ffmpeg_render.h"
#include "utils_render.h"
#include "window_render.h"
#include <pthread.h>
#include <unistd.h>

#define NUM_METHODS(x) ((int)(sizeof(x)/ sizeof(x[0])))

static JavaVM *g_jvm;
static int g_exit = 0;
jobject g_surfaceObj;
RGBPacketQueue *queue;
void *decode_thread(void * args);
void *render_thread(void *args);

struct RenderParams {
    int width;
    int height;
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

jint startRender(JNIEnv *env, jclass clz, jstring _url, jobject _surface) {
    g_exit = 0;
    if(!_url || !_surface) {
        RLOG_E("url or surface can not be null");
        return -1;
    }
    char *c_url = jstringToString(env, _url);
    if(! c_url) {
        RLOG_E("url can not be null");
        return NULL;
    }
    g_surfaceObj = env->NewGlobalRef(_surface);
    pthread_t id_decode_thread = 0;
    pthread_create(&id_decode_thread, NULL, decode_thread, c_url);

    return 0;
}

void stopRender(JNIEnv *env, jclass clz) {
    g_exit = 1;
}

void *decode_thread(void * args) {
    // 主线程与子线程分离
    // 子线程结束后，资源自动回收
    pthread_detach(pthread_self());

    char * input_url = (char *)args;
    int ret = createRenderFFmpeg(input_url);
    if(ret < 0) {
        return NULL;
    }
    // 初始化链表
    queue = (RGBPacketQueue *)malloc(sizeof(RGBPacketQueue));
    queue_rgb_init(queue);
    // 开启渲染线程
    RenderParams * params = (RenderParams *)malloc(sizeof(RenderParams));
    params->width = getVideoWidth();
    params->height = getVideoHeight();

    pthread_t id_render_thread = 0;
    pthread_create(&id_render_thread, NULL, render_thread, params);

    // 每帧图像解码、转换格式
    while (readH264DataFromAVPacket() == 0) {
        if(g_exit) {
            break;
        }
        if(decodeH264Data() >= 0) {
            RGBPacket *rgbPacket = (RGBPacket *)malloc(sizeof(RGBPacket));
            rgbPacket->data = g_render.rgbFrame->data[0];
            rgbPacket->size = g_render.rgbFrame->linesize[0];
            queue_rgb_put(queue, rgbPacket);
            RLOG_I("----- decode a video frame");
        }
    }
    // 等待渲染完毕，释放资源
    sleep(1);
    if(input_url) {
        free(input_url);
    }
    releaseRenderFFmpeg();
    RLOG_I("##### stop decode video success.");
    return NULL;
}

void *render_thread(void *args) {
    pthread_detach(pthread_self());
    RenderParams* params = (RenderParams *)args;
    if(! params) {
        RLOG_E("args can not be null in render_thread");
        return NULL;
    }
    int width = params->width;
    int height = params->height;
    // 绑定当前线程到JVM，获取JNIEnv实例
    JNIEnv *env = NULL;
    if(! g_jvm || g_jvm->GetEnv(reinterpret_cast<void **>(env), JNI_VERSION_1_4) > 0) {
        RLOG_E("get JVM failed");
        return NULL;
    }
    g_jvm->AttachCurrentThread(&env, NULL);
    // 初始化ANativeWindow
    int ret = create_native_window(env, g_surfaceObj);
    if(ret < 0) {
        g_jvm->DetachCurrentThread();
        return NULL;
    }
    set_buffers_geometry(width, height);
    // 循环读取RGB数据包，渲染
    RGBPacket packet;
    RGBPacket *rgbPacket = &packet;
    for(;;) {
        if (g_exit) {
            break;
        }
        // 渲染rgb
        if(queue_rgb_get(queue,rgbPacket) > 0) {
            if(render_window(rgbPacket->data, rgbPacket->size) >= 0) {
                RLOG_I("##### render a frame");
            }
        }
    }
    // 释放资源
    if(g_surfaceObj) {
        env->DeleteGlobalRef(g_surfaceObj);
    }
    if(params) {
        free(params);
    }
    g_jvm->DetachCurrentThread();
    destory_native_window();
    RLOG_I("##### stop render success.");
    return NULL;
}

static JNINativeMethod g_methods[] = {
        {"nativeStartRender", "(Ljava/lang/String;Landroid/view/Surface;)I", (void *)startRender},
        {"nativeStopRender", "()V", (void *)stopRender}
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
    jclass clz = env->FindClass("com/jiangdg/natives/RenderUtil");
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





