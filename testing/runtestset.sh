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

if [ $# -ne 1 -a $# -ne 2 ]
then
  echo "Usage:$0 pagesfile [-zoning]"
  exit 1
fi
if [ ! -d api ]
then
  echo "Run $0 from the tesseract-ocr root directory!"
  exit 1
fi
if [ ! -r api/tesseract ]
then
  if [ ! -r tesseract.exe ]
  then
    echo "Please build tesseract before running $0"
    exit 1
  else
    tess="./tesseract.exe"
  fi
else
  tess="time -f %U -o times.txt api/tesseract"
  export TESSDATA_PREFIX=$PWD/
fi

pages=$1
imdir=${pages%/pages}
setname=${imdir##*/}
if [ $# -eq 2 -a "$2" = "-zoning" ]
then
  config=unlv.auto
  resdir=testing/results/zoning.$setname
else
  config=unlv
  resdir=testing/results/$setname
fi
echo -e "Testing on set $setname in directory $imdir to $resdir\n"
mkdir -p $resdir
rm -f testing/reports/$setname.times
while read page dir
do
  # A pages file may be a list of files with subdirs or maybe just
  # a plain list of files so accommodate both.
  if [ "$dir" ]
  then
     srcdir="$imdir/$dir"
  else
     srcdir="$imdir"
  fi
#  echo "$srcdir/$page.tif"
  $tess $srcdir/$page.tif $resdir/$page -psm 6 $config 2>&1 |grep -v "OCR Engine"
  if [ -r times.txt ]
  then
    read t <times.txt
    echo "$page $t" >>testing/reports/$setname.times
    echo -e "\033M$page $t"
    if [ "$t" = "Command terminated by signal 2" ]
    then
      exit 0
    fi
  fi
done <$pages
