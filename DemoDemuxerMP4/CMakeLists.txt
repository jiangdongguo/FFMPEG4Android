CMAKE_MINIMUM_REQUIRED(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)

# 设置输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/src/main/output/${ANDROID_ABI})

# 将ffmpeg.so路径保存到变量mylibs
set(ffmpeglibs "${PROJECT_SOURCE_DIR}/src/main/jniLibs")
# 导入ffmpeg头文件
include_directories(${PROJECT_SOURCE_DIR}/src/main/cpp/include)

# 找到ffmpeg.so库
add_library(libffmpeg SHARED IMPORTED)
set_target_properties(
        libffmpeg
        PROPERTIES
        IMPORTED_LOCATION
        "${ffmpeglibs}/${ANDROID_ABI}/libffmpeg.so"
)

# 配置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -frtti")

add_library(
        demuxermp4

        SHARED

        src/main/cpp/native_demuxer.cpp
        src/main/cpp/ffmpeg_demuxer.cpp
        src/main/cpp/utils_demuxer.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(
        demuxermp4

        libffmpeg
        ${log-lib}
        ${android-lib}
)