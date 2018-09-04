# Language tests.
The scripts in this directory make it possible to test Accuracy of Tesseract for different languages. 
## Setup
### Step 1: If not already installed, download the modified ISRI toolkit, 
make and install the tools in /usr/local/bin.
```
git clone https://github.com/Shreeshrii/ocr-evaluation-tools.git
cd ~/ocr-evaluation-tools
sudo make install
```
### Step 2: If not alrady built, Build tesseract.
Use binaries from the tesseract/src/api and tesseract/src/training directory.
### Step 3
Download images and corresponding ground truth  text for the language to be tested.
Each testset should have only one kind of images (eg. tif, png, jpg etc).
The ground truth text files should have the same base filename with txt extension.
As needed, modify the filenames and create the `pages` file for each testset.
Instructions for testing Fraktur and Sanskrit languages are given below as an example.
## Testing for Fraktur - frk and script/Fraktur
### Download the images and groundtruth, modify to required format.
```
bash -x frk_setup.sh
```
### Run tests for Fraktur - frk and script/Fraktur
```
bash -x frk_test.sh
```
## Testing for Sanskrit - san and script/Devanagari
### Download the images and groundtruth, modify to required format.
```
bash -x deva_setup.sh
```
### Run tests 
```
bash -x deva_test.sh
```

### Notes from Nick White regarding wordacc

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
