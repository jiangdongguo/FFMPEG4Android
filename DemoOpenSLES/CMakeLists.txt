cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)
# 设置so输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/src/main/output/${ANDROID_ABI})
# 赋值变量libs
set(mylibs "${CMAKE_SOURCE_DIR}/src/main/jniLibs")
# 导入ffmpeg相关头文件
include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/include)

#-----------------------------导入ffmpeg库--------------------------------------
add_library(libffmpeg SHARED IMPORTED)
set_target_properties(libffmpeg PROPERTIES
        IMPORTED_LOCATION "${mylibs}/${ANDROID_ABI}/libffmpeg.so")

#------------------------------配置、链接动态库--------------------------------
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -frtti")

add_library(
        playaudio

        SHARED

        src/main/cpp/native_opensles.cpp
        src/main/cpp/ffmpeg_audio.cpp
        src/main/cpp/openSLES_audio.cpp
        src/main/cpp/threads_play_audio.cpp
        src/main/cpp/utils_audio.cpp
    )

# 查找NDK原生库log
find_library(log-lib log)
# 查找NDK原生库OpenSLES
find_library(openSLES-lib openSLES)
# 查找NDK原生库android
find_library(android-lib android)

# 链接所有库到avstream
target_link_libraries(playaudio
        libffmpeg
        ${log-lib}
        ${openSLES-lib}
        ${android-lib})