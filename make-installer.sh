#!/bin/sh

TAG=5.0.0-alpha.$(date +%Y%m%d)

git tag -a v$TAG -m "Tesseract $TAG"

ARCHS="i686 x86_64"

./autogen.sh

for ARCH in $ARCHS; do
  HOST=$ARCH-w64-mingw32
  BUILDDIR=bin/ndebug/$HOST-$TAG

  rm -rf $BUILDDIR
  mkdir -p $BUILDDIR
  (
  cd $BUILDDIR
  # Disable OpenMP (see https://github.com/tesseract-ocr/tesseract/issues/1662).
  ../../../configure --disable-openmp --host=$HOST --prefix=/usr/$HOST CXX=$HOST-g++-posix CXXFLAGS="-fno-math-errno -Wall -Wextra -Wpedantic -g -O2"
  make install-jars install training-install html winsetup prefix=$PWD/usr/$HOST
  )
done
