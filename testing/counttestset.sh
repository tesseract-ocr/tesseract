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
if [ ! -d api ]
then
  echo "Run $0 from the tesseract-ocr root directory!"
  exit 1
fi
if [ ! -r testing/unlv/accuracy ]
then
  echo "Please download the UNLV accuracy tools (and build) to testing/unlv"
  exit 1
fi
pages=$1

imdir=${pages%/pages}
setname=${imdir##*/}
resdir=testing/results/$setname
mkdir -p testing/reports
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
#  echo "$srcdir/$page.tif"
  # Count character errors.
  testing/unlv/accuracy "$srcdir/$page.txt" "$resdir/$page.txt" "$resdir/$page.acc"
  accfiles="$accfiles $resdir/$page.acc"
  # Count word errors.
  testing/unlv/wordacc "$srcdir/$page.txt" "$resdir/$page.txt" "$resdir/$page.wa"
  wafiles="$wafiles $resdir/$page.wa"
done <"$pages"
testing/unlv/accsum "$accfiles" >"testing/reports/$setname.characc"
testing/unlv/wordaccsum "$wafiles" >"testing/reports/$setname.wordacc"


