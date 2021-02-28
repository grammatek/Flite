#!/bin/bash

# This script builds all android target platforms of flite and installs the binaries in the local folder 'install'.
#
# You can override some variables with environment variables:
#   - ANDROID_NDK: Android ANDROID_NDK including its version subdirectory
#   - ANDROID_NDK_TOOLCHAIN:  Host platform depending on your build machine
#   - ANDROID_API: minSdkVersion
#
# Build directory is build/
# Installation directory is install/
#
# Prerequisites: installed Android NDK

set -o pipefail
set -x
set -e

# Set the location of where you downloaded the ANDROID_NDK including its version subdirectory
export ANDROID_NDK="${ANDROID_NDK:-/usr/local/ndk}"

# Set toolchain, depending on your build machine...
ANDROID_NDK_TOOLCHAIN=${ANDROID_NDK_TOOLCHAIN:-$ANDROID_NDK/toolchains/llvm/prebuilt/darwin-x86_64}
#export ANDROID_NDK_TOOLCHAIN=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64

# Your minSdkVersion.
ANDROID_API=${ANDROID_API:-21}

# This is the top level build directory used in the Makefile
BUILD_ROOT=$(pwd)/build

# Available target platforms:
#   - aarch64-linux-android
#   - armv7a-linux-androideabi
#   - i686-linux-android
#   - x86_64-linux-android
ARCHS="armv7a-linux-androideabi aarch64-linux-android i686-linux-android x86_64-linux-android"

# Cleanup eventually existing build artifacts from previous run
rm -rf $BUILD_ROOT

for TARGET in ${ARCHS}
do
  # Cleanup previous configuration and Makefile artifacts. Calling 'make clean' alone is unfortunately not sufficient
  # and out-of-tree builds are not supported.
  make clean
  rm -rf bin
  rm -f main/*.o
  rm -f config/config config/system.mak
  rm -f config.log config.status

  export AR=$ANDROID_NDK_TOOLCHAIN/bin/llvm-ar
  export CC=$ANDROID_NDK_TOOLCHAIN/bin/$TARGET$ANDROID_API-clang
  export AS=$CC
  export CXX=$ANDROID_NDK_TOOLCHAIN/bin/$TARGET$ANDROID_API-clang++
  export LD=$ANDROID_NDK_TOOLCHAIN/bin/ld
  export NM=$ANDROID_NDK_TOOLCHAIN/bin/nm
  export RANLIB=$ANDROID_NDK_TOOLCHAIN/bin/llvm-ranlib
  export STRIP=$ANDROID_NDK_TOOLCHAIN/bin/llvm-strip

  ./configure --host "$TARGET" --prefix=$(pwd)/install/"$TARGET"
  make -j && make -B install
done


