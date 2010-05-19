#!/bin/bash
# File:        runalltests.sh
# Description: Script to run a set of UNLV test sets.
# Author:      Ray Smith
# Created:     Thu Jun 14 08:21:01 PDT 2007
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
   echo "Usage:$0 unlv-data-dir version-id"
   exit 1
fi
if [ ! -d api ]
then
  echo "Run $0 from the tesseract-ocr root directory!"
  exit 1
fi
if [ ! -r api/tesseract -a ! -r tesseract.exe ]
then
  echo "Please build tesseract before running $0"
  exit 1
fi
if [ ! -r testing/unlv/accuracy -a ! -r testing/unlv/accuracy.exe ]
then
  echo "Please download the UNLV accuracy tools (and build) to testing/unlv"
  exit 1
fi

#deltapc new old calculates the %change from old to new
deltapc() {
awk ' BEGIN {
printf("%.2f", 100.0*('$1'-'$2')/'$2');
}'
}

#timesum computes the total cpu time
timesum() {
awk ' BEGIN {
total = 0.0;
}
{
  total += $2;
}
END {
  printf("%.2f\n", total);
}' $1
}

imdir="$1"
vid="$2"
bindir=${0%/*}
if [ "$bindir" = "$0" ]
then
    bindir="./"
fi
rdir=testing/reports
testsets="bus.3B doe3.3B mag.3B news.3B"

totalerrs=0
totalwerrs=0
totalnswerrs=0
totalolderrs=0
totaloldwerrs=0
totaloldnswerrs=0
for set in $testsets
do
    if [ -r $imdir/$set/pages ]
    then
	# Run tesseract on all the pages.
	$bindir/runtestset.sh $imdir/$set/pages
	# Count the errors on all the pages.
	$bindir/counttestset.sh $imdir/$set/pages
	# Get the old character word and nonstop word errors.
	olderrs=`cat testing/reports/1995.$set.sum | cut -f3`
	oldwerrs=`cat testing/reports/1995.$set.sum | cut -f6`
	oldnswerrs=`cat testing/reports/1995.$set.sum | cut -f9`
	# Get the new character word and nonstop word errors and accuracy.
	cherrs=`head -4 testing/reports/$set.characc |tail -1 |cut -c1-9 |
	    tr -d '[:blank:]'`
	chacc=`head -5 testing/reports/$set.characc |tail -1 |cut -c1-9 |
	    tr -d '[:blank:]'`
	wderrs=`head -4 testing/reports/$set.wordacc |tail -1 |cut -c1-9 |
	    tr -d '[:blank:]'`
	wdacc=`head -5 testing/reports/$set.wordacc |tail -1 |cut -c1-9 |
	    tr -d '[:blank:]'`
	nswderrs=`grep Total testing/reports/$set.wordacc |head -2 |tail -1 |
	    cut -c10-17 |tr -d '[:blank:]'`
	nswdacc=`grep Total testing/reports/$set.wordacc |head -2 |tail -1 |
	    cut -c19-26 |tr -d '[:blank:]'`
	# Compute the percent change.
	chdelta=`deltapc $cherrs $olderrs`
	wdelta=`deltapc $wderrs $oldwerrs`
	nswdelta=`deltapc $nswderrs $oldnswerrs`
	sumfile=$rdir/$vid.$set.sum
        if [ -r testing/reports/$set.times ]
        then
          total_time=`timesum testing/reports/$set.times`
          if [ -r testing/reports/prev/$set.times ]
          then
            paste testing/reports/prev/$set.times testing/reports/$set.times |
              awk '{ printf("%s %.2f\n", $1, $4-$2); }' |sort -k2n >testing/reports/$set.timedelta
          fi
	else
          total_time='0.0'
        fi
        echo "$vid	$set	$cherrs	$chacc	$chdelta%	$wderrs	$wdacc\
	$wdelta%	$nswderrs	$nswdacc	$nswdelta%	${total_time}s" >$sumfile
	# Sum totals over all the testsets.
	let totalerrs=totalerrs+cherrs
	let totalwerrs=totalwerrs+wderrs
	let totalnswerrs=totalnswerrs+nswderrs
	let totalolderrs=totalolderrs+olderrs
	let totaloldwerrs=totaloldwerrs+oldwerrs
	let totaloldnswerrs=totaloldnswerrs+oldnswerrs
    fi
done
# Compute grand total percent change.
chdelta=`deltapc $totalerrs $totalolderrs`
wdelta=`deltapc $totalwerrs $totaloldwerrs`
nswdelta=`deltapc $totalnswerrs $totaloldnswerrs `
tfile=$rdir/$vid.total.sum
echo "$vid	Total	$totalerrs	-	$chdelta%	$totalwerrs\
	-	$wdelta%	$totalnswerrs	-	$nswdelta%" >$tfile
cat $rdir/1995.*.sum $rdir/$vid.*.sum >$rdir/$vid.summary
