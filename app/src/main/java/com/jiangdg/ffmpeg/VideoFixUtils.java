package com.jiangdg.ffmpeg;


public class VideoFixUtils {
	

	public native static int getVideoAngle(String videoPath);

	
	static {
		System.loadLibrary("ffmpeg332");
		System.loadLibrary("avcodec-57");
		System.loadLibrary("avdevice-57");
		System.loadLibrary("avfilter-6");
		System.loadLibrary("avformat-57");
		System.loadLibrary("avutil-55");
		System.loadLibrary("swscale-4");
		System.loadLibrary("swresample-2");
		System.loadLibrary("postproc-54");
	}	
}
