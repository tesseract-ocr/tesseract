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
# https://code.google.com/p/tesseract-ocr/wiki/TrainingTesseract3
#
# USAGE:
#
# tesstrain.sh
#    --bin_dir PATH             # Location of training program.
#    --fontlist FONTS_STR       # A plus-separated list of fontnames to train on.
#    --fonts_dir FONTS_PATH     # Path to font files.
#    --lang LANG_CODE           # ISO 639 code.
#    --langdata_dir DATADIR     # Path to tesseract/training/langdata directory.
#    --output_dir OUTPUTDIR     # Location of output traineddata file.
#    --overwrite                # Safe to overwrite files in output_dir.
#    --run_shape_clustering     # Run shape clustering (use for Indic langs).
#
# OPTIONAL flags for input data. If unspecified we will look for them in
# the langdata_dir directory.
#    --training_text TEXTFILE   # Text to render and use for training.
#    --wordlist WORDFILE        # Word list for the language ordered by
#                               # decreasing frequency.
#
# OPTIONAL flag to specify location of existing traineddata files, required
# during feature extraction. If unspecified will use TESSDATA_PREFIX defined in
# the current environment.
#    --tessdata_dir TESSDATADIR     # Path to tesseract/tessdata directory.
#
# NOTE:
# The font names specified in --fontlist need to be recognizable by Pango using
# fontconfig. An easy way to list the canonical names of all fonts available on
# your system is to run text2image with --list_available_fonts and the
# appropriate --fonts_dir path.


FONTS=(
    "Arial" \
    "Times New Roman," \
)
FONTS_DIR="/usr/share/fonts/truetype/"
OUTPUT_DIR="/tmp/tesstrain/tessdata"
OVERWRITE=0
RUN_SHAPE_CLUSTERING=0
EXTRACT_FONT_PROPERTIES=1
WORKSPACE_DIR="/tmp/tesstrain"


# Logging helper functions.
tlog() {
    echo -e $* 2>&1 1>&2 | tee -a ${LOG_FILE}
}

err() {
    echo -e "ERROR: "$* 2>&1 1>&2 | tee -a ${LOG_FILE}
    exit 1
}

# Helper function to run a command and append its output to a log. Aborts early
# if the program file is not found.
# Usage: run_cmd CMD ARG1 ARG2...
run_cmd() {
    local cmd=$1
    shift
    if [[ ! -x ${cmd} ]]; then
        err "File ${cmd} not found"
    fi
    tlog "[$(date)] ${cmd} $@"
    ${cmd} "$@" 2>&1 1>&2 | tee -a ${LOG_FILE}
    # check completion status
    if [[ $? -gt 0 ]]; then
        err "Program $(basename ${cmd}) failed. Abort."
    fi
}

# Check if all the given files exist, or exit otherwise.
# Used to check required input files and produced output files in each phase.
# Usage: check_file_readable FILE1 FILE2...
check_file_readable() {
    for file in $@; do
        if [[ ! -r ${file} ]]; then
            err "${file} does not exist or is not readable"
        fi
    done
}


# Write a file (with name specified in $2) with records that account for
# n% (specified in $3) of the total weights of records in the input file
# (input file name specified in $1). The input file should have one record
# per line along with its weight separated by \t. The records should be
# sorted in non-ascending order of frequency.
# If $4 is true the first record is skipped.
# USAGE: discard_tail INPUT_FILE OUTPUT_FILE PERCENTAGE
discard_tail() {
    local infile=$1
    local outfile=$2
    local pct=$3
    local skip_first=$4

    local more_arg="1";
    if [[ ${skip_first} ]]; then
        more_arg="2"
    fi
    local sum=$(tail -n +${more_arg} ${infile} \
        | awk 'BEGIN {FS = "\t"} {if ($1 != " ") {s=s+$2}}; END {print s}')
    if [[ ${sum} == "" ]]; then sum=0
    fi
    local limit=$((${sum}*${pct}/100))
    tail -n +${more_arg} ${infile} | awk 'BEGIN {FS = "\t"}
        {if (s > 0) {print $1; if ($1 != " ") {s=s-$2;}}}' s=${limit} \
            >> ${outfile}
}


# Set global path variables that are based on parsed flags.
set_prog_paths() {
    if [[ -z ${BINDIR} ]]; then
        err "Need to specify location of program files"
    fi
    CN_TRAINING_EXE=${BINDIR}/cntraining
    COMBINE_TESSDATA_EXE=${BINDIR}/combine_tessdata
    MF_TRAINING_EXE=${BINDIR}/mftraining
    SET_UNICHARSET_PROPERTIES_EXE=${BINDIR}/set_unicharset_properties
    SHAPE_TRAINING_EXE=${BINDIR}/shapeclustering
    TESSERACT_EXE=${BINDIR}/tesseract
    TEXT2IMAGE_EXE=${BINDIR}/text2image
    UNICHARSET_EXTRACTOR_EXE=${BINDIR}/unicharset_extractor
    WORDLIST2DAWG_EXE=${BINDIR}/wordlist2dawg
}

# Sets the named variable to given value. Aborts if the value is missing or
# if it looks like a flag.
# Usage: parse_value VAR_NAME VALUE
parse_value() {
    local val="$2"
    if [[ -z $val ]]; then
        err "Missing value for variable $1"
        exit
    fi
    if [[ ${val:0:2} == "--" ]]; then
        err "Invalid value $val passed for variable $1"
        exit
    fi
    eval $1=\"$val\"
}

# Does simple command-line parsing and initialization.
parse_flags() {
    local i=0
    while test $i -lt ${#ARGV[@]}; do
        local j=$((i+1))
        case ${ARGV[$i]} in
            --)
                break;;
            --bin_dir)
                parse_value "BINDIR" ${ARGV[$j]}
                i=$j ;;
            --fontlist)   # Expect a plus-separated list of names
                if [[ -z ${ARGV[$j]} ]] || [[ ${ARGV[$j]:0:2} == "--" ]]; then
                    err "Invalid value passed to --fontlist"
                fi
                local ofs=$IFS
                IFS='+'
                FONTS=( ${ARGV[$j]} )
                IFS=$ofs ;;
            --fonts_dir)
                parse_value "FONTS_DIR" ${ARGV[$j]}
                i=$j ;;
            --lang)
                parse_value "LANG_CODE" ${ARGV[$j]}
                i=$j ;;
            --langdata_dir)
                parse_value "LANGDATA_ROOT" ${ARGV[$j]}
                i=$j ;;
            --output_dir)
                parse_value "OUTPUT_DIR" ${ARGV[$j]}
                i=$j ;;
            --overwrite)
                OVERWRITE=1 ;;
            --extract_font_properties)
                EXTRACT_FONT_PROPERTIES=1 ;;
            --noextract_font_properties)
                EXTRACT_FONT_PROPERTIES=0 ;;
            --run_shape_clustering)
                RUN_SHAPE_CLUSTERING=1 ;;
            --tessdata_dir)
                parse_value "TESSDATA_DIR" ${ARGV[$j]}
                i=$j ;;
            --training_text)
                parse_value "TRAINING_TEXT" "${ARGV[$j]}"
                i=$j ;;
            --wordlist)
                parse_value "WORDLIST_FILE" ${ARGV[$j]}
                i=$j ;;
            *)
                err "Unrecognized argument ${ARGV[$i]}" ;;
        esac
        i=$((i+1))
    done
    if [[ -z ${LANG_CODE} ]]; then
        err "Need to specify a language --lang"
    fi
    if [[ -z ${BINDIR} ]]; then
        err "Need to specify path to built binaries --bin_dir"
    fi
    if [[ -z ${LANGDATA_ROOT} ]]; then
        err "Need to specify path to language files --langdata_dir"
    fi
    if [[ -z ${TESSDATA_DIR} ]]; then
        if [[ -z ${TESSDATA_PREFIX} ]]; then
            err "Need to specify a --tessdata_dir or have a "\
        "TESSDATA_PREFIX variable defined in your environment"
        else
            TESSDATA_DIR="${TESSDATA_PREFIX}"
        fi
    fi

    set_prog_paths

    # Location where intermediate files will be created.
    TRAINING_DIR=${WORKSPACE_DIR}/${LANG_CODE}
    # Location of log file for the whole run.
    LOG_FILE=${TRAINING_DIR}/tesstrain.log

    # Take training text and wordlist from the langdata directory if not
    # specified in the commend-line.
    if [[ -z ${TRAINING_TEXT} ]]; then
        TRAINING_TEXT=${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.training_text
    fi
    if [[ -z ${WORDLIST_FILE} ]]; then
        WORDLIST_FILE=${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.wordlist.clean
    fi
    WORD_BIGRAMS_FILE=${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.word.bigrams.clean
    NUMBERS_FILE=${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.numbers
    PUNC_FILE=${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.punc
    BIGRAM_FREQS_FILE=${TRAINING_TEXT}.bigram_freqs
    UNIGRAM_FREQS_FILE=${TRAINING_TEXT}.unigram_freqs
    TRAIN_NGRAMS_FILE=${TRAINING_TEXT}.train_ngrams
}

# Phase I : Generate (I)mages from training text for each font.
phaseI_generate_image() {
    tlog "\n=== Phase I: Generating training images ==="
    if [[ -z ${TRAINING_TEXT} ]] || [[ ! -r ${TRAINING_TEXT} ]]; then
        err "Could not find training text file ${TRAINING_TEXT}"
    fi
    BOX_PADDING="0"
    CHAR_SPACING="0.0"
    EXPOSURE="0"
    LEADING="32"
    NGRAM_CHAR_SPACING="0.0"

    if (( ${EXTRACT_FONT_PROPERTIES} )) && [[ -r ${BIGRAM_FREQS} ]]; then
        # Parse .bigram_freqs file and compose a .train_ngrams file with text
        # for tesseract to recognize during training. Take only the ngrams whose
        # combined weight accounts for 95% of all the bigrams in the language.
        TMP_FILE="${TRAINING_DIR}/_tmp"
        cat ${BIGRAM_FREQS_FILE} > ${TMP_FILE}
        NGRAM_FRAC=$(cat ${BIGRAM_FREQS_FILE} \
            | awk '{s=s+$2}; END {print (s/100)*p}' p=99)
        cat ${BIGRAM_FREQS_FILE} | sort -rnk2 \
            | awk '{s=s+$2; if (s <= x) {printf "%s ", $1; } }' \
            x=${NGRAM_FRAC} > ${TRAIN_NGRAMS_FILE}
        check_file_readable ${TRAIN_NGRAMS_FILE}
    fi

    for font in "${FONTS[@]}"; do
        tlog "Rendering using ${font}"
        fontname=$(echo ${font} | tr ' ' '_' | sed 's/,//g')
        outbase=${TRAINING_DIR}/${LANG_CODE}.${fontname}.exp${EXPOSURE}

        common_args="--leading=${LEADING} --fonts_dir=${FONTS_DIR} "
        common_args+=" --box_padding=${BOX_PADDING} --strip_unrenderable_words"

        run_cmd ${TEXT2IMAGE_EXE} ${common_args} \
            --char_spacing=${CHAR_SPACING} --exposure=${EXPOSURE} \
            --font="${font}" --outputbase=${outbase} --text=${TRAINING_TEXT}
        check_file_readable ${outbase}.box ${outbase}.tif

        if (( ${EXTRACT_FONT_PROPERTIES} )) &&
            [[ -r ${TRAIN_NGRAMS_FILE} ]]; then
            tlog "Rendering ngrams using ${font}"
            outbase=${TRAINING_DIR}/ngrams/${LANG_CODE}.ngrams.${fontname}.exp${EXPOSURE}
            run_cmd ${TEXT2IMAGE_EXE} ${common_args} \
                --char_spacing=${NGRAM_CHAR_SPACING} --exposure=${EXPOSURE} \
                --font="${font}" --outputbase=${outbase} \
                --box_padding=${BOX_PADDING} --render_ngrams=1 \
                --text=${TRAIN_NGRAMS_FILE}
            check_file_readable ${outbase}.box ${outbase}.tif
        fi
    done
}


# Phase UP : Generate (U)nicharset and (P)roperties file.
phaseUP_generate_unicharset() {
    tlog "\n=== Phase UP: Generating unicharset and unichar properties files ==="

    box_files=$(ls ${TRAINING_DIR}/*.box)
    run_cmd ${UNICHARSET_EXTRACTOR_EXE} -D "${TRAINING_DIR}/" ${box_files}
    outfile=${TRAINING_DIR}/unicharset
    UNICHARSET_FILE="${TRAINING_DIR}/${LANG_CODE}.unicharset"
    check_file_readable ${outfile}
    mv ${outfile} ${UNICHARSET_FILE}

    XHEIGHTS_FILE="${TRAINING_DIR}/${LANG_CODE}.xheights"
    check_file_readable ${UNICHARSET_FILE}
    run_cmd ${SET_UNICHARSET_PROPERTIES_EXE} \
        -U ${UNICHARSET_FILE} -O ${UNICHARSET_FILE} -X ${XHEIGHTS_FILE} \
        --script_dir=${LANGDATA_ROOT}
    check_file_readable ${XHEIGHTS_FILE}
}

# Phase D : Generate (D)awg files from unicharset file and wordlist files
phaseD_generate_dawg() {
    tlog "\n=== Phase D: Generating Dawg files ==="
    # Output files
    WORD_DAWG=${TRAINING_DIR}/${LANG_CODE}.word-dawg
    FREQ_DAWG=${TRAINING_DIR}/${LANG_CODE}.freq-dawg
    PUNC_DAWG=${TRAINING_DIR}/${LANG_CODE}.punc-dawg
    NUMBER_DAWG=${TRAINING_DIR}/${LANG_CODE}.number-dawg
    BIGRAM_DAWG=${TRAINING_DIR}/${LANG_CODE}.bigram-dawg

    # Word DAWG
    local freq_wordlist_file=${TRAINING_DIR}/${LANG_CODE}.wordlist.clean.freq
    if [[ -r ${WORDLIST_FILE} ]]; then
        tlog "Generating word Dawg"
        check_file_readable ${UNICHARSET_FILE}
        run_cmd ${WORDLIST2DAWG_EXE} -r 1 ${WORDLIST_FILE} ${WORD_DAWG} \
            ${UNICHARSET_FILE}
        check_file_readable ${WORD_DAWG}

        FREQ_DAWG_SIZE=100
        head -n ${FREQ_DAWG_SIZE} ${WORDLIST_FILE} > ${freq_wordlist_file}
    fi

    # Freq-word DAWG
    if [[ -r ${freq_wordlist_file} ]]; then
        check_file_readable ${UNICHARSET_FILE}
        tlog "Generating frequent-word Dawg"
        run_cmd ${WORDLIST2DAWG_EXE}  -r 1 ${freq_wordlist_file} ${FREQ_DAWG} \
            ${UNICHARSET_FILE}
        check_file_readable ${FREQ_DAWG}
    fi

    # Punctuation DAWG
    local punc_clean="${LANGDATA_ROOT}/common.punc"
    if [[ -r ${PUNC_FILE} ]]; then
        local top_punc_file=${TRAINING_DIR}/${LANG_CODE}.punc.top
        head -n 1 ${PUNC_FILE} | awk 'BEGIN {FS = "\t"} {print $1}' \
            > ${top_punc_file}
        discard_tail ${PUNC_FILE} ${top_punc_file} 99 1
        punc_clean="${top_punc_file}"
    fi
    # -r arguments to WORDLIST2DAWG_EXE denote RTL reverse policy
    # (see Trie::RTLReversePolicy enum in third_party/tesseract/dict/trie.h).
    # We specify 0/RRP_DO_NO_REVERSE when generating number DAWG,
    # 1/RRP_REVERSE_IF_HAS_RTL for freq and word DAWGS,
    # 2/RRP_FORCE_REVERSE for the punctuation DAWG.
    local punc_reverse_policy=0;
    if [[ ${LANG_CODE} == "heb" || ${LANG_CODE} == "ara" ]]; then
        punc_reverse_policy=2
    fi
    if [[ -r ${punc_clean} ]]; then
        run_cmd ${WORDLIST2DAWG_EXE} -r ${punc_reverse_policy} \
            ${punc_clean} ${PUNC_DAWG} ${UNICHARSET_FILE}
        check_file_readable ${PUNC_DAWG}
    fi

    # Numbers DAWG
    if [[ -r ${NUMBERS_FILE} ]]; then
        local top_num_file=${TRAINING_DIR}/${LANG_CODE}.numbers.top
        head -n 1 ${NUMBERS_FILE} | awk 'BEGIN {FS = "\t"} {print $1}' \
            > ${top_num_file}
        discard_tail ${NUMBERS_FILE} ${top_num_file} 85 1
        run_cmd ${WORDLIST2DAWG_EXE} -r 0 \
            ${top_num_file} ${NUMBER_DAWG} ${UNICHARSET_FILE}
        check_file_readable ${NUMBER_DAWG}
    fi

    # Bigram dawg
    if [[ -r ${WORD_BIGRAMS_FILE} ]]; then
        run_cmd ${WORDLIST2DAWG_EXE} -r 1 \
            ${WORD_BIGRAMS_FILE} ${BIGRAM_DAWG} ${UNICHARSET_FILE}
        check_file_readable ${BIGRAM_DAWG}
    fi
}

# Phase E : (E)xtract .tr feature files from .tif/.box files
phaseE_extract_features() {
    tlog "\n=== Phase E: Extracting features ==="
    local box_config="box.train"
    TRAIN_EXPOSURES='0'

    for exposure in ${TRAIN_EXPOSURES}; do
        img_files=${img_files}' '$(ls ${TRAINING_DIR}/*.exp${exposure}.tif)
    done

    # Use any available language-specific configs.
    local config=""
    if [[ -r ${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.config ]]; then
        config=${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.config
    fi

    OLD_TESSDATA_PREFIX=${TESSDATA_PREFIX}
    export TESSDATA_PREFIX=${TESSDATA_DIR}
    tlog "Using TESSDATA_PREFIX=${TESSDATA_PREFIX}"
    for img_file in ${img_files}; do
        run_cmd ${TESSERACT_EXE} ${img_file} ${img_file%.*} \
            ${box_config} ${config}
    done
    export TESSDATA_PREFIX=${OLD_TESSDATA_PREFIX}
}

# Phase C : (C)luster feature prototypes in .tr into normproto file (cnTraining)
# phaseC_cluster_prototypes ${TRAINING_DIR}/${LANG_CODE}.normproto
phaseC_cluster_prototypes() {
    tlog "\n=== Phase C: Clustering feature prototypes (cnTraining) ==="
    local out_normproto=${TRAINING_DIR}/${LANG_CODE}.normproto

    run_cmd ${CN_TRAINING_EXE} -D "${TRAINING_DIR}/" \
        $(ls ${TRAINING_DIR}/*.tr)

    check_file_readable ${TRAINING_DIR}/normproto
    mv ${TRAINING_DIR}/normproto ${out_normproto}
}

# Phase S : (S)hape clustering
phaseS_cluster_shapes() {
    if (( ! ${RUN_SHAPE_CLUSTERING} )); then
        return
    fi
    check_file_readable ${LANGDATA_ROOT}/font_properties
    local font_props=${LANGDATA_ROOT}/font_properties
    if [[ -r ${font_props} ]]; then
        font_props="-F ${font_props}"
    else
        font_props=""
    fi
    if [[ -r ${TRAINING_DIR}/${LANG_CODE}.xheights ]] &&\
     [[ -s ${TRAINING_DIR}/${LANG_CODE}.xheights ]]; then
        font_props=${font_props}" -X ${TRAINING_DIR}/${LANG_CODE}.xheights"
    fi

    run_cmd ${SHAPE_TRAINING_EXE} \
        -D "${TRAINING_DIR}/" \
        -U ${TRAINING_DIR}/${LANG_CODE}.unicharset \
        -O ${TRAINING_DIR}/${LANG_CODE}.mfunicharset \
        ${font_props} \
        $(ls ${TRAINING_DIR}/*.tr)
    check_file_readable ${TRAINING_DIR}/shapetable \
        ${TRAINING_DIR}/${LANG_CODE}.mfunicharset
}

# Phase M : Clustering microfeatures (mfTraining)
phaseM_cluster_microfeatures() {
    tlog "\n=== Phase M : Clustering microfeatures (mfTraining) ==="

    font_props=${LANGDATA_ROOT}/font_properties
    if [[ -r ${font_props} ]]; then
        font_props="-F ${font_props}"
    else
        font_props=""
    fi
    if [[ -r ${TRAINING_DIR}/${LANG_CODE}.xheights ]] && \
       [[ -s ${TRAINING_DIR}/${LANG_CODE}.xheights ]]; then
        font_props=${font_props}" -X ${TRAINING_DIR}/${LANG_CODE}.xheights"
    fi

    run_cmd ${MF_TRAINING_EXE} \
        -D "${TRAINING_DIR}/" \
        -U ${TRAINING_DIR}/${LANG_CODE}.unicharset \
        -O ${TRAINING_DIR}/${LANG_CODE}.mfunicharset \
        ${font_props} \
        $(ls ${TRAINING_DIR}/*.tr)
    check_file_readable ${TRAINING_DIR}/inttemp ${TRAINING_DIR}/shapetable \
        ${TRAINING_DIR}/pffmtable ${TRAINING_DIR}/${LANG_CODE}.mfunicharset
    mv ${TRAINING_DIR}/inttemp ${TRAINING_DIR}/${LANG_CODE}.inttemp
    mv ${TRAINING_DIR}/shapetable ${TRAINING_DIR}/${LANG_CODE}.shapetable
    mv ${TRAINING_DIR}/pffmtable ${TRAINING_DIR}/${LANG_CODE}.pffmtable
    mv ${TRAINING_DIR}/${LANG_CODE}.mfunicharset ${TRAINING_DIR}/${LANG_CODE}.unicharset
}

phaseB_generate_ambiguities() {
  tlog "\n=== Phase B : ambiguities training ==="

  # Check for manually created ambiguities data.
  if [[ -r ${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.unicharambigs ]]; then
      tlog "Found file ${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.unicharambigs"
      cp ${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.unicharambigs \
          ${TRAINING_DIR}/${LANG_CODE}.unicharambigs
      # Make it writable, as it may be read-only in the client.
      chmod u+w ${TRAINING_DIR}/${LANG_CODE}.unicharambigs
      return
  else
      tlog "No unicharambigs file found!"
  fi

  # TODO: Add support for generating ambiguities automatically.
}


make_traineddata() {
  tlog "\n=== Making final traineddata file ==="
  local lang_prefix=${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}

  # Combine available files for this language from the langdata dir.
  if [[ -r ${lang_prefix}.config ]]; then
    tlog "Copying ${lang_prefix}.config to ${TRAINING_DIR}"
    cp ${lang_prefix}.config ${TRAINING_DIR}
    chmod u+w ${TRAINING_DIR}/${LANG_CODE}.config
  fi
  if [[ -r ${lang_prefix}.cube-unicharset ]]; then
    tlog "Copying ${lang_prefix}.cube-unicharset to ${TRAINING_DIR}"
    cp ${lang_prefix}.cube-unicharset ${TRAINING_DIR}
    chmod u+w ${TRAINING_DIR}/${LANG_CODE}.cube-unicharset
  fi
  if [[ -r ${lang_prefix}.cube-word-dawg ]]; then
    tlog "Copying ${lang_prefix}.cube-word-dawg to ${TRAINING_DIR}"
    cp ${lang_prefix}.cube-word-dawg ${TRAINING_DIR}
    chmod u+w ${TRAINING_DIR}/${LANG_CODE}.cube-word-dawg
  fi
  if [[ -r ${lang_prefix}.params-model ]]; then
    tlog "Copying ${lang_prefix}.params-model to ${TRAINING_DIR}"
    cp ${lang_prefix}.params-model ${TRAINING_DIR}
    chmod u+w ${TRAINING_DIR}/${LANG_CODE}.params-model
  fi

  # Compose the traineddata file.
  run_cmd ${COMBINE_TESSDATA_EXE} ${TRAINING_DIR}/${LANG_CODE}.

  # Copy it to the output dir, overwriting only if allowed by the cmdline flag.
  if [[ ! -d ${OUTPUT_DIR} ]]; then
      tlog "Creating new directory ${OUTPUT_DIR}"
      mkdir -p ${OUTPUT_DIR}
  fi
  local destfile=${OUTPUT_DIR}/${LANG_CODE}.traineddata;
  if [[ -f ${destfile} ]] && (( ! ${OVERWRITE} )); then
      err "File ${destfile} exists and no --overwrite specified";
  fi
  tlog "Moving ${TRAINING_DIR}/${LANG_CODE}.traineddata to ${OUTPUT_DIR}"
  cp -f ${TRAINING_DIR}/${LANG_CODE}.traineddata ${destfile}
}


ARGV=("$@")
parse_flags

tlog "\n=== Starting training for language '${LANG_CODE}'"

tlog "Cleaning workspace directory ${TRAINING_DIR}..."
mkdir -p ${TRAINING_DIR}
rm -fr ${TRAINING_DIR}/*

phaseI_generate_image
phaseUP_generate_unicharset
phaseD_generate_dawg
phaseE_extract_features
phaseC_cluster_prototypes
phaseS_cluster_shapes
phaseM_cluster_microfeatures
phaseB_generate_ambiguities
make_traineddata

tlog "\nCompleted training for language '${LANG_CODE}'\n"
