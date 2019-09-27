#!/bin/bash

export ANDROID_NDK=/home/jiangdg/opt/android-ndk-r14b
export TMPDIR=/home/jiangdg/opt/ffmpeg-4.0.2/mytmp

TOOLCHAIN_VERSION=4.9
BUILD_PLATFORM=linux-x86_64
ANDROID_VERSION=21

function build_one
{
SYSTEMROOT=$ANDROID_NDK/platforms/android-$ANDROID_VERSION/arch-$ARCH/
TOOLCHAIN_PREFIX=$ANDROID_NDK/toolchains/$ARCH_TYPE-$TOOLCHAIN_VERSION/prebuilt/$BUILD_PLATFORM

echo "######start to configure ffmpeg"

  ./configure \
--prefix=$PREFIX \
--arch=$ARCH \
--target-os=linux \
--enable-cross-compile \
--cross-prefix=$TOOLCHAIN_PREFIX/bin/$ARCH_TYPE- \
--sysroot=$SYSTEMROOT \
--cc=$TOOLCHAIN_PREFIX/bin/$ARCH_TYPE-gcc \
--nm=$TOOLCHAIN_PREFIX/bin/$ARCH_TYPE-nm \
--extra-cxxflags="-D__thumb__ -fexceptions -frtti" \
--extra-cflags="$CFLAGS -Os -fPIC -DANDROID -Wfatal-errors -Wno-deprecated" \
--extra-ldflags="-L$SYSTEMROOT/usr/lib" \
--disable-shared \
--enable-static \
--enable-gpl \
--enable-version3 \
--enable-runtime-cpudetect \
--enable-small \
--enable-network \
--disable-iconv \
--enable-asm \
--enable-neon \
--enable-yasm \
--disable-encoders \
--enable-encoder=h263 \
--enable-encoder=libx264 \
--enable-encoder=aac \
--enable-encoder=mpeg4 \
--enable-encoder=mjpeg \
--enable-encoder=png \
--enable-encoder=gif \
--enable-encoder=bmp \
--disable-muxers \
--enable-muxer=h264 \
--enable-muxer=flv \
--enable-muxer=gif \
--enable-muxer=mp3 \
--enable-muxer=dts \
--enable-muxer=mp4 \
--enable-muxer=mov \
--enable-muxer=mpegts \
--disable-decoders \
--enable-decoder=aac \
--enable-decoder=aac_latm \
--enable-decoder=mp3 \
--enable-decoder=h263 \
--enable-decoder=h264 \
--enable-decoder=mpeg4 \
--enable-decoder=mjpeg \
--enable-decoder=gif \
--enable-decoder=png \
--enable-decoder=bmp \
--enable-decoder=yuv4 \
--disable-demuxers \
--enable-demuxer=image2 \
--enable-demuxer=h263 \
--enable-demuxer=h264 \
--enable-demuxer=flv \
--enable-demuxer=gif \
--enable-demuxer=aac \
--enable-demuxer=ogg \
--enable-demuxer=dts \
--enable-demuxer=mp3 \
--enable-demuxer=mov \
--enable-demuxer=m4v \
--enable-demuxer=concat \
--enable-demuxer=mpegts \
--enable-demuxer=mjpeg \
--enable-demuxer=mpegvideo \
--enable-demuxer=rawvideo \
--enable-demuxer=yuv4mpegpipe \
--enable-demuxer=rtsp \
--disable-parsers \
--enable-parser=aac \
--enable-parser=ac3 \
--enable-parser=h264 \
--enable-parser=mjpeg \
--enable-parser=png \
--enable-parser=bmp\
--enable-parser=mpegvideo \
--enable-parser=mpegaudio \
--disable-protocols \
--enable-protocol=file \
--enable-protocol=hls \
--enable-protocol=concat \
--enable-protocol=rtp \
--enable-protocol=rtmp \
--enable-protocol=rtmpt \
--disable-filters \
--disable-filters \
--enable-filter=aresample \
--enable-filter=asetpts \
--enable-filter=setpts \
--enable-filter=ass \
--enable-filter=scale \
--enable-filter=concat \
--enable-filter=atempo \
--enable-filter=movie \
--enable-filter=overlay \
--enable-filter=rotate \
--enable-filter=transpose \
--enable-filter=hflip \
--enable-zlib \
--disable-outdevs \
--disable-doc \
--disable-ffplay \
--disable-ffmpeg \
--disable-debug \
--disable-ffprobe \
--disable-postproc \
--enable-avdevice \
--disable-symver \
--disable-stripping

echo "######start making $CPU"

make clean
make -j8
make install

echo "######finish making $CPU"

$TOOLCHAIN_PREFIX/bin/$ARCH_TYPE-ld \
-rpath-link=$SYSTEMROOT/usr/lib \
-L$SYSTEMROOT/usr/lib \
-L$PREFIX/lib \
-soname libffmpeg.so -shared -nostdlib -Bsymbolic --whole-archive --no-undefined -o \
$PREFIX/libffmpeg.so \
libavcodec/libavcodec.a \
libavfilter/libavfilter.a \
libswresample/libswresample.a \
libavformat/libavformat.a \
libavutil/libavutil.a \
libswscale/libswscale.a \
libavdevice/libavdevice.a \
-lc -lm -lz -ldl -llog --dynamic-linker=/system/bin/linker \
$TOOLCHAIN_PREFIX/lib/gcc/$ARCH_TYPE/4.9.x/libgcc.a

echo "######generated libffmpeg.so for $CPU"
}

# arm-v7a
#ARCH=arm
#CPU=armv7-a
#PREFIX=./android/$CPU
#ARCH_TYPE=arm-linux-androideabi
#CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=neon"
#build_one

# armeabi
#ARCH=arm
#CPU=armv6
#PREFIX=./android/$CPU
#ARCH_TYPE=arm-linux-androideabi
#CFLAGS="-march=armv6"
#build_one

# arm64
ARCH=arm64
CPU=armv8-a
PREFIX=./android/$CPU
ARCH_TYPE=aarch64-linux-android
CFLAGS="-march=armv8-a"
build_one
