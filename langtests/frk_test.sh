#!/bin/bash
#
# run langtests/runlangtests.sh with the root ISRI data dir, testname, tessdata-dir, language code:

cd ~/tesseract
langtests/runlangtests.sh ~/lang-files 4_fast_Fraktur ../tessdata_fast/script Fraktur tif

langtests/runlangtests.sh ~/lang-files 4_fast_frk ../tessdata_fast frk tif
langtests/runlangtests.sh ~/lang-files 4_best_int_frk ../tessdata frk tif
langtests/runlangtests.sh ~/lang-files 4_best_frk ../tessdata_best frk tif

### It takes a while to run.

