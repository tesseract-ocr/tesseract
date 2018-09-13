#!/bin/bash
# File:        counttestset.sh
# Description: Script to count the errors on a single UNLV set.
# Author:      Ray Smith
# Created:     Wed Jun 13 11:58:01 PDT 2007
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

if [ $# -ne 2 ]
then
  echo "Usage:$0 pagesfile langcode"
  exit 1
fi
if [ ! -d src/api ]
then
  echo "Run $0 from the tesseract-ocr root directory!"
  exit 1
fi

pages=$1
langcode=$2

imdir=${pages%/pages}
setname=${imdir##*/}
resdir=unlvtests/results/$setname
mkdir -p unlvtests/reports
echo "Counting on set $setname in directory $imdir to $resdir"
accfiles=""
wafiles=""
while read page dir
do
  if [ "$dir" ]
  then
     srcdir="$imdir/$dir"
  else
     srcdir="$imdir"
  fi
#echo "$srcdir/$page.tif"
  # Convert groundtruth and recognized text to UTF-8 to correctly treat accented letters.
  iconv -f  ISO8859-1 -t UTF-8 "$srcdir/$page.txt" >"$srcdir/$page.text"
  iconv -f  ISO8859-1 -t UTF-8 "$resdir/$page.unlv" >"$resdir/$page.text"
  # Count character errors.
  ocrevalutf8  accuracy "$srcdir/$page.text" "$resdir/$page.text" > "$resdir/$page.acc"
  accfiles="$accfiles $resdir/$page.acc"
  # Count word errors.
  #langcode should be either eng or spa
  if [ "$langcode" = "eng" ]
    then
      ocrevalutf8  wordacc "$srcdir/$page.text" "$resdir/$page.text" > "$resdir/$page.wa"
    else
      cp ~/ISRI-OCRtk/stopwords/spa.stopwords.txt "$resdir/spa.stopwords"
      ocrevalutf8   wordacc -S"$resdir/spa.stopwords" "$srcdir/$page.text" "$resdir/$page.text" > "$resdir/$page.wa"
  fi
  wafiles="$wafiles $resdir/$page.wa"
done <"$pages"

accsum $accfiles >"unlvtests/results/$setname.characc"
wordaccsum $wafiles >"unlvtests/results/$setname.wordacc"

