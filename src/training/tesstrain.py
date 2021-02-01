#!/usr/bin/env python3

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
# This script provides an easy way to execute various phases of training
# Tesseract.  For a detailed description of the phases, see
# https://tesseract-ocr.github.io/tessdoc/Training-Tesseract.html.

import logging
import os
import sys

if (sys.version_info.major < 3) or (sys.version_info.major == 3 and sys.version_info.minor < 6):
    raise Exception("Must be using Python minimum version 3.6!")

sys.path.insert(0, os.path.dirname(__file__))
from tesstrain_utils import (
    parse_flags,
    initialize_fontconfig,
    phase_I_generate_image,
    phase_UP_generate_unicharset,
    phase_E_extract_features,
    make_lstmdata,
    cleanup,
)
import language_specific

log = logging.getLogger()


def setup_logging_console():
    log.setLevel(logging.DEBUG)
    console = logging.StreamHandler()
    console.setLevel(logging.INFO)
    console_formatter = logging.Formatter(
        "[%(asctime)s] %(levelname)s - %(message)s", datefmt="%H:%M:%S"
    )
    console.setFormatter(console_formatter)
    log.addHandler(console)


def setup_logging_logfile(logfile):
    logfile = logging.FileHandler(logfile, encoding='utf-8')
    logfile.setLevel(logging.DEBUG)
    logfile_formatter = logging.Formatter(
        "[%(asctime)s] - %(levelname)s - %(name)s - %(message)s"
    )
    logfile.setFormatter(logfile_formatter)
    log.addHandler(logfile)
    return logfile


def main():
    setup_logging_console()
    ctx = parse_flags()
    logfile = setup_logging_logfile(ctx.log_file)
    if not ctx.linedata:
        log.error("--linedata_only is required since only LSTM is supported")
        sys.exit(1)

    log.info(f"=== Starting training for language {ctx.lang_code}")
    ctx = language_specific.set_lang_specific_parameters(ctx, ctx.lang_code)

    initialize_fontconfig(ctx)
    phase_I_generate_image(ctx, par_factor=8)
    phase_UP_generate_unicharset(ctx)

    if ctx.linedata:
        phase_E_extract_features(ctx, ["lstm.train"], "lstmf")
        make_lstmdata(ctx)

    log.removeHandler(logfile)
    logfile.close()
    cleanup(ctx)
    log.info("All done!")
    return 0


if __name__ == "__main__":
    main()

# _rc0 = subprocess.call(["tlog","\n=== Starting training for language '"+str(LANG_CODE.val)+"'"],shell=True)
# _rc0 = subprocess.call(["source",os.popen("dirname "+__file__).read().rstrip("\n")+"/language-specific.sh"],shell=True)
# _rc0 = subprocess.call(["set_lang_specific_parameters",str(LANG_CODE.val)],shell=True)
# _rc0 = subprocess.call(["initialize_fontconfig"],shell=True)
# _rc0 = subprocess.call(["phase_I_generate_image","8"],shell=True)
# _rc0 = subprocess.call(["phase_UP_generate_unicharset"],shell=True)
# if (LINEDATA ):
# subprocess.call(["phase_E_extract_features"," --psm 6  lstm.train ","8","lstmf"],shell=True)
#     subprocess.call(["make__lstmdata"],shell=True)
#     subprocess.call(["tlog","\nCreated starter traineddata for language '"+str(LANG_CODE.val)+"'\n"],shell=True)
#     subprocess.call(["tlog","\nRun lstmtraining to do the LSTM training for language '"+str(LANG_CODE.val)+"'\n"],shell=True)
# else:
#     subprocess.call(["phase_D_generate_dawg"],shell=True)
#     subprocess.call(["phase_E_extract_features","box.train","8","tr"],shell=True)
#     subprocess.call(["phase_C_cluster_prototypes",str(TRAINING_DIR.val)+"/"+str(LANG_CODE.val)+".normproto"],shell=True)
#     if (str(ENABLE_SHAPE_CLUSTERING.val) == "y" ):
#         subprocess.call(["phase_S_cluster_shapes"],shell=True)
#     subprocess.call(["phase_M_cluster_microfeatures"],shell=True)
#     subprocess.call(["phase_B_generate_ambiguities"],shell=True)
#     subprocess.call(["make__traineddata"],shell=True)
#     subprocess.call(["tlog","\nCompleted training for language '"+str(LANG_CODE.val)+"'\n"],shell=True)
