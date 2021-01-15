#!/bin/bash

# GitHub actions - Create Tesseract installer for Windows

# Author: Stefan Weil (2010)

set -e
set -x

LANG=C.UTF-8

ARCH=$1
DLLS="libgcc_s_sjlj-1.dll libgomp-1.dll libstdc++-6.dll"

if test "$ARCH" != "i686"; then
  ARCH=x86_64
  DLLS="libgcc_s_seh-1.dll libgomp-1.dll libstdc++-6.dll"
fi

ROOTDIR=$PWD
DISTDIR=$ROOTDIR/dist
HOST=$ARCH-w64-mingw32
BUILDDIR=bin/ndebug/$HOST-$TAG
PKG_ARCH=mingw64-${ARCH/_/-}

# Install cygwin key and add cygwin sources.
sudo curl -o /etc/apt/trusted.gpg.d/weilnetz.gpg https://qemu.weilnetz.de/debian/weilnetz.gpg
echo deb https://qemu.weilnetz.de/debian/ testing contrib | \
  sudo tee /etc/apt/sources.list.d/cygwin.list

# Install packages.
sudo apt-get update
sudo apt-get install --assume-yes --no-install-recommends \
  asciidoc xsltproc docbook-xml docbook-xsl \
  automake dpkg-dev libtool pkg-config default-jdk-headless \
  mingw-w64-tools nsis g++-mingw-w64-${ARCH/_/-} \
  $PKG_ARCH-liblept5 $PKG_ARCH-curl \
  $PKG_ARCH-libarchive $PKG_ARCH-giflib $PKG_ARCH-libpng \
  $PKG_ARCH-libwebp $PKG_ARCH-openjpeg2 $PKG_ARCH-openssl $PKG_ARCH-tiff \
  $PKG_ARCH-pango1.0 $PKG_ARCH-icu

sudo ln -sf $PWD/.github/workflows/pkg-config-crosswrapper /usr/bin/$HOST-pkg-config

for dll in $DLLS; do
  ln -sf /usr/lib/gcc/$HOST/*-posix/$dll dll/$HOST
done

TAG=5.0.0-alpha.$(date +%Y%m%d)

git config --global user.email "sw@weilnetz.de"
git config --global user.name "Stefan Weil"
git tag -a v$TAG -m "Tesseract $TAG"

# Run autogen.
./autogen.sh

# Build Tesseract installer.
mkdir -p $BUILDDIR && cd $BUILDDIR

# Run configure.
../../../configure --disable-openmp --host=$HOST --prefix=/usr/$HOST \
  CXX=$HOST-g++-posix \
  CXXFLAGS="-fno-math-errno -Wall -Wextra -Wpedantic -g -O2"

make install-jars install training-install html winsetup prefix=$PWD/usr/$HOST

# Copy result for upload.
mkdir -p $DISTDIR && cp nsis/tesseract-ocr-w*-setup-*.exe $DISTDIR
