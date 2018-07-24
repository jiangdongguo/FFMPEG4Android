#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include "com_jiangdg_ffmpeg_VideoFixUtils.h"
#include "ffmpeg.h"

JNIEXPORT jint JNICALL Java_com_jiangdg_ffmpeg_VideoFixUtils_getVideoAngle
  (JNIEnv *env, jclass jcls, jstring j_videoPath){
	const char *c_videoPath = (*env)->GetStringUTFChars(env,j_videoPath,NULL);

	av_register_all();
	AVFormatContext *fmtCtx = avformat_alloc_context();
	avformat_open_input(&fmtCtx,c_videoPath,NULL,NULL);
	int i;
	int v_stream_idx = -1;
	for(i=0 ; i<fmtCtx->nb_streams ; i++){
		if(fmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			v_stream_idx = i;
			break;
		}
	}
	AVDictionaryEntry *tag = NULL;
	tag = av_dict_get(fmtCtx->streams[v_stream_idx]->metadata,"rotate",tag,NULL);
	int angle = -1;
	if(tag != NULL){
		angle = atoi(tag->value);
	}
	avformat_free_context(fmtCtx);
	(*env)->ReleaseStringUTFChars(env,j_videoPath,c_videoPath);
	LOGI("get video angle");
	return angle;
}
