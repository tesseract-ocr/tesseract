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
# This script defines functions that are used by tesstrain.sh
# For a detailed description of the phases, see
# https://github.com/tesseract-ocr/tesseract/wiki/TrainingTesseract
#
# USAGE: source tesstrain_utils.sh

FONTS=(
    "Arial" \
    "Times New Roman," \
)
if [ "$(uname)" == "Darwin" ];then
    FONTS_DIR="/Library/Fonts/"
else
    FONTS_DIR="/usr/share/fonts/"
fi
OUTPUT_DIR="/tmp/tesstrain/tessdata"
OVERWRITE=0
RUN_SHAPE_CLUSTERING=0
EXTRACT_FONT_PROPERTIES=1
WORKSPACE_DIR="/tmp/tesstrain"

# Logging helper functions.
tlog() {
    echo -e $* 2>&1 1>&2 | tee -a ${LOG_FILE}
}

err_exit() {
    echo -e "ERROR: "$* 2>&1 1>&2 | tee -a ${LOG_FILE}
    exit 1
}

# Helper function to run a command and append its output to a log. Aborts early
# if the program file is not found.
# Usage: run_command CMD ARG1 ARG2...
run_command() {
    local cmd=$1
    shift
    if [[ ! -x ${cmd} ]]; then
        err_exit "File ${cmd} not found"
    fi
    tlog "[$(date)] ${cmd} $@"
    ${cmd} "$@" 2>&1 1>&2 | tee -a ${LOG_FILE}
    # check completion status
    if [[ $? -gt 0 ]]; then
        err_exit "Program $(basename ${cmd}) failed. Abort."
    fi
}

# Check if all the given files exist, or exit otherwise.
# Used to check required input files and produced output files in each phase.
# Usage: check_file_readable FILE1 FILE2...
check_file_readable() {
    for file in $@; do
        if [[ ! -r ${file} ]]; then
            err_exit "${file} does not exist or is not readable"
        fi
    done
}

# Set global path variables that are based on parsed flags.
set_prog_paths() {
    if [[ -z ${BINDIR} ]]; then
        err_exit "Need to specify location of program files"
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
        err_exit "Missing value for variable $1"
        exit
    fi
    if [[ ${val:0:2} == "--" ]]; then
        err_exit "Invalid value $val passed for variable $1"
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
                    err_exit "Invalid value passed to --fontlist"
                fi
                local ofs=$IFS
                IFS='+'
                FONTS=( ${ARGV[$j]} )
                IFS=$ofs
                i=$j ;;
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
                err_exit "Unrecognized argument ${ARGV[$i]}" ;;
        esac
        i=$((i+1))
    done
    if [[ -z ${LANG_CODE} ]]; then
        err_exit "Need to specify a language --lang"
    fi
    if [[ -z ${BINDIR} ]]; then
        err_exit "Need to specify path to built binaries --bin_dir"
    fi
    if [[ -z ${LANGDATA_ROOT} ]]; then
        err_exit "Need to specify path to language files --langdata_dir"
    fi
    if [[ -z ${TESSDATA_DIR} ]]; then
        if [[ -z ${TESSDATA_PREFIX} ]]; then
            err_exit "Need to specify a --tessdata_dir or have a "\
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
    # specified in the command-line.
    if [[ -z ${TRAINING_TEXT} ]]; then
        TRAINING_TEXT=${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.training_text
    fi
    if [[ -z ${WORDLIST_FILE} ]]; then
        WORDLIST_FILE=${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.wordlist
    fi
    WORD_BIGRAMS_FILE=${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.word.bigrams
    NUMBERS_FILE=${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.numbers
    PUNC_FILE=${LANGDATA_ROOT}/${LANG_CODE}/${LANG_CODE}.punc
    BIGRAM_FREQS_FILE=${TRAINING_TEXT}.bigram_freqs
    UNIGRAM_FREQS_FILE=${TRAINING_TEXT}.unigram_freqs
    TRAIN_NGRAMS_FILE=${TRAINING_TEXT}.train_ngrams
    GENERATE_DAWGS=1
}

# Function initializes font config with a unique font cache dir.
initialize_fontconfig() {
    export FONT_CONFIG_CACHE=$(mktemp -d --tmpdir font_tmp.XXXXXXXXXX)
    local sample_path=${FONT_CONFIG_CACHE}/sample_text.txt
    echo "Text" >${sample_path}
    run_command ${TEXT2IMAGE_EXE} --fonts_dir=${FONTS_DIR} \
        --font="Arial" --outputbase=${sample_path} --text=${sample_path} \
        --fontconfig_tmpdir=${FONT_CONFIG_CACHE}
}

# Helper function for phaseI_generate_image. Generates the image for a single
# language/font combination in a way that can be run in parallel.
generate_font_image() {
    local font="$1"
    tlog "Rendering using ${font}"
    local fontname=$(echo ${font} | tr ' ' '_' | sed 's/,//g')
    local outbase=${TRAINING_DIR}/${LANG_CODE}.${fontname}.exp${EXPOSURE}

    local common_args="--fontconfig_tmpdir=${FONT_CONFIG_CACHE}"
    common_args+=" --fonts_dir=${FONTS_DIR} --strip_unrenderable_words"
    common_args+=" --fontconfig_refresh_config_file=false --leading=${LEADING}"
    common_args+=" --char_spacing=${CHAR_SPACING} --exposure=${EXPOSURE}"
    common_args+=" --outputbase=${outbase}"

    # add --writing_mode=vertical-upright to common_args if the font is
    # specified to be rendered vertically.
    for vfont in "${VERTICAL_FONTS[@]}"; do
      if [[ "${font}" == "${vfont}" ]]; then
        common_args+=" --writing_mode=vertical-upright "
        break
      fi
    done

    run_command ${TEXT2IMAGE_EXE} ${common_args} --font="${font}" \
        --text=${TRAINING_TEXT} ${TEXT2IMAGE_EXTRA_ARGS}
    check_file_readable ${outbase}.box ${outbase}.tif

    if (( ${EXTRACT_FONT_PROPERTIES} )) &&
        [[ -r ${TRAIN_NGRAMS_FILE} ]]; then
        tlog "Extracting font properties of ${font}"
        run_command ${TEXT2IMAGE_EXE} ${common_args} --font="${font}" \
            --ligatures=false --text=${TRAIN_NGRAMS_FILE} \
            --only_extract_font_properties --ptsize=32
        check_file_readable ${outbase}.fontinfo
    fi
}


# Phase I : Generate (I)mages from training text for each font.
phase_I_generate_image() {
    local par_factor=$1
    if [[ -z ${par_factor} || ${par_factor} -le 0 ]]; then
        par_factor=1
    fi
    tlog "\n=== Phase I: Generating training images ==="
    if [[ -z ${TRAINING_TEXT} ]] || [[ ! -r ${TRAINING_TEXT} ]]; then
        err_exit "Could not find training text file ${TRAINING_TEXT}"
    fi
    CHAR_SPACING="0.0"
    EXPOSURE="0"

    if (( ${EXTRACT_FONT_PROPERTIES} )) && [[ -r ${BIGRAM_FREQS_FILE} ]]; then
        # Parse .bigram_freqs file and compose a .train_ngrams file with text
        # for tesseract to recognize during training. Take only the ngrams whose
        # combined weight accounts for 95% of all the bigrams in the language.
        NGRAM_FRAC=$(cat ${BIGRAM_FREQS_FILE} \
            | awk '{s=s+$2}; END {print (s/100)*p}' p=99)
        cat ${BIGRAM_FREQS_FILE} | sort -rnk2 \
            | awk '{s=s+$2; if (s <= x) {printf "%s ", $1; } }' \
            x=${NGRAM_FRAC} > ${TRAIN_NGRAMS_FILE}
        check_file_readable ${TRAIN_NGRAMS_FILE}
    fi

    local counter=0
    for font in "${FONTS[@]}"; do
        generate_font_image "${font}" &
        let counter=counter+1
        let rem=counter%par_factor
        if [[ "${rem}" -eq 0 ]]; then
          wait
        fi
    done
    wait
    # Check that each process was successful.
    for font in "${FONTS[@]}"; do
        local fontname=$(echo ${font} | tr ' ' '_' | sed 's/,//g')
        local outbase=${TRAINING_DIR}/${LANG_CODE}.${fontname}.exp${EXPOSURE}
        check_file_readable ${outbase}.box ${outbase}.tif
    done
}

# Phase UP : Generate (U)nicharset and (P)roperties file.
phase_UP_generate_unicharset() {
    tlog "\n=== Phase UP: Generating unicharset and unichar properties files ==="

    local box_files=$(ls ${TRAINING_DIR}/*.box)
    run_command ${UNICHARSET_EXTRACTOR_EXE} -D "${TRAINING_DIR}/" ${box_files}
    local outfile=${TRAINING_DIR}/unicharset
    UNICHARSET_FILE="${TRAINING_DIR}/${LANG_CODE}.unicharset"
    check_file_readable ${outfile}
    mv ${outfile} ${UNICHARSET_FILE}

    XHEIGHTS_FILE="${TRAINING_DIR}/${LANG_CODE}.xheights"
    check_file_readable ${UNICHARSET_FILE}
    run_command ${SET_UNICHARSET_PROPERTIES_EXE} \
        -U ${UNICHARSET_FILE} -O ${UNICHARSET_FILE} -X ${XHEIGHTS_FILE} \
        --script_dir=${LANGDATA_ROOT}
    check_file_readable ${XHEIGHTS_FILE}
}

# Phase D : Generate (D)awg files from unicharset file and wordlist files
phase_D_generate_dawg() {
    tlog "\n=== Phase D: Generating Dawg files ==="

    # Skip if requested
    if [[ ${GENERATE_DAWGS} -eq 0 ]]; then
      tlog "Skipping ${phase_name}"
      return
    fi

    # Output files
    WORD_DAWG=${TRAINING_DIR}/${LANG_CODE}.word-dawg
    FREQ_DAWG=${TRAINING_DIR}/${LANG_CODE}.freq-dawg
    PUNC_DAWG=${TRAINING_DIR}/${LANG_CODE}.punc-dawg
    NUMBER_DAWG=${TRAINING_DIR}/${LANG_CODE}.number-dawg
    BIGRAM_DAWG=${TRAINING_DIR}/${LANG_CODE}.bigram-dawg

    # Word DAWG
    local freq_wordlist_file=${TRAINING_DIR}/${LANG_CODE}.wordlist.clean.freq
    if [[ -s ${WORDLIST_FILE} ]]; then
        tlog "Generating word Dawg"
        check_file_readable ${UNICHARSET_FILE}
        run_command ${WORDLIST2DAWG_EXE} -r 1 ${WORDLIST_FILE} ${WORD_DAWG} \
            ${UNICHARSET_FILE}
        check_file_readable ${WORD_DAWG}

        FREQ_DAWG_SIZE=100
        head -n ${FREQ_DAWG_SIZE} ${WORDLIST_FILE} > ${freq_wordlist_file}
    fi

    # Freq-word DAWG
    if [[ -s ${freq_wordlist_file} ]]; then
        check_file_readable ${UNICHARSET_FILE}
        tlog "Generating frequent-word Dawg"
        run_command ${WORDLIST2DAWG_EXE}  -r 1 ${freq_wordlist_file} \
            ${FREQ_DAWG} ${UNICHARSET_FILE}
        check_file_readable ${FREQ_DAWG}
    fi

    # Punctuation DAWG
    # -r arguments to WORDLIST2DAWG_EXE denote RTL reverse policy
    # (see Trie::RTLReversePolicy enum in third_party/tesseract/dict/trie.h).
    # We specify 0/RRP_DO_NO_REVERSE when generating number DAWG,
    # 1/RRP_REVERSE_IF_HAS_RTL for freq and word DAWGS,
    # 2/RRP_FORCE_REVERSE for the punctuation DAWG.
    local punc_reverse_policy=0;
    case ${LANG_CODE} in
      ara | div| fas | pus | snd | syr | uig | urd | heb | yid )
        punc_reverse_policy=2 ;;
      * ) ;;
    esac
    if [[ ! -s ${PUNC_FILE} ]]; then
        PUNC_FILE="${LANGDATA_ROOT}/common.punc"
    fi
    check_file_readable ${PUNC_FILE}
    run_command ${WORDLIST2DAWG_EXE} -r ${punc_reverse_policy} \
        ${PUNC_FILE} ${PUNC_DAWG} ${UNICHARSET_FILE}
    check_file_readable ${PUNC_DAWG}

    # Numbers DAWG
    if [[ -s ${NUMBERS_FILE} ]]; then
        run_command ${WORDLIST2DAWG_EXE} -r 0 \
            ${NUMBERS_FILE} ${NUMBER_DAWG} ${UNICHARSET_FILE}
        check_file_readable ${NUMBER_DAWG}
    fi

    # Bigram dawg
    if [[ -s ${WORD_BIGRAMS_FILE} ]]; then
        run_command ${WORDLIST2DAWG_EXE} -r 1 \
            ${WORD_BIGRAMS_FILE} ${BIGRAM_DAWG} ${UNICHARSET_FILE}
        check_file_readable ${BIGRAM_DAWG}
    fi
}

# Phase E : (E)xtract .tr feature files from .tif/.box files
phase_E_extract_features() {
    local box_config=$1
    local par_factor=$2
    if [[ -z ${par_factor} || ${par_factor} -le 0 ]]; then
        par_factor=1
    fi
    tlog "\n=== Phase E: Extracting features ==="
    TRAIN_EXPOSURES='0'

    local img_files=""
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
    local counter=0
    for img_file in ${img_files}; do
        run_command ${TESSERACT_EXE} ${img_file} ${img_file%.*} \
            ${box_config} ${config} &
      let counter=counter+1
      let rem=counter%par_factor
      if [[ "${rem}" -eq 0 ]]; then
        wait
      fi
    done
    wait
    export TESSDATA_PREFIX=${OLD_TESSDATA_PREFIX}
    # Check that all the output files were produced.
    for img_file in ${img_files}; do
        check_file_readable ${img_file%.*}.tr
    done
}

# Phase C : (C)luster feature prototypes in .tr into normproto file (cnTraining)
# phaseC_cluster_prototypes ${TRAINING_DIR}/${LANG_CODE}.normproto
phase_C_cluster_prototypes() {
    tlog "\n=== Phase C: Clustering feature prototypes (cnTraining) ==="
    local out_normproto=$1

    run_command ${CN_TRAINING_EXE} -D "${TRAINING_DIR}/" \
        $(ls ${TRAINING_DIR}/*.tr)

    check_file_readable ${TRAINING_DIR}/normproto
    mv ${TRAINING_DIR}/normproto ${out_normproto}
}

# Phase S : (S)hape clustering
phase_S_cluster_shapes() {
    if (( ! ${RUN_SHAPE_CLUSTERING} )); then
        tlog "\n=== Shape Clustering disabled ==="
        return
    fi
    check_file_readable ${LANGDATA_ROOT}/font_properties
    local font_props="-F ${LANGDATA_ROOT}/font_properties"
    if [[ -r ${TRAINING_DIR}/${LANG_CODE}.xheights ]] &&\
       [[ -s ${TRAINING_DIR}/${LANG_CODE}.xheights ]]; then
        font_props=${font_props}" -X ${TRAINING_DIR}/${LANG_CODE}.xheights"
    fi

    run_command ${SHAPE_TRAINING_EXE} \
        -D "${TRAINING_DIR}/" \
        -U ${TRAINING_DIR}/${LANG_CODE}.unicharset \
        -O ${TRAINING_DIR}/${LANG_CODE}.mfunicharset \
        ${font_props} \
        $(ls ${TRAINING_DIR}/*.tr)
    check_file_readable ${TRAINING_DIR}/shapetable \
        ${TRAINING_DIR}/${LANG_CODE}.mfunicharset
}

# Phase M : Clustering microfeatures (mfTraining)
phase_M_cluster_microfeatures() {
    tlog "\n=== Phase M : Clustering microfeatures (mfTraining) ==="

    check_file_readable ${LANGDATA_ROOT}/font_properties
    font_props="-F ${LANGDATA_ROOT}/font_properties"
    if [[ -r ${TRAINING_DIR}/${LANG_CODE}.xheights ]] && \
       [[ -s ${TRAINING_DIR}/${LANG_CODE}.xheights ]]; then
        font_props=${font_props}" -X ${TRAINING_DIR}/${LANG_CODE}.xheights"
    fi

    run_command ${MF_TRAINING_EXE} \
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

phase_B_generate_ambiguities() {
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


make__traineddata() {
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
  run_command ${COMBINE_TESSDATA_EXE} ${TRAINING_DIR}/${LANG_CODE}.

  # Copy it to the output dir, overwriting only if allowed by the cmdline flag.
  if [[ ! -d ${OUTPUT_DIR} ]]; then
      tlog "Creating new directory ${OUTPUT_DIR}"
      mkdir -p ${OUTPUT_DIR}
  fi
  local destfile=${OUTPUT_DIR}/${LANG_CODE}.traineddata;
  if [[ -f ${destfile} ]] && (( ! ${OVERWRITE} )); then
      err_exit "File ${destfile} exists and no --overwrite specified";
  fi
  tlog "Moving ${TRAINING_DIR}/${LANG_CODE}.traineddata to ${OUTPUT_DIR}"
  cp -f ${TRAINING_DIR}/${LANG_CODE}.traineddata ${destfile}
}

