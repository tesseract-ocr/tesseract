#!/bin/bash
#
mkdir -p ~/lang-files
rm -rf  ~/lang-files/san-*
for testset in vedic fontsamples oldstyle shreelipi alphabetsamples
do
	cd ~/lang-files
	mkdir -p ./san-$testset
	cp ~/lang-deva-downloads/imagessan/$testset/*.* ./san-$testset/
	cd ./san-$testset/
	rename s/-gt.txt/.txt/ *.txt
	ls -1 *.png >pages
	sed -i -e 's/.png//g' pages
done

mkdir -p ~/lang-stopwords
cd ~/lang-stopwords
cp ~/lang-deva-downloads/imagessan/stopwords.txt ./san.stopwords.txt 
