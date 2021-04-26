#! /bin/bash
#
# Because otherwise I loose the magic incantation...
# 

mkdir build
cd build
cmake -D CMAKE_FIND_DEBUG_MODE=ON -D CMAKE_PREFIX_PATH=../leptonica/build/ ..
