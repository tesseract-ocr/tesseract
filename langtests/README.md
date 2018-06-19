# How to run Language tests.

The scripts in this directory make it possible to test Accuracy of Tesseract 
for different languages. 

### Step 1: If not already installed, download the modified ISRI toolkit, 
make and install the tools in /usr/local/bin.

```
git clone https://github.com/Shreeshrii/ocr-evaluation-tools.git
cd ~/ocr-evaluation-tools
sudo make install
```

### Step 2: If not alrady installed, Build tesseract.

## Fraktur - frk and scripts/Fraktur

### Step 3: download the images and groundtruth

```
mkdir -p ~/lang-downloads
cd ~/lang-downloads
wget -O frk-jbarth-ubhd.zip http://digi.ub.uni-heidelberg.de/diglitData/v/abbyy11r8-vs-tesseract4.zip
```

### Step 4: extract the files. 
It doesn't really matter where in your filesystem you put them, 
but they must go under a common root, for example, ~/lang-files

```
mkdir -p ~/lang-files
cd ~/lang-files
unzip ~/lang-downloads/frk-jbarth-ubhd.zip
mkdir ./frk-jbarth-ubhd
cp ./abbyy-vs-tesseract/manual/*.txt ./frk-jbarth-ubhd/
cp ./abbyy-vs-tesseract/*.tif ./frk-jbarth-ubhd/
rename s/.txt/-gt.txt/ ./frk-jbarth-ubhd/*.txt
cd ./frk-jbarth-ubhd/
ls -1 *.tif >pages
sed -i -e 's/.tif//g' pages
cat pages
```

```
mkdir -p ~/lang-stopwords
cd ~/lang-stopwords
wget -O frk.stopwords.txt https://raw.githubusercontent.com/stopwords-iso/stopwords-de/master/stopwords-de.txt
```
Edit ~/lang-files/stopwords/frk.stopwords.txt as 
wordacc uses a space delimited stopwords file, not line delimited.

```
sed -i -e 's/\n/ /g' frk.stopwords.txt
cat frk.stopwords.txt
```

### Step 5: run langtests/runlangtests.sh with the root ISRI data dir, testname, tessdata-dir, langusge code:

```
langtests/runlangtests.sh ~/lang-files 4_fast_frk ../tessdata_fast frk
langtests/runlangtests.sh ~/lang-files 4_best_frk ../tessdata_best frk
langtests/runlangtests.sh ~/lang-files 4_best_int_frk ../tessdata frk

 bash -x langtests/runlangtests.sh ~/lang-files 4_fast_Fraktur ../tessdata_fast/script Fraktur

```
and go to the gym, have lunch etc. It takes a while to run.

### Step 6: There should be a RELEASE.summary file
*langtests/reports/4-beta_fast.summary* that contains the final summarized accuracy

```

#### Notes from Nick White regarding wordacc

If you just want to remove all lines which have 100% recognition,
you can add a 'awk' command like this:

ocrevalutf8 wordacc ground.txt ocr.txt | awk '$3 != 100 {print $0}'  
results.txt

or if you've already got a results file you want to change, you can do this:

awk '$3 != 100 {print $0}'  results.txt  newresults.txt

If you only want the last sections where things are broken down by
word, you can add a sed commend, like this:

ocrevalutf8 wordacc ground.txt ocr.txt | sed '/^   Count   Missed %Right   $/,$ 
!d' | awk '$3 != 100 {print $0}'  results.txt
