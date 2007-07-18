How to run UNLV tests.

The scripts in this directory make it possible to duplicate the tests
published in the Fourth Annual Test of OCR Accuracy.
See http://www.isri.unlv.edu/downloads/AT-1995.pdf
but first you have to get the tools and data from UNLV:

Step 1: to download the images goto
http://www.isri.unlv.edu/ISRI/OCRtk
and get 3b.tgz, Bb.tgz, Mb.tgz and Nb.tgz.

Step 2: extract the files. It doesn't really matter where
in your filesystem you put them, but they must go under a common
root so you have directories 3, B, M and N in, for example,
/users/me/ISRI-OCRtk.

Step 3: Reorg the files
The lack of tif extensions on the images is inconvenient, so there
is a script to reorganize the data to match the rest of the test
scripts.
cd to /users/me/ISRI-OCRtk or wherever 3, B, M and N ended up and run
/blah/blah/tesseract-ocr/testing/reorgdata.sh 3B
This makes directories doe3.3B, bus.3B, mag.3B and news.3B.
You can now get rid of 3, B, M, and N unless you want to get some of the
other scanning resolutions out of them.

Step 4: Download the ISRI toolkit from:
http://www.isri.unlv.edu/downloads/ftk-1.0.tgz

Step 5: If they work for you, use the binaries directly from the bin
directory and put them in tesseract-ocr/testing/unlv
otherwise build the tools for yourself and put them there.

Step 6: cd back to your main tesseract-ocr dir and Build tesseract.

Step 7: run testing/runalltests.sh with the root data dir and testname:
testing/runalltests.sh /users/me/ISRI-OCRtk tess2.0
and go to the gym, have lunch etc.

Step 8: There should be a file
testing/reports/tess2.0.summary that contains the final summarized accuracy
report and comparison with the 1995 results.

