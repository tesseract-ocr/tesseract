#! /bin/bash
#
# Because otherwise I loose the magic incantation...
# 

mkdir build
cd build
cmake -D CMAKE_FIND_DEBUG_MODE=ON -D ICU_ROOT=../owemdjee/unicode-icu/icu4c -D LibArchive_INCLUDE_DIR=../owemdjee/libarchive/libarchive -D "Python_ROOT_DIR=C:/Program Files/Python38" -D Leptonica_INCLUDE_DIRS=../leptonica/src/ -D Leptonica_DIR=../leptonica/build/ ..
