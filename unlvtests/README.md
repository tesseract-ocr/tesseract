## How to run UNLV tests.

The scripts in this directory make it possible to duplicate the tests
published in the Fourth Annual Test of OCR Accuracy.
See http://www.expervision.com/wp-content/uploads/2012/12/1995.The_Fourth_Annual_Test_of_OCR_Accuracy.pdf
but first you have to get the tools and data used by  UNLV:

### Step 1: to download the images go to
https://sourceforge.net/projects/isri-ocr-evaluation-tools-alt/files/
and get doe3.3B.tar.gz, bus.3B.tar.gz, mag.3B.tar.gz and news.3B.tar.gz
spn.3B.tar.gz is incorrect in this repo, so get it from code.google

```
mkdir -p ~/isri-downloads
cd ~/isri-downloads
curl  -L https://sourceforge.net/projects/isri-ocr-evaluation-tools-alt/files/bus.3B.tar.gz > bus.3B.tar.gz
curl  -L https://sourceforge.net/projects/isri-ocr-evaluation-tools-alt/files/doe3.3B.tar.gz > doe3.3B.tar.gz
curl  -L https://sourceforge.net/projects/isri-ocr-evaluation-tools-alt/files/mag.3B.tar.gz > mag.3B.tar.gz
curl  -L https://sourceforge.net/projects/isri-ocr-evaluation-tools-alt/files/news.3B.tar.gz > news.3B.tar.gz
curl  -L https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/isri-ocr-evaluation-tools/spn.3B.tar.gz > spn.3B.tar.gz
```

### Step 2: extract the files.
It doesn't really matter where
in your filesystem you put them, but they must go under a common
root so you have directories doe3.3B, bus.3B, mag.3B and news.3B. in, for example,
~/ISRI-OCRtk.

```
mkdir -p ~/ISRI-OCRtk
cd ~/ISRI-OCRtk
tar xzvf ~/isri-downloads/bus.3B.tar.gz
tar xzvf ~/isri-downloads/doe3.3B.tar.gz
tar xzvf ~/isri-downloads/mag.3B.tar.gz
tar xzvf ~/isri-downloads/news.3B.tar.gz
tar xzvf ~/isri-downloads/spn.3B.tar.gz
mkdir -p stopwords
cd stopwords
wget -O spa.stopwords.txt https://raw.githubusercontent.com/stopwords-iso/stopwords-es/master/stopwords-es.txt
```
Edit ~/ISRI-OCRtk/stopwords/spa.stopwords.txt
wordacc uses a space delimited stopwords file, not line delimited.
s/\n/ /g

Edit ~/ISRI-OCRtk/spn.3B/pages
Delete the line containing the following imagename as it [crashes tesseract](https://github.com/tesseract-ocr/tesseract/issues/1647#issuecomment-395954717).

7733_005.3B 3

### Step 3: Download the modified ISRI toolkit, make and install the tools :
These will be installed in /usr/local/bin.

```
git clone https://github.com/Shreeshrii/ocr-evaluation-tools.git
cd ~/ocr-evaluation-tools
sudo make install
```

### Step 4: cd back to your main tesseract-ocr dir and Build tesseract.

### Step 5: run unlvtests/runalltests.sh with the root ISRI data dir, testname, tessdata-dir:

```
unlvtests/runalltests.sh ~/ISRI-OCRtk 4_fast_eng ../tessdata_fast
```
and go to the gym, have lunch etc. It takes a while to run.

### Step 6: There should be a RELEASE.summary file
*unlvtests/reports/4-beta_fast.summary* that contains the final summarized accuracy
report and comparison with the 1995 results.

### Step 7: run the test for Spanish.

```
unlvtests/runalltests_spa.sh ~/ISRI-OCRtk 4_fast_spa ../tessdata_fast
```

#### Notes from Nick White regarding wordacc

If you just want to remove all lines which have 100% recognition,
you can add a 'awk' command like this:

ocrevalutf8 wordacc ground.txt ocr.txt | awk '$3 != 100 {print $0}'
results.txt

or if you've already got a results file you want to change, you can do this:

awk '$3 != 100 {print $0}'  results.txt  newresults.txt

If you only want the last sections where things are broken down by
word, you can add a sed command, like this:

ocrevalutf8 wordacc ground.txt ocr.txt | sed '/^   Count   Missed %Right   $/,$
!d' | awk '$3 != 100 {print $0}'  results.txt
