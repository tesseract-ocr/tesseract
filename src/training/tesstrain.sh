#!/bin/bash
# (C) Copyright 2014, Google Inc.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# This script provides an easy way to execute various phases of training
# Tesseract.  For a detailed description of the phases, see
# https://tesseract-ocr.github.io/tessdoc/Training-Tesseract.html.
#

display_usage() {
echo -e 'USAGE: tesstrain.sh
     --exposures EXPOSURES      # A list of exposure levels to use (e.g. "-1 0 1").
     --fontlist FONTS           # A list of fontnames to train on.
     --fonts_dir FONTS_PATH     # Path to font files.
     --lang LANG_CODE           # ISO 639 code.
     --langdata_dir DATADIR     # Path to tesseract/training/langdata directory.
     --linedata_only            # Only generate training data for lstmtraining.
     --output_dir OUTPUTDIR     # Location of output traineddata file.
     --overwrite                # Safe to overwrite files in output_dir.
     --run_shape_clustering     # Run shape clustering (use for Indic langs).
     --maxpages                 # Specify maximum pages to output (default:0=all)
     --save_box_tiff            # Save box/tiff pairs along with lstmf files.
     --xsize                    # Specify width of output image (default:3600)

  OPTIONAL flag for specifying directory with user specified box/tiff pairs.
  Files should be named similar to ${LANG_CODE}.${fontname}.exp${EXPOSURE}.box/tif
     --my_boxtiff_dir MY_BOXTIFF_DIR # Location of user specified box/tiff files.

  OPTIONAL flags for input data. If unspecified we will look for them in
  the langdata_dir directory.
     --training_text TEXTFILE   # Text to render and use for training.
     --wordlist WORDFILE        # Word list for the language ordered by
                                # decreasing frequency.
  OPTIONAL flag to specify location of existing traineddata files, required
  during feature extraction. If unspecified will use TESSDATA_PREFIX defined in
  the current environment.
     --tessdata_dir TESSDATADIR     # Path to tesseract/tessdata directory.
  NOTE:
  The font names specified in --fontlist need to be recognizable by Pango using
  fontconfig. An easy way to list the canonical names of all fonts available on
  your system is to run text2image with --list_available_fonts and the
  appropriate --fonts_dir path.'
}

source "$(dirname $0)/tesstrain_utils.sh"
if [[ $# -eq 0 || "$1" == "--help" || "$1" == "-h" ]]; then
    display_usage
    exit 0
fi
if [ $# == 0 ]; then
    display_usage
    exit 1
fi

ARGV=("$@")
parse_flags

mkdir -p ${TRAINING_DIR}

if [[ ${MY_BOXTIFF_DIR} != "" ]]; then
    tlog "\n=== Copy existing box/tiff pairs from '${MY_BOXTIFF_DIR}'"
    cp  ${MY_BOXTIFF_DIR}/*.box ${TRAINING_DIR} | true
    cp  ${MY_BOXTIFF_DIR}/*.tif ${TRAINING_DIR} | true
    ls -l  ${TRAINING_DIR}
fi

tlog "\n=== Starting training for language '${LANG_CODE}'"

source "$(dirname $0)/language-specific.sh"
set_lang_specific_parameters ${LANG_CODE}

initialize_fontconfig

phase_I_generate_image 8
phase_UP_generate_unicharset
if $LINEDATA; then
  phase_E_extract_features " --psm 6  lstm.train " 8 "lstmf"
  make__lstmdata
  tlog "\nCreated starter traineddata for LSTM training of language '${LANG_CODE}'\n"
  tlog "\nRun 'lstmtraining' command to continue LSTM training for language '${LANG_CODE}'\n"
else
  phase_D_generate_dawg
  phase_E_extract_features "box.train" 8 "tr"
  phase_C_cluster_prototypes "${TRAINING_DIR}/${LANG_CODE}.normproto"
  phase_S_cluster_shapes
  phase_M_cluster_microfeatures
  phase_B_generate_ambiguities
  make__traineddata
  tlog "\nCompleted training for language '${LANG_CODE}'\n"
fi
