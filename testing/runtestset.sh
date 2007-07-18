#!/bin/bash
# File:        runtestset.sh
# Description: Script to run tesseract on a single UNLV set.
# Author:      Ray Smith
# Created:     Wed Jun 13 10:13:01 PDT 2007
#
# (C) Copyright 2007, Google Inc.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if [ $# -ne 1 ]
then
  echo "Usage:$0 pagesfile"
  exit 1
fi
if [ ! -d ccmain ]
then
  echo "Run $0 from the tesseract-ocr root directory!"
  exit 1
fi
if [ ! -r ccmain/tesseract ]
then
  if [ ! -r tesseract.exe ]
  then
    echo "Please build tesseract before running $0"
    exit 1
  else
    tess="./tesseract.exe"
  fi
else
  tess="ccmain/tesseract"
  export TESSDATA_PREFIX=$PWD/
fi

pages=$1

imdir=${pages%/pages}
setname=${imdir##*/}
resdir=testing/results/$setname
echo "Testing on set $setname in directory $imdir to $resdir"
mkdir -p $resdir
while read page dir
do
  # A pages file may be a list of files with subdirs or maybe just
  # a plain list of files so accomodate both.
  if [ "$dir" ]
  then
     srcdir="$imdir/$dir"
  else
     srcdir="$imdir"
  fi
#  echo "$srcdir/$page.tif"
  $tess $srcdir/$page.tif $resdir/$page nobatch unlv
done <$pages
