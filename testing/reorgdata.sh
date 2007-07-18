#!/bin/bash

if [ $# -ne 1 ]
then
    echo "Usage:$0 scantype"
    echo "UNLV data comes in several scan types:"
    echo "3B=300 dpi binary"
    echo "3A=adaptive thresholded 300 dpi"
    echo "3G=300 dpi grey"
    echo "4B=400dpi binary"
    echo "2B=200dpi binary"
    echo "For now we only use 3B"
    exit 1
fi
ext=$1

#There are several test sets without meaningful names, so rename
#them with something a bit more meaningful.
#Each s is oldname/newname
for s in 3/doe3 B/bus M/mag N/news L/legal R/rep S/spn Z/zset
do
    old=${s%/*}
    #if this set was downloaded then process it.
    if [ -r "$old/PAGES" ]
    then
	new=${s#*/}.$ext
	mkdir -p $new
    	echo "Set $old -> $new"
	#The pages file had - instead of _ so fix it and add the extension.
	for page in `cat $old/PAGES`
	do
    	    echo "${page%-*}_${page#*-}.$ext"
	done >$new/pages
	for f in `cat $new/pages`
	do
    	    #Put a tif extension on the tif files.
	    cp $old/${old}_B/$f $new/$f.tif
	    #Put a uzn extension on the zone files.
	    cp $old/${old}_B/${f}Z $new/$f.uzn
	    #Cat all the truth files together and put into a single txt file.
	    cat $old/${old}_GT/${f%.$ext}.Z* >$new/$f.txt
	done
    fi
done
