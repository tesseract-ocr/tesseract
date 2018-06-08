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

if  [ $# -ne 3 ] && [ $# -ne 4 ]
then
  echo "Usage:$0 pagesfile tessdata-dir lang [-zoning]"
  exit 1
fi
if [ ! -d src/api ]
then
  echo "Run $0 from the tesseract-ocr root directory!"
  exit 1
fi
if [ ! -r src/api/tesseract ]
then
  if [ ! -r tesseract.exe ]
  then
    echo "Please build tesseract before running $0"
    exit 1
  else
    tess="./tesseract.exe"
  fi
else
  tess="time -f %U -o times.txt src/api/tesseract"
  #tess="time -f %U -o times.txt tesseract"
fi

tessdata=$2
lang=$3
pages=$1
imdir=${pages%/pages}
setname=${imdir##*/}
if [ $# -eq 4 ] && [ "$4" = "-zoning" ]
then
  config=unlv.auto
  resdir=unlvtests/results/zoning.$setname
else
  config=unlv
  resdir=unlvtests/results/$setname
fi
echo -e "Testing on set $setname in directory $imdir to $resdir\n"
mkdir -p "$resdir"
rm -f "unlvtests/results/$setname.times"
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
  $tess "$srcdir/$page.tif" "$resdir/$page" --tessdata-dir $tessdata --oem 1 -l $lang --psm 6 $config 2>&1 |grep -v "OCR Engine" |grep -v "Page 1"
  if [ -r times.txt ]
  then
    read t <times.txt
    echo "$page $t" >>"unlvtests/results/$setname.times"
    echo -e "\033M$page $t"
    if [ "$t" = "Command terminated by signal 2" ]
    then
      exit 0
    fi
  fi
done <"$pages"
