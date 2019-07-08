# (C) Copyright 2014, Google Inc.
# (C) Copyright 2018, James R Barlow
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
# For a detailed description of the phases, see
# https://github.com/tesseract-ocr/tesseract/wiki/TrainingTesseract
#

import argparse
import atexit
import concurrent.futures
import logging
import os
import pathlib
import platform
import shutil
import subprocess
import sys
from datetime import date
from operator import itemgetter
from tempfile import TemporaryDirectory, mkdtemp

from tqdm import tqdm

from language_specific import VERTICAL_FONTS

log = logging.getLogger(__name__)


class TrainingArgs(argparse.Namespace):
    def __init__(self):
        super(TrainingArgs, self).__init__()
        self.uname = platform.uname().system.lower()
        self.lang_code = "eng"
        self.timestamp = str(date.today())

        self._font_config_cache = TemporaryDirectory(prefix="font_tmp")
        self.font_config_cache = self._font_config_cache.name
        self.fonts_dir = (
            "/Library/Fonts/" if "darwin" in self.uname else "/usr/share/fonts/"
        )

        self.max_pages = 0
        self.save_box_tiff = False
        self.overwrite = False
        self.linedata = False
        self.run_shape_clustering = False
        self.extract_font_properties = True
        self.distort_image = False

    def __eq__(self, other):
        return (argparse.Namespace.__eq__(self, other) and
        self.uname == other.uname and self.lang_code == other.lang_code and
        self.timestamp == other.timestamp and self.font_config_cache == other.font_config_cache and
        self.fonts_dir == other.fonts_dir and self.max_pages == other.max_pages and
        self.save_box_tiff == other.save_box_tiff and self.overwrite == other.overwrite and
        self.linedata == other.linedata and self.run_shape_clustering == other.run_shape_clustering and
        self.extract_font_properties == other.extract_font_properties and
        self.distort_image == other.distort_image)


def err_exit(msg):
    log.critical(msg)
    sys.exit(1)


# Helper function to run a command and append its output to a log. Aborts early
# if the program file is not found.
# Usage: run_command CMD ARG1 ARG2...
def run_command(cmd, *args, env=None):
    for d in ("", "api/", "training/"):
        testcmd = shutil.which(f"{d}{cmd}")
        if shutil.which(testcmd):
            cmd = testcmd
            break
    if not shutil.which(cmd):
        err_exit(f"{cmd} not found")

    log.debug(f"Running {cmd}")
    args = list(args)
    for idx, arg in enumerate(args):
        log.debug(arg)
        # Workaround for https://bugs.python.org/issue33617
        # TypeError: argument of type 'WindowsPath' is not iterable
        if isinstance(arg, pathlib.WindowsPath):
            args[idx] = str(arg)

    proc = subprocess.run(
        [cmd, *args], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env
    )
    proclog = logging.getLogger(cmd)
    if proc.returncode == 0:
        proclog.debug(proc.stdout.decode("utf-8", errors="replace"))
    else:
        try:
            proclog.error(proc.stdout.decode("utf-8", errors="replace"))
        except Exception as e:
            proclog.error(e)
        err_exit(f"Program {cmd} failed with return code {proc.returncode}. Abort.")


# Check if all the given files exist, or exit otherwise.
# Used to check required input files and produced output files in each phase.
# Usage: check_file_readable FILE1 FILE2...
def check_file_readable(*filenames):
    if isinstance(filenames, (str, pathlib.Path)):
        filenames = [filenames]
    for filename in filenames:
        try:
            with pathlib.Path(filename).open():
                pass
        except FileNotFoundError:
            err_exit(f"Required/expected file '{filename}' does not exist")
        except PermissionError:
            err_exit(f"{filename} is not readable")
        except IOError as e:
            err_exit(f"{filename} IO Error: {str(e)}")
    return True


parser = argparse.ArgumentParser(
    epilog="""
    The font names specified in --fontlist need to be recognizable by Pango using
    fontconfig. An easy way to list the canonical names of all fonts available on
    your system is to run text2image with --list_available_fonts and the
    appropriate --fonts_dir path.
    """
)
parser.add_argument(
    "--fontlist",
    dest="fonts",
    nargs="+",
    type=str,
    help="A list of fontnames to train on.",
)
parser.add_argument("--fonts_dir", help="Path to font files.")
parser.add_argument("--tmp_dir", help="Path to temporary training directory.")
parser.add_argument(
    "--lang", metavar="LANG_CODE", dest="lang_code", help="ISO 639 code."
)
parser.add_argument(
    "--langdata_dir",
    metavar="DATADIR",
    help="Path to tesseract/training/langdata directory.",
)
parser.add_argument("--maxpages", type=int, dest="max_pages")
parser.add_argument(
    "--output_dir", metavar="OUTPUTDIR", help="Location of output traineddata file."
)
parser.add_argument(
    "--overwrite", action="store_true", help="Safe to overwrite files in output_dir."
)
parser.add_argument(
    "--save_box_tiff",
    action="store_true",
    help="Save box/tiff pairs along with lstmf files.",
)
parser.add_argument(
    "--linedata_only",
    dest="linedata",
    action="store_true",
    help="Only generate training data for lstmtraining.",
)

inputdata_group = parser.add_argument_group(
    "inputdata",
    "OPTIONAL flags for input data. If unspecified we will look for them in the langdata_dir directory.",
)
inputdata_group.add_argument(
    "--training_text", metavar="TEXTFILE", help="Text to render and use for training."
)
inputdata_group.add_argument(
    "--wordlist",
    dest="wordlist_file",
    metavar="WORDFILE",
    help="Word list for the language ordered by decreasing frequency.",
)

parser.add_argument("--extract_font_properties", action="store_true")
parser.add_argument(
    "--noextract_font_properties", dest="extract_font_properties", action="store_false"
)

parser.add_argument(
    "--distort_image", dest="distort_image", action="store_true"
)

tessdata_group = parser.add_argument_group(
    "tessdata",
    "OPTIONAL flag to specify location of existing traineddata files, required during feature extraction. If unspecified will use TESSDATA_PREFIX defined in the current environment.",
)
tessdata_group.add_argument(
    "--tessdata_dir",
    metavar="TESSDATADIR",
    help="Path to tesseract/tessdata directory.",
)

parser.add_argument(
    "--exposures",
    metavar="EXPOSURES",
    action="append",
    nargs="+",
    help="A list of exposure levels to use (e.g. -1,0,1).",
)


# Does simple command-line parsing and initialization.
def parse_flags(argv=None):
    ctx = TrainingArgs()
    log.debug(ctx)
    parser.parse_args(args=argv, namespace=ctx)
    log.debug(ctx)

    if not ctx.lang_code:
        err_exit("Need to specify a language --lang")
    if not ctx.langdata_dir:
        err_exit("Need to specify path to language files --langdata_dir")
    if not ctx.tessdata_dir:
        tessdata_prefix = os.environ.get("TESSDATA_PREFIX", "")
        if not tessdata_prefix:
            err_exit(
                "Need to specify a --tessdata_dir or have a "
                "TESSDATA_PREFIX variable defined in your environment"
            )
        else:
            ctx.tessdata_dir = tessdata_prefix
    if not ctx.output_dir:
        ctx.output_dir = mkdtemp(prefix=f"trained-{ctx.lang_code}-{ctx.timestamp}")
        log.info(f"Output directory set to: {ctx.output_dir}")

    # Location where intermediate files will be created.
    if not ctx.tmp_dir:
        ctx.training_dir = mkdtemp(prefix=f"{ctx.lang_code}-{ctx.timestamp}")
    else:
        ctx.training_dir = mkdtemp(prefix=f"{ctx.lang_code}-{ctx.timestamp}", dir=ctx.tmp_dir)
    # Location of log file for the whole run.
    ctx.log_file = pathlib.Path(ctx.training_dir) / "tesstrain.log"
    log.info(f"Log file location: {ctx.log_file}")

    def show_tmpdir_location(training_dir):
        # On successful exit we will delete this first; on failure we want to let the user
        # know where the log is
        if pathlib.Path(training_dir).exists():
            print(f"Temporary files retained at: {training_dir}")

    atexit.register(show_tmpdir_location, ctx.training_dir)

    # Take training text and wordlist from the langdata directory if not
    # specified in the command-line.
    if not ctx.training_text:
        ctx.training_text = (
                pathlib.Path(ctx.langdata_dir) / ctx.lang_code / f"{ctx.lang_code}.training_text"
        )
    if not ctx.wordlist_file:
        ctx.wordlist_file = (
                pathlib.Path(ctx.langdata_dir) / ctx.lang_code / f"{ctx.lang_code}.wordlist"
        )

    ctx.word_bigrams_file = (
            pathlib.Path(ctx.langdata_dir) / ctx.lang_code / f"{ctx.lang_code}.word.bigrams"
    )
    ctx.numbers_file = (
            pathlib.Path(ctx.langdata_dir) / ctx.lang_code / f"{ctx.lang_code}.numbers"
    )
    ctx.punc_file = pathlib.Path(ctx.langdata_dir) / ctx.lang_code / f"{ctx.lang_code}.punc"
    ctx.bigram_freqs_file = pathlib.Path(ctx.training_text).with_suffix(
        ".training_text.bigram_freqs"
    )
    ctx.unigram_freqs_file = pathlib.Path(ctx.training_text).with_suffix(
        ".training_text.unigram_freqs"
    )
    ctx.train_ngrams_file = pathlib.Path(ctx.training_text).with_suffix(
        ".training_text.train_ngrams"
    )
    ctx.generate_dawgs = 1
    log.debug(ctx)
    return ctx


def cleanup(ctx):
    shutil.copy(ctx.log_file, ctx.output_dir)
    shutil.rmtree(ctx.training_dir)
    return


# Function initializes font config with a unique font cache dir.
def initialize_fontconfig(ctx):
    sample_path = pathlib.Path(ctx.font_config_cache) / "sample_text.txt"
    pathlib.Path(sample_path).write_text("Text\n")
    log.info(f"Testing font: {ctx.fonts[0]}")
    run_command(
        "text2image",
        f"--fonts_dir={ctx.fonts_dir}",
        f"--font={ctx.fonts[0]}",
        f"--outputbase={sample_path}",
        f"--text={sample_path}",
        f"--fontconfig_tmpdir={ctx.font_config_cache}",
    )


def make_fontname(font):
    return font.replace(" ", "_").replace(",", "")


def make_outbase(ctx, fontname, exposure):
    return pathlib.Path(ctx.training_dir) / f"{ctx.lang_code}.{fontname}.exp{exposure}"


# Helper function for phaseI_generate_image. Generates the image for a single
# language/font combination in a way that can be run in parallel.
def generate_font_image(ctx, font, exposure, char_spacing):
    log.info(f"Rendering using {font}")
    fontname = make_fontname(font)
    outbase = make_outbase(ctx, fontname, exposure)

    common_args = [
        f"--fontconfig_tmpdir={ctx.font_config_cache}",
        f"--fonts_dir={ctx.fonts_dir}",
        f"--strip_unrenderable_words",
        f"--leading={ctx.leading}",
        f"--char_spacing={char_spacing}",
        f"--exposure={exposure}",
        f"--outputbase={outbase}",
        f"--max_pages={ctx.max_pages}",
    ]

    if ctx.distort_image:
        common_args.append("--distort_image")

    # add --writing_mode=vertical-upright to common_args if the font is
    # specified to be rendered vertically.
    if font in VERTICAL_FONTS:
        common_args.append("--writing_mode=vertical-upright")

    run_command(
        "text2image",
        *common_args,
        f"--font={font}",
        f"--text={ctx.training_text}",
        *ctx.text2image_extra_args,
    )

    check_file_readable(str(outbase) + ".box", str(outbase) + ".tif")

    if ctx.extract_font_properties and pathlib.Path(ctx.train_ngrams_file).exists():
        log.info(f"Extracting font properties of {font}")
        run_command(
            "text2image",
            *common_args,
            f"--font={font}",
            f"--ligatures=false",
            f"--text={ctx.train_ngrams_file}",
            f"--only_extract_font_properties",
            f"--ptsize=32",
        )
        check_file_readable(str(outbase) + ".fontinfo")
    return f"{font}-{exposure}"


# Phase I : Generate (I)mages from training text for each font.
def phase_I_generate_image(ctx, par_factor=None):
    if not par_factor or par_factor <= 0:
        par_factor = 1

    log.info("=== Phase I: Generating training images ===")
    check_file_readable(ctx.training_text)
    char_spacing = 0.0

    for exposure in ctx.exposures:
        if ctx.extract_font_properties and pathlib.Path(ctx.bigram_freqs_file).exists():
            # Parse .bigram_freqs file and compose a .train_ngrams file with text
            # for tesseract to recognize during training. Take only the ngrams whose
            # combined weight accounts for 95% of all the bigrams in the language.
            lines = pathlib.Path(ctx.bigram_freqs_file).read_text(encoding="utf-8").split("\n")
            records = (line.split() for line in lines)
            p = 0.99
            ngram_frac = p * sum(int(rec[1]) for rec in records if len(rec) >= 2)

            with pathlib.Path(ctx.train_ngrams_file).open("w", encoding="utf-8") as f:
                cumsum = 0
                for bigram, count in sorted(records, key=itemgetter(1), reverse=True):
                    if cumsum > ngram_frac:
                        break
                    f.write(bigram + " ")
                    cumsum += count

            check_file_readable(ctx.train_ngrams_file)

        with tqdm(
                total=len(ctx.fonts)
        ) as pbar, concurrent.futures.ThreadPoolExecutor(max_workers=par_factor) as executor:
            futures = [
                executor.submit(generate_font_image, ctx, font, exposure, char_spacing)
                for font in ctx.fonts
            ]
            for future in concurrent.futures.as_completed(futures):
                try:
                    future.result()
                except Exception as exc:
                    err_exit("Failed while generating images " + str(exc))
                else:
                    pbar.update(1)

        # Check that each process was successful.
        for font in ctx.fonts:
            fontname = make_fontname(font)
            outbase = make_outbase(ctx, fontname, exposure)
            check_file_readable(str(outbase) + ".box", str(outbase) + ".tif")
    return


# Phase UP : Generate (U)nicharset and (P)roperties file.
def phase_UP_generate_unicharset(ctx):
    log.info("=== Phase UP: Generating unicharset and unichar properties files ===")

    box_files = pathlib.Path(ctx.training_dir).glob("*.box")

    ctx.unicharset_file = pathlib.Path(ctx.training_dir) / f"{ctx.lang_code}.unicharset"

    run_command(
        "unicharset_extractor",
        "--output_unicharset",
        f"{ctx.unicharset_file}",
        "--norm_mode",
        f"{ctx.norm_mode}",
        *box_files,
    )
    check_file_readable(ctx.unicharset_file)

    ctx.xheights_file = pathlib.Path(ctx.training_dir) / f"{ctx.lang_code}.xheights"
    run_command(
        "set_unicharset_properties",
        "-U",
        f"{ctx.unicharset_file}",
        "-O",
        f"{ctx.unicharset_file}",
        "-X",
        f"{ctx.xheights_file}",
        f"--script_dir={ctx.langdata_dir}",
    )
    check_file_readable(ctx.xheights_file)


# # Phase D : Generate (D)awg files from unicharset file and wordlist files
# phase_D_generate_dawg() {
#     tlog "\n=== Phase D: Generating Dawg files ==="

#     # Skip if requested
#     if [[ ${GENERATE_DAWGS} -eq 0 ]]; then
#       tlog "Skipping ${phase_name}"
#       return
#     fi

#     # Output files
#     WORD_DAWG=${TRAINING_DIR}/${LANG_CODE}.word-dawg
#     FREQ_DAWG=${TRAINING_DIR}/${LANG_CODE}.freq-dawg
#     PUNC_DAWG=${TRAINING_DIR}/${LANG_CODE}.punc-dawg
#     NUMBER_DAWG=${TRAINING_DIR}/${LANG_CODE}.number-dawg
#     BIGRAM_DAWG=${TRAINING_DIR}/${LANG_CODE}.bigram-dawg

#     # Word DAWG
#     local freq_wordlist_file=${TRAINING_DIR}/${LANG_CODE}.wordlist.clean.freq
#     if [[ -s ${WORDLIST_FILE} ]]; then
#         tlog "Generating word Dawg"
#         check_file_readable ${unicharset_file}
#         run_command wordlist2dawg -r 1 ${WORDLIST_FILE} ${WORD_DAWG} \
#             ${UNICHARSET_FILE}
#         check_file_readable ${WORD_DAWG}

#         FREQ_DAWG_SIZE=100
#         head -n ${FREQ_DAWG_SIZE} ${WORDLIST_FILE} > ${freq_wordlist_file}
#     fi

#     # Freq-word DAWG
#     if [[ -s ${freq_wordlist_file} ]]; then
#         check_file_readable ${UNICHARSET_FILE}
#         tlog "Generating frequent-word Dawg"
#         run_command wordlist2dawg  -r 1 ${freq_wordlist_file} \
#             ${FREQ_DAWG} ${UNICHARSET_FILE}
#         check_file_readable ${FREQ_DAWG}
#     fi

#     # Punctuation DAWG
#     # -r arguments to wordlist2dawg denote RTL reverse policy
#     # (see Trie::RTLReversePolicy enum in tesseract/src/dict/trie.h).
#     # We specify 0/RRP_DO_NO_REVERSE when generating number DAWG,
#     # 1/RRP_REVERSE_IF_HAS_RTL for freq and word DAWGS,
#     # 2/RRP_FORCE_REVERSE for the punctuation DAWG.
#     local punc_reverse_policy=0;
#     if [[ "${LANG_IS_RTL}" == "1" ]]; then
#       punc_reverse_policy=2
#     fi
#     if [[ ! -s ${PUNC_FILE} ]]; then
#         PUNC_FILE="{ctx.langdata_dir}/common.punc"
#     fi
#     check_file_readable ${PUNC_FILE}
#     run_command wordlist2dawg -r ${punc_reverse_policy} \
#         ${PUNC_FILE} ${PUNC_DAWG} ${UNICHARSET_FILE}
#     check_file_readable ${PUNC_DAWG}

#     # Numbers DAWG
#     if [[ -s ${NUMBERS_FILE} ]]; then
#         run_command wordlist2dawg -r 0 \
#             ${NUMBERS_FILE} ${NUMBER_DAWG} ${UNICHARSET_FILE}
#         check_file_readable ${NUMBER_DAWG}
#     fi

#     # Bigram dawg
#     if [[ -s ${WORD_BIGRAMS_FILE} ]]; then
#         run_command wordlist2dawg -r 1 \
#             ${WORD_BIGRAMS_FILE} ${BIGRAM_DAWG} ${UNICHARSET_FILE}
#         check_file_readable ${BIGRAM_DAWG}
#     fi
# }

# Phase E : (E)xtract .tr feature files from .tif/.box files
def phase_E_extract_features(ctx, box_config, ext):
    log.info(f"=== Phase E: Generating {ext} files ===")

    img_files = list(pathlib.Path(ctx.training_dir).glob("*.exp*.tif"))
    log.debug(img_files)

    # Use any available language-specific configs.
    config = ""
    testconfig = pathlib.Path(ctx.langdata_dir) / ctx.lang_code / f"{ctx.lang_code}.config"
    if testconfig.exists():
        config = testconfig
        log.info(f"Using {ctx.lang_code}.config")

    tessdata_environ = os.environ.copy()
    tessdata_environ["TESSDATA_PREFIX"] = str(ctx.tessdata_dir)

    log.info(f"Using TESSDATA_PREFIX={tessdata_environ['TESSDATA_PREFIX']}")

    with tqdm(total=len(img_files)) as pbar, concurrent.futures.ThreadPoolExecutor(
            max_workers=2
    ) as executor:
        futures = []
        for img_file in img_files:
            future = executor.submit(
                run_command,
                "tesseract",
                img_file,
                pathlib.Path(img_file).with_suffix(""),
                *box_config,
                config,
                env=tessdata_environ,
            )
            futures.append(future)

        for future in concurrent.futures.as_completed(futures):
            try:
                future.result()
            except Exception as exc:
                err_exit("Failed while extracting features: " + str(exc))
            else:
                pbar.update(1)
    # Check that all the output files were produced.
    for img_file in img_files:
        check_file_readable(pathlib.Path(img_file.with_suffix("." + ext)))

    return


# # Phase C : (C)luster feature prototypes in .tr into normproto file (cnTraining)
# # phaseC_cluster_prototypes ${TRAINING_DIR}/${LANG_CODE}.normproto
# phase_C_cluster_prototypes() {
#     tlog "\n=== Phase C: Clustering feature prototypes (cnTraining) ==="
#     local out_normproto=$1

#     run_command cntraining -D "${TRAINING_DIR}/" \
#         $(ls ${TRAINING_DIR}/*.tr)

#     check_file_readable ${TRAINING_DIR}/normproto
#     mv ${TRAINING_DIR}/normproto ${out_normproto}
# }

# # Phase S : (S)hape clustering
# phase_S_cluster_shapes() {
#     if ((! RUN_SHAPE_CLUSTERING)); then
#         tlog "\n=== Shape Clustering disabled ==="
#         return
#     fi
#     check_file_readable {ctx.langdata_dir}/font_properties
#     local font_props="-F {ctx.langdata_dir}/font_properties"
#     if [[ -r ${TRAINING_DIR}/${LANG_CODE}.xheights ]] &&\
#        [[ -s ${TRAINING_DIR}/${LANG_CODE}.xheights ]]; then
#         font_props=${font_props}" -X ${TRAINING_DIR}/${LANG_CODE}.xheights"
#     fi

#     run_command shapeclustering \
#         -D "${TRAINING_DIR}/" \
#         -U ${TRAINING_DIR}/${LANG_CODE}.unicharset \
#         -O ${TRAINING_DIR}/${LANG_CODE}.mfunicharset \
#         ${font_props} \
#         $(ls ${TRAINING_DIR}/*.tr)
#     check_file_readable ${TRAINING_DIR}/shapetable \
#         ${TRAINING_DIR}/${LANG_CODE}.mfunicharset
# }

# # Phase M : Clustering microfeatures (mfTraining)
# phase_M_cluster_microfeatures() {
#     tlog "\n=== Phase M : Clustering microfeatures (mfTraining) ==="

#     check_file_readable {ctx.langdata_dir}/font_properties
#     font_props="-F {ctx.langdata_dir}/font_properties"
#     if [[ -r ${TRAINING_DIR}/${LANG_CODE}.xheights ]] && \
#        [[ -s ${TRAINING_DIR}/${LANG_CODE}.xheights ]]; then
#         font_props=${font_props}" -X ${TRAINING_DIR}/${LANG_CODE}.xheights"
#     fi

#     run_command mftraining \
#         -D "${TRAINING_DIR}/" \
#         -U ${TRAINING_DIR}/${LANG_CODE}.unicharset \
#         -O ${TRAINING_DIR}/${LANG_CODE}.mfunicharset \
#         ${font_props} \
#         $(ls ${TRAINING_DIR}/*.tr)
#     check_file_readable ${TRAINING_DIR}/inttemp ${TRAINING_DIR}/shapetable \
#         ${TRAINING_DIR}/pffmtable ${TRAINING_DIR}/${LANG_CODE}.mfunicharset
#     mv ${TRAINING_DIR}/inttemp ${TRAINING_DIR}/${LANG_CODE}.inttemp
#     mv ${TRAINING_DIR}/shapetable ${TRAINING_DIR}/${LANG_CODE}.shapetable
#     mv ${TRAINING_DIR}/pffmtable ${TRAINING_DIR}/${LANG_CODE}.pffmtable
#     mv ${TRAINING_DIR}/${LANG_CODE}.mfunicharset ${TRAINING_DIR}/${LANG_CODE}.unicharset
# }

# phase_B_generate_ambiguities() {
#   tlog "\n=== Phase B : ambiguities training ==="

#   # Check for manually created ambiguities data.
#   if [[ -r {ctx.langdata_dir}/${LANG_CODE}/${LANG_CODE}.unicharambigs ]]; then
#       tlog "Found file {ctx.langdata_dir}/${LANG_CODE}/${LANG_CODE}.unicharambigs"
#       cp {ctx.langdata_dir}/${LANG_CODE}/${LANG_CODE}.unicharambigs \
#           ${TRAINING_DIR}/${LANG_CODE}.unicharambigs
#       # Make it writable, as it may be read-only in the client.
#       chmod u+w ${TRAINING_DIR}/${LANG_CODE}.unicharambigs
#       return
#   else
#       tlog "No unicharambigs file found!"
#   fi

#   # TODO: Add support for generating ambiguities automatically.
# }


def make_lstmdata(ctx):
    log.info("=== Constructing LSTM training data ===")
    lang_prefix = f"{ctx.langdata_dir}/{ctx.lang_code}/{ctx.lang_code}"
    path_output = pathlib.Path(ctx.output_dir)
    if not path_output.is_dir():
        log.info(f"Creating new directory {ctx.output_dir}")
        path_output.mkdir(exist_ok=True, parents=True)

    args = []
    if ctx.lang_is_rtl:
        args.append("--lang_is_rtl")
    if ctx.norm_mode >= 2:
        args.append("--pass_through_recoder")

    # Build the starter traineddata from the inputs.
    run_command(
        "combine_lang_model",
        "--input_unicharset",
        f"{ctx.training_dir}/{ctx.lang_code}.unicharset",
        "--script_dir",
        f"{ctx.langdata_dir}",
        "--words",
        f"{lang_prefix}.wordlist",
        "--numbers",
        f"{lang_prefix}.numbers",
        "--puncs",
        f"{lang_prefix}.punc",
        "--output_dir",
        f"{ctx.output_dir}",
        "--lang",
        f"{ctx.lang_code}",
        *args,
    )

    def get_file_list():
        training_path = pathlib.Path(ctx.training_dir)
        if ctx.save_box_tiff:
            log.info("=== Saving box/tiff pairs for training data ===")
            yield from training_path.glob(f"{ctx.lang_code}*.box")
            yield from training_path.glob(f"{ctx.lang_code}*.tif")
        log.info("=== Moving lstmf files for training data ===")
        yield from training_path.glob(f"{ctx.lang_code}.*.lstmf")

    for f in get_file_list():
        log.debug(f"Moving {f} to {path_output / f.name}")
        shutil.move(str(f), path_output / f.name)

    lstm_list = f"{ctx.output_dir}/{ctx.lang_code}.training_files.txt"
    dir_listing = (str(p) for p in path_output.glob(f"{ctx.lang_code}.*.lstmf"))
    pathlib.Path(lstm_list).write_text("\n".join(dir_listing))

# make__traineddata() {
#   tlog "\n=== Making final traineddata file ==="
#   local lang_prefix={ctx.langdata_dir}/${LANG_CODE}/${LANG_CODE}

#   # Combine available files for this language from the langdata dir.
#   if [[ -r ${lang_prefix}.config ]]; then
#     tlog "Copying ${lang_prefix}.config to ${TRAINING_DIR}"
#     cp ${lang_prefix}.config ${TRAINING_DIR}
#     chmod u+w ${TRAINING_DIR}/${LANG_CODE}.config
#   fi
#   if [[ -r ${lang_prefix}.params-model ]]; then
#     tlog "Copying ${lang_prefix}.params-model to ${TRAINING_DIR}"
#     cp ${lang_prefix}.params-model ${TRAINING_DIR}
#     chmod u+w ${TRAINING_DIR}/${LANG_CODE}.params-model
#   fi

#   # Compose the traineddata file.
#   run_command combine_tessdata ${TRAINING_DIR}/${LANG_CODE}.

#   # Copy it to the output dir, overwriting only if allowed by the cmdline flag.
#   if [[ ! -d ${OUTPUT_DIR} ]]; then
#       tlog "Creating new directory ${OUTPUT_DIR}"
#       mkdir -p ${OUTPUT_DIR}
#   fi
#   local destfile=${OUTPUT_DIR}/${LANG_CODE}.traineddata;
#   if [[ -f ${destfile} ]] && ((! OVERWRITE)); then
#       err_exit "File ${destfile} exists and no --overwrite specified";
#   fi
#   tlog "Moving ${TRAINING_DIR}/${LANG_CODE}.traineddata to ${OUTPUT_DIR}"
#   cp -f ${TRAINING_DIR}/${LANG_CODE}.traineddata ${destfile}
# }
