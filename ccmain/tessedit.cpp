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

#include          "stderr.h"
#include          "basedir.h"
#include          "tessvars.h"
#include          "control.h"
#include          "reject.h"
#include          "pageres.h"
#include          "nwmain.h"
#include          "pgedit.h"
#include          "tprintf.h"
#include          "tessedit.h"
#include "stopper.h"
#include "intmatcher.h"
#include "chop.h"
#include "efio.h"
#include "danerror.h"
#include "globals.h"
#include "tesseractclass.h"
#include "params.h"

#define VARDIR        "configs/" /*variables files */
                                 //config under api
#define API_CONFIG      "configs/api_config"

ETEXT_DESC *global_monitor = NULL;  // progress monitor

namespace tesseract {

// Read a "config" file containing a set of variable, value pairs.
// Searches the standard places: tessdata/configs, tessdata/tessconfigs
// and also accepts a relative or absolute path name.
void Tesseract::read_config_file(const char *filename,
                                 SetParamConstraint constraint) {
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
  ParamUtils::ReadParamsFile(path.string(), constraint, this->params());
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
    bool set_only_non_debug_params) {
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
        SET_PARAM_CONSTRAINT_NONE, this->params());
    if (tessdata_manager_debug_level) {
      tprintf("Loaded language config file\n");
    }
  }

  SetParamConstraint set_params_constraint = set_only_non_debug_params ?
      SET_PARAM_CONSTRAINT_NON_DEBUG_ONLY : SET_PARAM_CONSTRAINT_NONE;
  // Load tesseract variables from config files. This is done after loading
  // language-specific variables from [lang].traineddata file, so that custom
  // config files can override values in [lang].traineddata file.
  for (int i = 0; i < configs_size; ++i) {
    read_config_file(configs[i], set_params_constraint);
  }

  // Set params specified in vars_vec (done after setting params from config
  // files, so that params in vars_vec can override those from files).
  if (vars_vec != NULL && vars_values != NULL) {
    for (int i = 0; i < vars_vec->size(); ++i) {
      if (!ParamUtils::SetParam((*vars_vec)[i].string(),
                                (*vars_values)[i].string(),
                                set_params_constraint, this->params())) {
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

  // If we are only loading the config file (and so not planning on doing any
  // recognition) then there's nothing else do here.
  if (tessedit_init_config_only) {
    if (tessdata_manager_debug_level) {
      tprintf("Returning after loading config file\n");
    }
    return true;
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
  if (tessdata_manager_debug_level) tprintf("Loaded unicharset\n");
  right_to_left_ = unicharset.major_right_to_left();

  // Setup initial unichar ambigs table and read universal ambigs.
  UNICHARSET encoder_unicharset;
  encoder_unicharset.CopyFrom(unicharset);
  unichar_ambigs.InitUnicharAmbigs(unicharset, use_ambigs_for_adaption);
  unichar_ambigs.LoadUniversal(encoder_unicharset, &unicharset);

  if (!tessedit_ambigs_training &&
      tessdata_manager.SeekToStart(TESSDATA_AMBIGS)) {
    TFile ambigs_file;
    ambigs_file.Open(tessdata_manager.GetDataFilePtr(),
                     tessdata_manager.GetEndOffset(TESSDATA_AMBIGS) + 1);
    unichar_ambigs.LoadUnicharAmbigs(
        encoder_unicharset,
        &ambigs_file,
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

  // Init ParamsModel.
  // Load pass1 and pass2 weights (for now these two sets are the same, but in
  // the future separate sets of weights can be generated).
  for (int p = ParamsModel::PTRAIN_PASS1;
      p < ParamsModel::PTRAIN_NUM_PASSES; ++p) {
    language_model_->getParamsModel().SetPass(
        static_cast<ParamsModel::PassEnum>(p));
    if (tessdata_manager.SeekToStart(TESSDATA_PARAMS_MODEL)) {
      if (!language_model_->getParamsModel().LoadFromFp(
          lang.string(), tessdata_manager.GetDataFilePtr(),
          tessdata_manager.GetEndOffset(TESSDATA_PARAMS_MODEL))) {
        return false;
      }
    }
  }
  if (tessdata_manager_debug_level) language_model_->getParamsModel().Print();

  return true;
}

// Helper returns true if the given string is in the vector of strings.
static bool IsStrInList(const STRING& str,
                        const GenericVector<STRING>& str_list) {
  for (int i = 0; i < str_list.size(); ++i) {
    if (str_list[i] == str)
      return true;
  }
  return false;
}

// Parse a string of the form [~]<lang>[+[~]<lang>]*.
// Langs with no prefix get appended to to_load, provided they
// are not in there already.
// Langs with ~ prefix get appended to not_to_load, provided they are not in
// there already.
void Tesseract::ParseLanguageString(const char* lang_str,
                                    GenericVector<STRING>* to_load,
                                    GenericVector<STRING>* not_to_load) {
  STRING remains(lang_str);
  while (remains.length() > 0) {
    // Find the start of the lang code and which vector to add to.
    const char* start = remains.string();
    while (*start == '+')
      ++start;
    GenericVector<STRING>* target = to_load;
    if (*start == '~') {
      target = not_to_load;
      ++start;
    }
    // Find the index of the end of the lang code in string start.
    int end = strlen(start);
    const char* plus = strchr(start, '+');
    if (plus != NULL && plus - start < end)
      end = plus - start;
    STRING lang_code(start);
    lang_code.truncate_at(end);
    STRING next(start + end);
    remains = next;
    // Check whether lang_code is already in the target vector and add.
    if (!IsStrInList(lang_code, *target)) {
      if (tessdata_manager_debug_level)
        tprintf("Adding language '%s' to list\n", lang_code.string());
      target->push_back(lang_code);
    }
  }
}

// Initialize for potentially a set of languages defined by the language
// string and recursively any additional languages required by any language
// traineddata file (via tessedit_load_sublangs in its config) that is loaded.
// See init_tesseract_internal for args.
int Tesseract::init_tesseract(
    const char *arg0, const char *textbase, const char *language,
    OcrEngineMode oem, char **configs, int configs_size,
    const GenericVector<STRING> *vars_vec,
    const GenericVector<STRING> *vars_values,
    bool set_only_non_debug_params) {
  GenericVector<STRING> langs_to_load;
  GenericVector<STRING> langs_not_to_load;
  ParseLanguageString(language, &langs_to_load, &langs_not_to_load);

  sub_langs_.delete_data_pointers();
  sub_langs_.clear();
  // Find the first loadable lang and load into this.
  // Add any languages that this language requires
  bool loaded_primary = false;
  // Load the rest into sub_langs_.
  for (int lang_index = 0; lang_index < langs_to_load.size(); ++lang_index) {
    if (!IsStrInList(langs_to_load[lang_index], langs_not_to_load)) {
      const char *lang_str = langs_to_load[lang_index].string();
      Tesseract *tess_to_init;
      if (!loaded_primary) {
        tess_to_init = this;
      } else {
        tess_to_init = new Tesseract;
      }

      int result = tess_to_init->init_tesseract_internal(
          arg0, textbase, lang_str, oem, configs, configs_size,
          vars_vec, vars_values, set_only_non_debug_params);

      if (!loaded_primary) {
        if (result < 0) {
          tprintf("Failed loading language '%s'\n", lang_str);
        } else {
          if (tessdata_manager_debug_level)
            tprintf("Loaded language '%s' as main language\n", lang_str);
          ParseLanguageString(tess_to_init->tessedit_load_sublangs.string(),
                              &langs_to_load, &langs_not_to_load);
          loaded_primary = true;
        }
      } else {
        if (result < 0) {
          tprintf("Failed loading language '%s'\n", lang_str);
          delete tess_to_init;
        } else {
          if (tessdata_manager_debug_level)
            tprintf("Loaded language '%s' as secondary language\n", lang_str);
          sub_langs_.push_back(tess_to_init);
          // Add any languages that this language requires
          ParseLanguageString(tess_to_init->tessedit_load_sublangs.string(),
                              &langs_to_load, &langs_not_to_load);
        }
      }
    }
  }
  if (!loaded_primary) {
    tprintf("Tesseract couldn't load any languages!\n");
    return -1;  // Couldn't load any language!
  }
  if (!sub_langs_.empty()) {
    // In multilingual mode word ratings have to be directly comparable,
    // so use the same language model weights for all languages:
    // use the primary language's params model if
    // tessedit_use_primary_params_model is set,
    // otherwise use default language model weights.
    if (tessedit_use_primary_params_model) {
      for (int s = 0; s < sub_langs_.size(); ++s) {
        sub_langs_[s]->language_model_->getParamsModel().Copy(
            this->language_model_->getParamsModel());
      }
      tprintf("Using params model of the primary language\n");
      if (tessdata_manager_debug_level)  {
        this->language_model_->getParamsModel().Print();
      }
    } else {
      this->language_model_->getParamsModel().Clear();
      for (int s = 0; s < sub_langs_.size(); ++s) {
        sub_langs_[s]->language_model_->getParamsModel().Clear();
      }
      if (tessdata_manager_debug_level)
        tprintf("Using default language params\n");
    }
  }

  SetupUniversalFontIds();
  return 0;
}

// Common initialization for a single language.
// arg0 is the datapath for the tessdata directory, which could be the
// path of the tessdata directory with no trailing /, or (if tessdata
// lives in the same directory as the executable, the path of the executable,
// hence the name arg0.
// textbase is an optional output file basename (used only for training)
// language is the language code to load.
// oem controls which engine(s) will operate on the image
// configs (argv) is an array of config filenames to load variables from.
// May be NULL.
// configs_size (argc) is the number of elements in configs.
// vars_vec is an optional vector of variables to set.
// vars_values is an optional corresponding vector of values for the variables
// in vars_vec.
// If set_only_init_params is true, then only the initialization variables
// will be set.
int Tesseract::init_tesseract_internal(
    const char *arg0, const char *textbase, const char *language,
    OcrEngineMode oem, char **configs, int configs_size,
    const GenericVector<STRING> *vars_vec,
    const GenericVector<STRING> *vars_values,
    bool set_only_non_debug_params) {
  if (!init_tesseract_lang_data(arg0, textbase, language, oem, configs,
                                configs_size, vars_vec, vars_values,
                                set_only_non_debug_params)) {
    return -1;
  }
  if (tessedit_init_config_only) {
    tessdata_manager.End();
    return 0;
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

// Helper builds the all_fonts table by adding new fonts from new_fonts.
static void CollectFonts(const UnicityTable<FontInfo>& new_fonts,
                         UnicityTable<FontInfo>* all_fonts) {
  for (int i = 0; i < new_fonts.size(); ++i) {
    // UnicityTable uniques as we go.
    all_fonts->push_back(new_fonts.get(i));
  }
}

// Helper assigns an id to lang_fonts using the index in all_fonts table.
static void AssignIds(const UnicityTable<FontInfo>& all_fonts,
                      UnicityTable<FontInfo>* lang_fonts) {
  for (int i = 0; i < lang_fonts->size(); ++i) {
    int index = all_fonts.get_id(lang_fonts->get(i));
    lang_fonts->get_mutable(i)->universal_id = index;
  }
}

// Set the universal_id member of each font to be unique among all
// instances of the same font loaded.
void Tesseract::SetupUniversalFontIds() {
  // Note that we can get away with bitwise copying FontInfo in
  // all_fonts, as it is a temporary structure and we avoid setting the
  // delete callback.
  UnicityTable<FontInfo> all_fonts;
  all_fonts.set_compare_callback(NewPermanentTessCallback(CompareFontInfo));

  // Create the universal ID table.
  CollectFonts(get_fontinfo_table(), &all_fonts);
  for (int i = 0; i < sub_langs_.size(); ++i) {
    CollectFonts(sub_langs_[i]->get_fontinfo_table(), &all_fonts);
  }
  // Assign ids from the table to each font table.
  AssignIds(all_fonts, &get_fontinfo_table());
  for (int i = 0; i < sub_langs_.size(); ++i) {
    AssignIds(all_fonts, &sub_langs_[i]->get_fontinfo_table());
  }
  font_table_size_ = all_fonts.size();
}

// init the LM component
int Tesseract::init_tesseract_lm(const char *arg0,
                   const char *textbase,
                   const char *language) {
  if (!init_tesseract_lang_data(arg0, textbase, language, OEM_TESSERACT_ONLY,
                                NULL, 0, NULL, NULL, false))
    return -1;
  getDict().Load(Dict::GlobalDawgCache());
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
