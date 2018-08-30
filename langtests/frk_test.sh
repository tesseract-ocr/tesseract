#!/bin/bash
#
# run langtests/runlangtests.sh with the root ISRI data dir, testname, tessdata-dir, language code:

cd ~/tesseract
langtests/runlangtests.sh ~/lang-files 4_fast_Fraktur ../tessdata_fast/script Fraktur

langtests/runlangtests.sh ~/lang-files 4_fast_frk ../tessdata_fast frk
langtests/runlangtests.sh ~/lang-files 4_best_int_frk ../tessdata frk
langtests/runlangtests.sh ~/lang-files 4_best_frk ../tessdata_best frk

### It takes a while to run.

