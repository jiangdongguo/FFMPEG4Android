cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)
# 设置so输出路径
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})
# 赋值变量libs
set(libs "${CMAKE_SOURCE_DIR}/src/main/jniLibs")
# 导入ffmpeg相关头文件
include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/include)

#-----------------------------导入ffmpeg库--------------------------------------
add_library(libavcodec-57 SHARED IMPORTED)
set_target_properties(libavcodec-57 PROPERTIES
        IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libavcodec-57.so")

add_library(libavdevice-57 SHARED IMPORTED)
set_target_properties(libavdevice-57 PROPERTIES
        IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libavdevice-57.so")

add_library(libavfilter-6 SHARED IMPORTED)
set_target_properties(libavfilter-6 PROPERTIES
        IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libavfilter-6.so")

add_library(libavformat-57 SHARED IMPORTED)
set_target_properties(libavformat-57 PROPERTIES
        IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libavformat-57.so")

add_library(libavutil-55 SHARED IMPORTED)
set_target_properties(libavutil-55 PROPERTIES
        IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libavutil-55.so")

add_library(libpostproc-54 SHARED IMPORTED)
set_target_properties(libpostproc-54 PROPERTIES
        IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libpostproc-54.so")

add_library(libswresample-2 SHARED IMPORTED)
set_target_properties(libswresample-2 PROPERTIES
        IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libswresample-2.so")

add_library(libswscale-4 SHARED IMPORTED)
set_target_properties(libswscale-4 PROPERTIES
        IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libswscale-4.so")

#------------------------------配置、链接动态库--------------------------------
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -frtti")

add_library(
        avstream

        SHARED

        src/main/cpp/NativeFFmpeg.cpp

        # PlayAudio示例源码
        src/main/cpp/lessons/PlayAudio/ffmpeg_audio.cpp
        src/main/cpp/lessons/PlayAudio/openSLES_audio.cpp
        src/main/cpp/lessons/PlayAudio/threads_play_audio.cpp
        src/main/cpp/lessons/PlayAudio/utils_audio.cpp

        # PlayVideo示例源码
        src/main/cpp/lessons/PlayVideo/ffmpeg_video.cpp
        src/main/cpp/lessons/PlayVideo/nativeWindow_video.cpp
        src/main/cpp/lessons/PlayVideo/threads_play_video.cpp
        src/main/cpp/lessons/PlayVideo/utils_video.cpp

        # PlayStream示例源码
        src/main/cpp/lessons/PlayStream/ffmpeg_stream.cpp
        src/main/cpp/lessons/PlayStream/native_window.cpp
        src/main/cpp/lessons/PlayStream/threads_play_stream.cpp
        src/main/cpp/lessons/PlayStream/openSLES_stream.cpp
        src/main/cpp/lessons/PlayStream/queue_stream.cpp

        # SaveStream示例源码
        src/main/cpp/lessons/SaveStream/ffmpeg_save.cpp
        src/main/cpp/lessons/SaveStream/thread_save_streams.cpp)

# 查找NDK原生库log
find_library(log-lib log)
# 查找NDK原生库OpenSLES
find_library(openSLES-lib openSLES)
# 查找NDK原生库android
find_library(android-lib android)

# 链接所有库到avstream
target_link_libraries(avstream
        libavcodec-57
        libavdevice-57
        libavfilter-6
        libavformat-57
        libavutil-55
        libpostproc-54
        libswresample-2
        libswscale-4
        ${log-lib}
        ${openSLES-lib}
        ${android-lib})