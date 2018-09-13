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

if  [ $# -ne 4 ] 
then
  echo "Usage:$0 pagesfile tessdata-dir langcode imgext"
  exit 1
fi

tess="time -f %U -o times.txt ./src/api/tesseract"

tessdata=$2
langcode=$3
imgext=$4
pages=$1
imdir=${pages%/pages}
setname=${imdir##*/}

config=""
resdir=langtests/results/$setname

echo -e "Testing on set $setname in directory $imdir to $resdir\n"
mkdir -p "$resdir"
rm -f "langtests/results/$setname.times"
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
  echo "$srcdir/$page"
  $tess "$srcdir/$page.$imgext" "$resdir/$page" --tessdata-dir $tessdata --oem 1 -l $langcode --psm 6 $config 2>&1 |grep -v "OCR Engine" |grep -v "Page 1"
  if [ -r times.txt ]
  then
    read t <times.txt
    echo "$page $t" >>"langtests/results/$setname.times"
    echo -e "\033M$page $t"
    if [ "$t" = "Command terminated by signal 2" ]
    then
      exit 0
    fi
  fi
done <"$pages"
