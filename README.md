### FFMPEG4Android项目收录的是自己在工作学习中使用ffmpeg进行音视频处理的各种案例，除了ffmpeg本身具备的功能，这里面还结合了大量的第三方库来处理音视频数据，比如OpenSLES、SDL等等。其中，ffmpeg的版本为`4.0.2`，在`Ubuntu16.04`环境下裁剪编译，NDK版本为`r14b`，支持`armeabi`、`armeabi-v7a`以及`arm64-v8a`架构。  


### FFmpeg入门教程  

(1) [Amdroid直播开发之旅(5)：详解ffmpeg编译与在Android平台上的移植](https://blog.csdn.net/AndrExpert/article/details/73823740)  
(2) [Android直播开发之旅(6)：详解ffmpeg命令在Android平台上的使用](https://blog.csdn.net/AndrExpert/article/details/74015671)  
(3) [Android直播开发之旅(12)：初探FFmpeg开源框架](https://blog.csdn.net/AndrExpert/article/details/83268563)  




### 1. [DemoOpenSLES](https://github.com/jiangdongguo/FFMPEG4Android/tree/master/DemoOpenSLES)

&emsp;该项目利用ffmpeg+OpenSLES库播放网络音频流，其中，ffmpeg用于解协议、解码得到合适的pcm数据，openSLES用于播放pcm数据。讲解博客：[Android直播开发之旅(13)：使用FFmpeg+OpenSL ES播放PCM音频](https://blog.csdn.net/AndrExpert/article/details/85254794)  

![Android播放PCM音频项目实战](https://img-blog.csdnimg.cn/20181225215430873.PNG?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L0FuZHJFeHBlcnQ=,size_16,color_FFFFFF,t_70)   

### 2. [DemoSaveFile](https://github.com/jiangdongguo/FFMPEG4Android/tree/master/DemoSaveFile)

&emsp;该项目利用ffmpeg将rtsp、rtmp等网络流保存到文件，封装格式为MP4。讲解博客：Android直播开发之旅(16)：FFmpeg保存网络流到本地文件

