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

if [ $# -ne 1 ]
then
  echo "Usage:$0 pagesfile"
  exit 1
fi
if [ ! -d src/api ]
then
  echo "Run $0 from the tesseract-ocr root directory!"
  exit 1
fi

pages=$1

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
echo "$srcdir/$page.tif"
  # Count character errors.
  unlvtests/ocreval/bin/ocrevalutf8  unlvtests/ocreval/bin/accuracy "$srcdir/$page.txt" "$resdir/$page.unlv" "$resdir/$page.acc"
   accfiles="$accfiles $resdir/$page.acc"
  # Count word errors.
    unlvtests/ocreval/bin/ocrevalutf8  unlvtests/ocreval/bin/wordacc "$srcdir/$page.txt" "$resdir/$page.unlv" "$resdir/$page.wa"
  wafiles="$wafiles $resdir/$page.wa"
done <"$pages"

echo "$accfiles"
echo "$wafiles"

  unlvtests/ocreval/bin/accsum $accfiles >"unlvtests/reports/$setname.characc"
  unlvtests/ocreval/bin/ocrevalutf8 unlvtests/ocreval/bin/wordaccsum $wafiles >"unlvtests/reports/$setname.wordacc"
