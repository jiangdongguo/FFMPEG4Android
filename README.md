### FFMPEG4Android项目收录的是自己在工作学习中使用ffmpeg进行音视频处理的各种案例，除了ffmpeg本身具备的功能，这里面还结合了大量的第三方库来处理音视频数据，比如OpenSLES、SDL等等。其中，ffmpeg的版本为4.0.2，在Ubuntu16.04环境下裁剪编译，NDK版本为r14b，支持armeabi、armeabi-v7a以及arm64-v8a架构。  




### 1. [DemoOpenSLES](https://github.com/jiangdongguo/FFMPEG4Android/tree/master/DemoOpenSLES)

&emsp;该项目利用ffmpeg+OpenSLES库播放网络音频流，其中，ffmpeg用于解协议、解码得到合适的pcm数据，openSLES用于播放pcm数据。讲解博客：[Android直播开发之旅(13)：使用FFmpeg+OpenSL ES播放PCM音频](https://blog.csdn.net/gongxp123456/article/details/62418752)
