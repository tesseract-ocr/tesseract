How to run UNLV tests.

The scripts in this directory make it possible to duplicate the tests
published in the Fourth Annual Test of OCR Accuracy.
See http://www.isri.unlv.edu/downloads/AT-1995.pdf
but first you have to get the tools and data used by  UNLV:

Step 1: to download the images goto
https://sourceforge.net/projects/isri-ocr-evaluation-tools-alt/files/ 
and get doe3.3B.tar.gz, bus.3B.tar.gz, mag.3B.tar.gz and news.3B.tar.gz

mkdir -p ~/isri-downloads
cd ~/isri-downloads
curl  -L https://sourceforge.net/projects/isri-ocr-evaluation-tools-alt/files/bus.3B.tar.gz > bus.3B.tar.gz
curl  -L https://sourceforge.net/projects/isri-ocr-evaluation-tools-alt/files/doe3.3B.tar.gz > doe3.3B.tar.gz
curl  -L https://sourceforge.net/projects/isri-ocr-evaluation-tools-alt/files/mag.3B.tar.gz > mag.3B.tar.gz
curl  -L https://sourceforge.net/projects/isri-ocr-evaluation-tools-alt/files/news.3B.tar.gz > news.3B.tar.gz

Step 2: extract the files. It doesn't really matter where
in your filesystem you put them, but they must go under a common
root so you have directories doe3.3B, bus.3B, mag.3B and news.3B. in, for example,
~/ISRI-OCRtk.

mkdir -p ~/ISRI-OCRtk
cd ~/ISRI-OCRtk
tar xzvf ~/isri-downloads/bus.3B.tar.gz
tar xzvf ~/isri-downloads/doe3.3B.tar.gz
tar xzvf ~/isri-downloads/mag.3B.tar.gz
tar xzvf ~/isri-downloads/news.3B.tar.gz

Step 4: Download the modified ISRI toolkit from:
https://ancientgreekocr.org/ocr-evaluation-tools.git

make and install the tools in unlvtests/ocreval/bin by
`make PREFIX=~/tesseract/unlvtests/ocreval install`

Step 6: cd back to your main tesseract-ocr dir and Build tesseract.

Step 7: run unlvtests/runalltests.sh with the root ISRI data dir and testname:
unlvtests/runalltests.sh ~/ISRI-OCRtk tess4.0.0-beta.1
and go to the gym, have lunch etc.

Step 8: There should be a file
unlvtests/reports/tess4.0.0-beta.1.summary that contains the final summarized accuracy
report and comparison with the 1995 results.
