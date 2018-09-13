#!/bin/bash
# run langtests/runlangtests.sh with the root data dir, testname, tessdata-dir, language code and image extension

cd ~/tesseract

langtests/runlangtests.sh ~/lang-files 4_fast_Devanagari ../tessdata_fast/script Devanagari png
langtests/runlangtests.sh ~/lang-files 4_best_int_Devanagari ../tessdata/script Devanagari png
langtests/runlangtests.sh ~/lang-files 4_best_Devanagari ../tessdata_best/script Devanagari png
langtests/runlangtests.sh ~/lang-files 4_fast_san ../tessdata_fast san png
langtests/runlangtests.sh ~/lang-files 4_best_int_san ../tessdata san png
langtests/runlangtests.sh ~/lang-files 4_best_san ../tessdata_best san png

langtests/runlangtests.sh ~/lang-files    4_plus40k_san    ../tesstutorial-deva    san    png

#/home/ubuntu/tesstutorial-deva/san.traineddata at n iterations

### It takes a while to run.

