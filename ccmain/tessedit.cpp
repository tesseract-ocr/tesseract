/**********************************************************************
 * File:        tessedit.cpp  (Formerly tessedit.c)
 * Description: Main program for merge of tess and editor.
 * Author:					Ray Smith
 * Created:					Tue Jan 07 15:21:46 GMT 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include "mfcpch.h"
//#include                                                      <osfcn.h>
//#include                                                      <signal.h>
//#include                                                      <time.h>
//#include                                                      <unistd.h>
#include          "tfacep.h"     //must be before main.h
//#include                                                      "fileerr.h"
#include          "stderr.h"
#include          "basedir.h"
#include          "tessvars.h"
//#include                                                      "debgwin.h"
//#include                                      "epapdest.h"
#include          "control.h"
#include          "imgs.h"
#include          "reject.h"
#include          "pageres.h"
//#include                                                      "gpapdest.h"
#include          "nwmain.h"
#include          "pgedit.h"
#include          "tprintf.h"
//#include                                      "ipeerr.h"
//#include                                                      "restart.h"
#include          "tessedit.h"
//#include                                                      "fontfind.h"
#include "permute.h"
#include "stopper.h"
#include "intmatcher.h"
#include "chop.h"
#include "efio.h"
#include "danerror.h"
#include "globals.h"
#include "tesseractclass.h"
#include "params.h"

#include          "notdll.h"     //phils nn stuff

#define VARDIR        "configs/" /*variables files */
                                 //config under api
#define API_CONFIG      "configs/api_config"

ETEXT_DESC *global_monitor = NULL;  // progress monitor

namespace tesseract {

// Read a "config" file containing a set of variable, value pairs.
// Searches the standard places: tessdata/configs, tessdata/tessconfigs
// and also accepts a relative or absolute path name.
void Tesseract::read_config_file(const char *filename, bool init_only) {
  STRING path = datadir;
  path += "configs/";
  path += filename;
  FILE* fp;
  if ((fp = fopen(path.string(), "rb")) != NULL) {
    fclose(fp);
  } else {
    path = datadir;
    path += "tessconfigs/";
    path += filename;
    if ((fp = fopen(path.string(), "rb")) != NULL) {
      fclose(fp);
    } else {
      path = filename;
    }
  }
  ParamUtils::ReadParamsFile(path.string(), init_only, this->params());
}

// Returns false if a unicharset file for the specified language was not found
// or was invalid.
// This function initializes TessdataManager. After TessdataManager is
// no longer needed, TessdataManager::End() should be called.
//
// This function sets tessedit_oem_mode to the given OcrEngineMode oem, unless
// it is OEM_DEFAULT, in which case the value of the variable will be obtained
// from the language-specific config file (stored in [lang].traineddata), from
// the config files specified on the command line or left as the default
// OEM_TESSERACT_ONLY if none of the configs specify this variable.
bool Tesseract::init_tesseract_lang_data(
    const char *arg0, const char *textbase, const char *language,
    OcrEngineMode oem, char **configs, int configs_size,
    const GenericVector<STRING> *vars_vec,
    const GenericVector<STRING> *vars_values,
    bool set_only_init_params) {
  // Set the basename, compute the data directory.
  main_setup(arg0, textbase);

  // Set the language data path prefix
  lang = language != NULL ? language : "eng";
  language_data_path_prefix = datadir;
  language_data_path_prefix += lang;
  language_data_path_prefix += ".";

  // Initialize TessdataManager.
  STRING tessdata_path = language_data_path_prefix + kTrainedDataSuffix;
  if (!tessdata_manager.Init(tessdata_path.string(),
                             tessdata_manager_debug_level)) {
    return false;
  }

  // If a language specific config file (lang.config) exists, load it in.
  if (tessdata_manager.SeekToStart(TESSDATA_LANG_CONFIG)) {
    ParamUtils::ReadParamsFromFp(
        tessdata_manager.GetDataFilePtr(),
        tessdata_manager.GetEndOffset(TESSDATA_LANG_CONFIG),
        false, this->params());
    if (tessdata_manager_debug_level) {
      tprintf("Loaded language config file\n");
    }
  }

  // Load tesseract variables from config files. This is done after loading
  // language-specific variables from [lang].traineddata file, so that custom
  // config files can override values in [lang].traineddata file.
  for (int i = 0; i < configs_size; ++i) {
    read_config_file(configs[i], set_only_init_params);
  }

  // Set params specified in vars_vec (done after setting params from config
  // files, so that params in vars_vec can override those from files).
  if (vars_vec != NULL && vars_values != NULL) {
    for (int i = 0; i < vars_vec->size(); ++i) {
      if (!ParamUtils::SetParam((*vars_vec)[i].string(),
                                (*vars_values)[i].string(),
                                set_only_init_params, this->params())) {
        tprintf("Error setting param %s\n", (*vars_vec)[i].string());
        exit(1);
      }
    }
  }

  if (((STRING &)tessedit_write_params_to_file).length() > 0) {
    FILE *params_file = fopen(tessedit_write_params_to_file.string(), "wb");
    if (params_file != NULL) {
      ParamUtils::PrintParams(params_file, this->params());
      fclose(params_file);
      if (tessdata_manager_debug_level > 0) {
        tprintf("Wrote parameters to %s\n",
                tessedit_write_params_to_file.string());
      }
    } else {
      tprintf("Failed to open %s for writing params.\n",
              tessedit_write_params_to_file.string());
    }
  }

  // Determine which ocr engine(s) should be loaded and used for recognition.
  if (oem != OEM_DEFAULT) tessedit_ocr_engine_mode.set_value(oem);
  if (tessdata_manager_debug_level) {
    tprintf("Loading Tesseract/Cube with tessedit_ocr_engine_mode %d\n",
            static_cast<int>(tessedit_ocr_engine_mode));
  }

  // Load the unicharset
  if (!tessdata_manager.SeekToStart(TESSDATA_UNICHARSET) ||
      !unicharset.load_from_file(tessdata_manager.GetDataFilePtr())) {
    return false;
  }
  if (unicharset.size() > MAX_NUM_CLASSES) {
    tprintf("Error: Size of unicharset is greater than MAX_NUM_CLASSES\n");
    return false;
  }
  right_to_left_ = unicharset.any_right_to_left();
  if (tessdata_manager_debug_level) tprintf("Loaded unicharset\n");

  if (!tessedit_ambigs_training &&
      tessdata_manager.SeekToStart(TESSDATA_AMBIGS)) {
    unichar_ambigs.LoadUnicharAmbigs(
        tessdata_manager.GetDataFilePtr(),
        tessdata_manager.GetEndOffset(TESSDATA_AMBIGS),
        ambigs_debug_level, use_ambigs_for_adaption, &unicharset);
    if (tessdata_manager_debug_level) tprintf("Loaded ambigs\n");
  }

  // Load Cube objects if necessary.
  if (tessedit_ocr_engine_mode == OEM_CUBE_ONLY) {
    ASSERT_HOST(init_cube_objects(false, &tessdata_manager));
    if (tessdata_manager_debug_level)
      tprintf("Loaded Cube w/out combiner\n");
  } else if (tessedit_ocr_engine_mode == OEM_TESSERACT_CUBE_COMBINED) {
    ASSERT_HOST(init_cube_objects(true, &tessdata_manager));
    if (tessdata_manager_debug_level)
      tprintf("Loaded Cube with combiner\n");
  }

  return true;
}

int Tesseract::init_tesseract(
    const char *arg0, const char *textbase, const char *language,
    OcrEngineMode oem, char **configs, int configs_size,
    const GenericVector<STRING> *vars_vec,
    const GenericVector<STRING> *vars_values,
    bool set_only_init_params) {
  if (!init_tesseract_lang_data(arg0, textbase, language, oem, configs,
                                configs_size, vars_vec, vars_values,
                                set_only_init_params)) {
    return -1;
  }
  // If only Cube will be used, skip loading Tesseract classifier's
  // pre-trained templates.
  bool init_tesseract_classifier =
    (tessedit_ocr_engine_mode == OEM_TESSERACT_ONLY ||
     tessedit_ocr_engine_mode == OEM_TESSERACT_CUBE_COMBINED);
  // If only Cube will be used and if it has its own Unicharset,
  // skip initializing permuter and loading Tesseract Dawgs.
  bool init_dict =
    !(tessedit_ocr_engine_mode == OEM_CUBE_ONLY &&
      tessdata_manager.SeekToStart(TESSDATA_CUBE_UNICHARSET));
  program_editup(textbase, init_tesseract_classifier, init_dict);
  tessdata_manager.End();
  return 0;                      //Normal exit
}

// init the LM component
int Tesseract::init_tesseract_lm(const char *arg0,
                   const char *textbase,
                   const char *language) {
  if (!init_tesseract_lang_data(arg0, textbase, language, OEM_TESSERACT_ONLY,
                                NULL, 0, NULL, NULL, false))
    return -1;
  getDict().Load();
  tessdata_manager.End();
  return 0;
}

void Tesseract::end_tesseract() {
  end_recog();
}

/* Define command type identifiers */

enum CMD_EVENTS
{
  ACTION_1_CMD_EVENT,
  RECOG_WERDS,
  RECOG_PSEUDO,
  ACTION_2_CMD_EVENT
};

}  // namespace tesseract
