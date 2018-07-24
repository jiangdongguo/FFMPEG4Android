# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_VERBOSE_MAKEFILE on)
set(libs "${CMAKE_SOURCE_DIR}/src/main/jniLibs")
include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/include)

#-----------------------------导入动态库--------------------------------------
add_library(libavcodec-57 SHARED IMPORTED )
set_target_properties(libavcodec-57 PROPERTIES
    IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libavcodec-57.so")

add_library(libavdevice-57 SHARED IMPORTED )
set_target_properties(libavdevice-57 PROPERTIES
    IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libavdevice-57.so")

add_library(libavfilter-6 SHARED IMPORTED )
set_target_properties(libavfilter-6 PROPERTIES
    IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libavfilter-6.so")

add_library(libavformat-57 SHARED IMPORTED )
set_target_properties(libavformat-57 PROPERTIES
    IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libavformat-57.so")

add_library(libavutil-55 SHARED IMPORTED )
set_target_properties(libavutil-55 PROPERTIES
    IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libavutil-55.so")

add_library(libpostproc-54 SHARED IMPORTED )
set_target_properties(libpostproc-54 PROPERTIES
    IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libpostproc-54.so")

add_library(libswresample-2 SHARED IMPORTED )
set_target_properties(libswresample-2 PROPERTIES
    IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libswresample-2.so")

add_library(libswscale-4 SHARED IMPORTED )
set_target_properties(libswscale-4 PROPERTIES
    IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libswscale-4.so")

#------------------------------配置、链接动态库--------------------------------
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -frtti")

add_library( # Sets the name of the library.
             ffmpeg332

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             src/main/cpp/FFMPEG4Android.c)

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log)

target_link_libraries(ffmpeg332 android log
    libavcodec-57
    libavdevice-57
    libavfilter-6
    libavformat-57
    libavutil-55
    libpostproc-54
    libswresample-2
    libswscale-4
    ${log-lib}
    )