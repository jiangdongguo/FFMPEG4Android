cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)
set(ffmpeglibs "${PROJECT_SOURCE_DIR}/src/main/jniLibs")
# 指定so输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/src/main/output/${ANDROID_ABI})
# 导入ffmpeg头文件
include_directories(${PROJECT_SOURCE_DIR}/src/main/cpp/include)
# 导入libffmpeg.so
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
        savefile

        SHARED

        src/main/cpp/native_save.cpp
        src/main/cpp/ffmpeg_save.cpp
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(
        savefile

        libffmpeg
        ${log-lib}
        ${android-lib}
)