#!/bin/bash

# GitHub actions - Create Tesseract installer for Windows

# Author: Stefan Weil (2010-2026)

set -e
set -x

LANG=C.UTF-8

ARCH=$1

if [ "$ARCH" = "i686" ]; then
  MINGW=/mingw32
elif [ "$ARCH" = "aarch64" ]; then
  MINGW=/clangarm64
else
  ARCH=x86_64
  MINGW=/mingw64
fi

ROOTDIR=$PWD
HOST=$ARCH-w64-mingw32
TAG=$(cat VERSION).$(date +%Y%m%d)
BUILDDIR=bin/ndebug/$HOST
PKG_ARCH=mingw-w64-${ARCH/_/-}
PKG=mingw-w64-$ARCH
GXX=g++-$PKG_ARCH
CXX=$HOST-g++-posix
INCLUDE=-isystem
STRIP_FLAG=-s

if [ "$ARCH" = "aarch64" ]; then
  # No ARM64 MinGW g++ in Ubuntu. LLVM must not be newer than MSYS2 libc++.
  LLVM_MINGW=llvm-mingw-20260616-ucrt-ubuntu-22.04-x86_64
  curl -sSL https://github.com/mstorsjo/llvm-mingw/releases/download/20260616/$LLVM_MINGW.tar.xz |
    sudo tar -xJ -C /opt
  export PATH=/opt/$LLVM_MINGW/bin:$PATH
  export CC=$HOST-clang
  export PKG_CONFIG_SYSTEM_INCLUDE_PATH=$MINGW/include
  PKG=mingw-w64-clang-aarch64
  GXX=
  CXX=$HOST-clang++
  # MSYS2 headers must come after the libc++ headers.
  INCLUDE=-idirafter
  # libtool drops compiler-rt, which provides __chkstk.
  EXTRA_LDFLAGS=-Wl,$($CC -print-libgcc-file-name)
  WINPATH_CXX="$CXX -static"
  # The strip of the build host does not know ARM64 Windows binaries.
  STRIP_FLAG="-s --strip-program=$HOST-strip"
fi

# Install packages.
sudo apt-get update --quiet
sudo apt-get install --assume-yes --no-install-recommends --quiet \
  asciidoctor ruby-asciidoctor-pdf curl \
  automake dpkg-dev libtool pkg-config default-jdk-headless \
  mingw-w64-tools nsis ${GXX:+"$GXX"} \
  makepkg pacman-package-manager python3-venv unzip

# Configure pacman.

# Enable mirrorlist.
sudo sed -Ei 's/^#.*(Include.*mirrorlist)/\1/' /etc/pacman.conf
(
# Add msys key for pacman.
cd /usr/share/keyrings
sudo curl -OsS https://raw.githubusercontent.com/msys2/MSYS2-keyring/master/msys2.gpg
sudo curl -OsS https://raw.githubusercontent.com/msys2/MSYS2-keyring/master/msys2-revoked
sudo curl -OsS https://raw.githubusercontent.com/msys2/MSYS2-keyring/master/msys2-trusted
)
(
# Add active environments for pacman.
# See https://www.msys2.org/docs/repos-mirrors/.
sudo mkdir -p /etc/pacman.d
cd /etc/pacman.d
cat <<eod | sudo tee mirrorlist >/dev/null
[${MINGW#/}]
Include = /etc/pacman.d/mirrorlist.mingw
eod
sudo curl -OsS https://raw.githubusercontent.com/msys2/MSYS2-packages/master/pacman-mirrors/mirrorlist.mingw
# sudo curl -OsS https://raw.githubusercontent.com/msys2/MSYS2-packages/master/pacman-mirrors/mirrorlist.msys
)

sudo pacman-key --init
sudo pacman-key --populate msys2
sudo pacman -Syu --noconfirm

# Install required pacman packages.
sudo pacman -S --noconfirm \
 "$PKG-curl-winssl" \
 "$PKG-giflib" \
 "$PKG-icu" \
 "$PKG-leptonica" \
 "$PKG-libarchive" \
 "$PKG-libidn2" \
 "$PKG-openjpeg2" \
 "$PKG-openssl" \
 "$PKG-pango" \
 "$PKG-libpng" \
 "$PKG-libtiff" \
 "$PKG-libwebp"

git config --global user.email "sw@weilnetz.de"
git config --global user.name "Stefan Weil"
git tag -a "v$TAG" -m "Tesseract $TAG"

# Run autogen.
./autogen.sh

# Build Tesseract installer.
mkdir -p "$BUILDDIR" && cd "$BUILDDIR"

# Run configure.
PKG_CONFIG_PATH=$MINGW/lib/pkgconfig
export PKG_CONFIG_PATH
# Disable OpenMP (see https://github.com/tesseract-ocr/tesseract/issues/1662).
../../../configure --disable-openmp --host="$HOST" --prefix="/usr/$HOST" \
  CXX="$CXX" \
  CXXFLAGS="-fno-math-errno -Wall -Wextra -Wpedantic -g -O2 $INCLUDE $MINGW/include" \
  LDFLAGS="-L$MINGW/lib $EXTRA_LDFLAGS" \
  lt_cv_to_host_file_cmd=func_convert_file_noop

make all -j$(nproc)
make training -j$(nproc)

MINGW_INSTALL=${PWD}${MINGW}
make install-jars install training-install html prefix="$MINGW_INSTALL" INSTALL_STRIP_FLAG="$STRIP_FLAG"
test -d venv || python3 -m venv venv
source venv/bin/activate
pip install pefile
mkdir -p dll
ln -sv $("$ROOTDIR/nsis/find_deps.py" --dlldir "$MINGW/bin/" "$MINGW_INSTALL"/bin/*.exe "$MINGW_INSTALL"/bin/*.dll) dll/
if [ -n "$GXX" ]; then
  ln -svf /usr/lib/gcc/x86_64-w64-mingw32/*-win32/libstdc++-6.dll dll/
  ln -svf /usr/lib/gcc/x86_64-w64-mingw32/*-win32/libgcc_s_seh-1.dll dll/
fi
make winsetup prefix="$MINGW_INSTALL" \
  ${WINPATH_CXX:+"WINPATH_CXX=$WINPATH_CXX" "WINPATH_STRIP=$HOST-strip"}
