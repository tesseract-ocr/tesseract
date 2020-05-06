cd $SRC/leptonica
./autogen.sh
./configure
make -j$(nproc)
make install
ldconfig

cd $SRC/tesseract
./autogen.sh
CXXFLAGS="$CXXFLAGS -D_GLIBCXX_DEBUG" ./configure --disable-graphics --disable-shared
make -j$(nproc)

cp -R $SRC/tessdata $OUT

$CXX $CXXFLAGS \
    -I $SRC/tesseract/include \
     $SRC/tesseract/unittest/fuzzers/fuzzer-api.cpp -o $OUT/fuzzer-api \
     $SRC/tesseract/.libs/libtesseract.a \
     /usr/local/lib/liblept.a \
     /usr/lib/x86_64-linux-gnu/libtiff.a \
     /usr/lib/x86_64-linux-gnu/libpng.a \
     /usr/lib/x86_64-linux-gnu/libjpeg.a \
     /usr/lib/x86_64-linux-gnu/libjbig.a \
     /usr/lib/x86_64-linux-gnu/liblzma.a \
     -lz \
     $LIB_FUZZING_ENGINE

$CXX $CXXFLAGS \
    -DTESSERACT_FUZZER_WIDTH=512 \
    -DTESSERACT_FUZZER_HEIGHT=256 \
    -I $SRC/tesseract/include \
     $SRC/tesseract/unittest/fuzzers/fuzzer-api.cpp -o $OUT/fuzzer-api-512x256 \
     $SRC/tesseract/.libs/libtesseract.a \
     /usr/local/lib/liblept.a \
     /usr/lib/x86_64-linux-gnu/libtiff.a \
     /usr/lib/x86_64-linux-gnu/libpng.a \
     /usr/lib/x86_64-linux-gnu/libjpeg.a \
     /usr/lib/x86_64-linux-gnu/libjbig.a \
     /usr/lib/x86_64-linux-gnu/liblzma.a \
     -lz \
     $LIB_FUZZING_ENGINE
