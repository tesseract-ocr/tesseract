#!/bin/bash -eu
# Copyright 2019 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################

cd "$SRC"/leptonica
./autogen.sh
./configure --disable-shared
make SUBDIRS=src install -j"$(nproc)"
ldconfig

cd "$SRC"/tesseract
./autogen.sh
CXXFLAGS="$CXXFLAGS -D_GLIBCXX_DEBUG" ./configure --disable-graphics --disable-shared
make -j"$(nproc)"

# Get the models which are needed for the fuzzers.

mkdir -p "$OUT"/tessdata
(
cd "$OUT"/tessdata
test -f eng.traineddata || \
  curl -sSL -O https://github.com/tesseract-ocr/tessdata/raw/main/eng.traineddata
)

# OSS-Fuzz requires static linking for the project specific libraries,
# so get the list of those libraries for Leptonica and TIFF.
# Note that libm must be linker dynamically to avoid linker errors.

LEPTONICA_CFLAGS=$(pkg-config --cflags lept)
LEPTONICA_LIBS=$(pkg-config --static --libs lept)
LIBTIFF_LIBS=$(pkg-config --static --libs libtiff-4 | sed 's/ -lm//')

$CXX $CXXFLAGS \
    -I "$SRC"/tesseract/include \
     "$SRC"/tesseract/unittest/fuzzers/fuzzer-api.cpp -o "$OUT"/fuzzer-api \
     "$SRC"/tesseract/.libs/libtesseract.a \
     $LEPTONICA_CFLAGS \
     -Wl,-Bstatic $LEPTONICA_LIBS $LIBTIFF_LIBS -Wl,-Bdynamic \
     $LIB_FUZZING_ENGINE

$CXX $CXXFLAGS \
    -DTESSERACT_FUZZER_WIDTH=512 \
    -DTESSERACT_FUZZER_HEIGHT=256 \
    -I "$SRC"/tesseract/include \
     "$SRC"/tesseract/unittest/fuzzers/fuzzer-api.cpp -o "$OUT"/fuzzer-api-512x256 \
     "$SRC"/tesseract/.libs/libtesseract.a \
     $LEPTONICA_CFLAGS \
     -Wl,-Bstatic $LEPTONICA_LIBS $LIBTIFF_LIBS -Wl,-Bdynamic \
     $LIB_FUZZING_ENGINE

# fuzzer-raw-image reads the page segmentation mode and the size, depth and
# resolution of the image from a text line at the start of the fuzz input.
# The dictionary has the keys and values for that line. HAVE_CONFIG_H is
# needed for config_auto.h which tells whether the legacy engine was built.

$CXX $CXXFLAGS \
    -DHAVE_CONFIG_H \
    -I "$SRC"/tesseract/include \
     "$SRC"/tesseract/unittest/fuzzers/fuzzer-raw-image.cpp -o "$OUT"/fuzzer-raw-image \
     "$SRC"/tesseract/.libs/libtesseract.a \
     $LEPTONICA_CFLAGS \
     -Wl,-Bstatic $LEPTONICA_LIBS $LIBTIFF_LIBS -Wl,-Bdynamic \
     $LIB_FUZZING_ENGINE

cp "$SRC"/tesseract/unittest/fuzzers/fuzzer-raw-image.dict "$OUT"/

# The seed corpus is generated, so that no image files are needed.

python3 "$SRC"/tesseract/unittest/fuzzers/fuzzer-raw-image-seeds.py \
    "$OUT"/fuzzer-raw-image_seed_corpus.zip
