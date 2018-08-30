#!/bin/bash
#
mkdir -p ~/lang-downloads
cd ~/lang-downloads
wget -O frk-jbarth-ubhd.zip http://digi.ub.uni-heidelberg.de/diglitData/v/abbyy11r8-vs-tesseract4.zip
wget -O frk-stweil-gt.zip https://digi.bib.uni-mannheim.de/~stweil/fraktur-gt.zip

mkdir -p ~/lang-files
cd ~/lang-files
unzip ~/lang-downloads/frk-jbarth-ubhd.zip -d frk
unzip ~/lang-downloads/frk-stweil-gt.zip -d frk
mkdir -p ./frk-ligatures
cp ./frk/abbyy-vs-tesseract/*.tif ./frk-ligatures/
cp ./frk/gt/*.txt ./frk-ligatures/

cd ./frk-ligatures/
ls -1 *.tif >pages
sed -i -e 's/.tif//g' pages

mkdir -p ~/lang-stopwords
cd ~/lang-stopwords
wget -O frk.stopwords.txt https://raw.githubusercontent.com/stopwords-iso/stopwords-de/master/stopwords-de.txt

echo "Edit ~/lang-files/stopwords/frk.stopwords.txt as wordacc uses a space delimited stopwords file, not line delimited."
