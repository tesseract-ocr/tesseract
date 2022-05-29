/**********************************************************************
 * File:        tessedit.cpp  (Formerly tessedit.c)
 * Description: (Previously) Main program for merge of tess and editor.
 *              Now just code to load the language model and various
 *              engine-specific data files.
 * Author:      Ray Smith
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "control.h"
#include "matchdefs.h"
#include "pageres.h"
#include "params.h"
#include "stopper.h"
#include "tesseractclass.h"
#include "tessvars.h"
#include "tprintf.h"
#ifndef DISABLED_LEGACY_ENGINE
#  include "chop.h"
#  include "intmatcher.h"
#  include "reject.h"
#endif
#include "lstmrecognizer.h"

namespace tesseract {

// Read a "config" file containing a set of variable, value pairs.
// Searches the standard places: tessdata/configs, tessdata/tessconfigs
// and also accepts a relative or absolute path name.
void Tesseract::read_config_file(const char *filename, SetParamConstraint constraint) {
  std::string path = datadir;
  path += "configs/";
  path += filename;
  FILE *fp;
  if ((fp = fopen(path.c_str(), "rb")) != nullptr) {
    fclose(fp);
  } else {
    path = datadir;
    path += "tessconfigs/";
    path += filename;
    if ((fp = fopen(path.c_str(), "rb")) != nullptr) {
      fclose(fp);
    } else {
      path = filename;
    }
  }
  ParamUtils::ReadParamsFile(path.c_str(), constraint, this->params());
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
bool Tesseract::init_tesseract_lang_data(const std::string &arg0,
                                         const std::string &language, OcrEngineMode oem,
                                         char **configs, int configs_size,
                                         const std::vector<std::string> *vars_vec,
                                         const std::vector<std::string> *vars_values,
                                         bool set_only_non_debug_params, TessdataManager *mgr) {
  // Set the language data path prefix
  lang = !language.empty() ? language : "eng";
  language_data_path_prefix = datadir;
  language_data_path_prefix += lang;
  language_data_path_prefix += ".";

  // Initialize TessdataManager.
  std::string tessdata_path = language_data_path_prefix + kTrainedDataSuffix;
  if (!mgr->is_loaded() && !mgr->Init(tessdata_path.c_str())) {
    tprintf("Error opening data file %s\n", tessdata_path.c_str());
    tprintf(
        "Please make sure the TESSDATA_PREFIX environment variable is set"
        " to your \"tessdata\" directory.\n");
    return false;
  }
#ifdef DISABLED_LEGACY_ENGINE
  tessedit_ocr_engine_mode.set_value(OEM_LSTM_ONLY);
#else
  if (oem == OEM_DEFAULT) {
    // Set the engine mode from availability, which can then be overridden by
    // the config file when we read it below.
    if (!mgr->IsLSTMAvailable()) {
      tessedit_ocr_engine_mode.set_value(OEM_TESSERACT_ONLY);
    } else if (!mgr->IsBaseAvailable()) {
      tessedit_ocr_engine_mode.set_value(OEM_LSTM_ONLY);
    } else {
      tessedit_ocr_engine_mode.set_value(OEM_TESSERACT_LSTM_COMBINED);
    }
  }
#endif // ndef DISABLED_LEGACY_ENGINE

  // If a language specific config file (lang.config) exists, load it in.
  TFile fp;
  if (mgr->GetComponent(TESSDATA_LANG_CONFIG, &fp)) {
    ParamUtils::ReadParamsFromFp(SET_PARAM_CONSTRAINT_NONE, &fp, this->params());
  }

  SetParamConstraint set_params_constraint =
      set_only_non_debug_params ? SET_PARAM_CONSTRAINT_NON_DEBUG_ONLY : SET_PARAM_CONSTRAINT_NONE;
  // Load tesseract variables from config files. This is done after loading
  // language-specific variables from [lang].traineddata file, so that custom
  // config files can override values in [lang].traineddata file.
  for (int i = 0; i < configs_size; ++i) {
    read_config_file(configs[i], set_params_constraint);
  }

  // Set params specified in vars_vec (done after setting params from config
  // files, so that params in vars_vec can override those from files).
  if (vars_vec != nullptr && vars_values != nullptr) {
    for (unsigned i = 0; i < vars_vec->size(); ++i) {
      if (!ParamUtils::SetParam((*vars_vec)[i].c_str(), (*vars_values)[i].c_str(),
                                set_params_constraint, this->params())) {
        tprintf("Warning: The parameter '%s' was not found.\n", (*vars_vec)[i].c_str());
      }
    }
  }

  if (!tessedit_write_params_to_file.empty()) {
    FILE *params_file = fopen(tessedit_write_params_to_file.c_str(), "wb");
    if (params_file != nullptr) {
      ParamUtils::PrintParams(params_file, this->params());
      fclose(params_file);
    } else {
      tprintf("Failed to open %s for writing params.\n", tessedit_write_params_to_file.c_str());
    }
  }

#ifndef DISABLED_LEGACY_ENGINE
  // Determine which ocr engine(s) should be loaded and used for recognition.
  if (oem != OEM_DEFAULT) {
    tessedit_ocr_engine_mode.set_value(oem);
  }
#endif

  // If we are only loading the config file (and so not planning on doing any
  // recognition) then there's nothing else do here.
  if (tessedit_init_config_only) {
    return true;
  }

// The various OcrEngineMode settings (see tesseract/publictypes.h) determine
// which engine-specific data files need to be loaded. If LSTM_ONLY is
// requested, the base Tesseract files are *Not* required.
#ifdef DISABLED_LEGACY_ENGINE
  if (tessedit_ocr_engine_mode == OEM_LSTM_ONLY) {
#else
  if (tessedit_ocr_engine_mode == OEM_LSTM_ONLY ||
      tessedit_ocr_engine_mode == OEM_TESSERACT_LSTM_COMBINED) {
#endif // ndef DISABLED_LEGACY_ENGINE
    if (mgr->IsComponentAvailable(TESSDATA_LSTM)) {
      lstm_recognizer_ = new LSTMRecognizer(language_data_path_prefix.c_str());
      ASSERT_HOST(lstm_recognizer_->Load(this->params(), lstm_use_matrix ? language : "", mgr));
    } else {
      tprintf("Error: LSTM requested, but not present!! Loading tesseract.\n");
      tessedit_ocr_engine_mode.set_value(OEM_TESSERACT_ONLY);
    }
  }

  // Load the unicharset
  if (tessedit_ocr_engine_mode == OEM_LSTM_ONLY) {
    // Avoid requiring a unicharset when we aren't running base tesseract.
    unicharset.CopyFrom(lstm_recognizer_->GetUnicharset());
  }
#ifndef DISABLED_LEGACY_ENGINE
  else if (!mgr->GetComponent(TESSDATA_UNICHARSET, &fp) || !unicharset.load_from_file(&fp, false)) {
    tprintf(
        "Error: Tesseract (legacy) engine requested, but components are "
        "not present in %s!!\n",
        tessdata_path.c_str());
    return false;
  }
#endif // ndef DISABLED_LEGACY_ENGINE
  if (unicharset.size() > MAX_NUM_CLASSES) {
    tprintf("Error: Size of unicharset is greater than MAX_NUM_CLASSES\n");
    return false;
  }
  right_to_left_ = unicharset.major_right_to_left();

#ifndef DISABLED_LEGACY_ENGINE

  // Setup initial unichar ambigs table and read universal ambigs.
  UNICHARSET encoder_unicharset;
  encoder_unicharset.CopyFrom(unicharset);
  unichar_ambigs.InitUnicharAmbigs(unicharset, use_ambigs_for_adaption);
  unichar_ambigs.LoadUniversal(encoder_unicharset, &unicharset);

  if (!tessedit_ambigs_training && mgr->GetComponent(TESSDATA_AMBIGS, &fp)) {
    unichar_ambigs.LoadUnicharAmbigs(encoder_unicharset, &fp, ambigs_debug_level,
                                     use_ambigs_for_adaption, &unicharset);
  }

  // Init ParamsModel.
  // Load pass1 and pass2 weights (for now these two sets are the same, but in
  // the future separate sets of weights can be generated).
  for (int p = ParamsModel::PTRAIN_PASS1; p < ParamsModel::PTRAIN_NUM_PASSES; ++p) {
    language_model_->getParamsModel().SetPass(static_cast<ParamsModel::PassEnum>(p));
    if (mgr->GetComponent(TESSDATA_PARAMS_MODEL, &fp)) {
      if (!language_model_->getParamsModel().LoadFromFp(lang.c_str(), &fp)) {
        return false;
      }
    }
  }
#endif // ndef DISABLED_LEGACY_ENGINE

  return true;
}

// Helper returns true if the given string is in the vector of strings.
static bool IsStrInList(const std::string &str, const std::vector<std::string> &str_list) {
  for (const auto &i : str_list) {
    if (i == str) {
      return true;
    }
  }
  return false;
}

// Parse a string of the form [~]<lang>[+[~]<lang>]*.
// Langs with no prefix get appended to to_load, provided they
// are not in there already.
// Langs with ~ prefix get appended to not_to_load, provided they are not in
// there already.
void Tesseract::ParseLanguageString(const std::string &lang_str, std::vector<std::string> *to_load,
                                    std::vector<std::string> *not_to_load) {
  std::string remains(lang_str);
  // Look whether the model file uses a prefix which must be applied to
  // included model files as well.
  std::string prefix;
  size_t found = lang.find_last_of('/');
  if (found != std::string::npos) {
    // A prefix was found.
    prefix = lang.substr(0, found + 1);
  }
  while (!remains.empty()) {
    // Find the start of the lang code and which vector to add to.
    const char *start = remains.c_str();
    while (*start == '+') {
      ++start;
    }
    std::vector<std::string> *target = to_load;
    if (*start == '~') {
      target = not_to_load;
      ++start;
    }
    // Find the index of the end of the lang code in string start.
    int end = strlen(start);
    const char *plus = strchr(start, '+');
    if (plus != nullptr && plus - start < end) {
      end = plus - start;
    }
    std::string lang_code(start);
    lang_code.resize(end);
    std::string next(start + end);
    remains = next;
    lang_code = prefix + lang_code;
    // Check whether lang_code is already in the target vector and add.
    if (!IsStrInList(lang_code, *target)) {
      target->push_back(lang_code);
    }
  }
}

// Initialize for potentially a set of languages defined by the language
// string and recursively any additional languages required by any language
// traineddata file (via tessedit_load_sublangs in its config) that is loaded.
// See init_tesseract_internal for args.
int Tesseract::init_tesseract(const std::string &arg0, const std::string &textbase,
                              const std::string &language, OcrEngineMode oem, char **configs,
                              int configs_size, const std::vector<std::string> *vars_vec,
                              const std::vector<std::string> *vars_values,
                              bool set_only_non_debug_params, TessdataManager *mgr) {
  std::vector<std::string> langs_to_load;
  std::vector<std::string> langs_not_to_load;
  ParseLanguageString(language, &langs_to_load, &langs_not_to_load);

  for (auto *lang : sub_langs_) {
    delete lang;
  }

  // Set the basename, compute the data directory.
  main_setup(arg0, textbase);

  sub_langs_.clear();
  // Find the first loadable lang and load into this.
  // Add any languages that this language requires
  bool loaded_primary = false;
  // Load the rest into sub_langs_.
  // A range based for loop does not work here because langs_to_load
  // might be changed in the loop when a new submodel is found.
  for (auto &lang_to_load : langs_to_load) {
    if (!IsStrInList(lang_to_load, langs_not_to_load)) {
      const char *lang_str = lang_to_load.c_str();
      Tesseract *tess_to_init;
      if (!loaded_primary) {
        tess_to_init = this;
      } else {
        tess_to_init = new Tesseract;
        tess_to_init->main_setup(arg0, textbase);
      }

      int result = tess_to_init->init_tesseract_internal(arg0, textbase, lang_str, oem, configs,
                                                         configs_size, vars_vec, vars_values,
                                                         set_only_non_debug_params, mgr);
      // Forget that language, but keep any reader we were given.
      mgr->Clear();

      if (!loaded_primary) {
        if (result < 0) {
          tprintf("Failed loading language '%s'\n", lang_str);
        } else {
          ParseLanguageString(tess_to_init->tessedit_load_sublangs, &langs_to_load,
                              &langs_not_to_load);
          loaded_primary = true;
        }
      } else {
        if (result < 0) {
          tprintf("Failed loading language '%s'\n", lang_str);
          delete tess_to_init;
        } else {
          sub_langs_.push_back(tess_to_init);
          // Add any languages that this language requires
          ParseLanguageString(tess_to_init->tessedit_load_sublangs, &langs_to_load,
                              &langs_not_to_load);
        }
      }
    }
  }
  if (!loaded_primary && !langs_to_load.empty()) {
    tprintf("Tesseract couldn't load any languages!\n");
    return -1; // Couldn't load any language!
  }
#ifndef DISABLED_LEGACY_ENGINE
  if (!sub_langs_.empty()) {
    // In multilingual mode word ratings have to be directly comparable,
    // so use the same language model weights for all languages:
    // use the primary language's params model if
    // tessedit_use_primary_params_model is set,
    // otherwise use default language model weights.
    if (tessedit_use_primary_params_model) {
      for (auto &sub_lang : sub_langs_) {
        sub_lang->language_model_->getParamsModel().Copy(this->language_model_->getParamsModel());
      }
      tprintf("Using params model of the primary language\n");
    } else {
      this->language_model_->getParamsModel().Clear();
      for (auto &sub_lang : sub_langs_) {
        sub_lang->language_model_->getParamsModel().Clear();
      }
    }
  }

  SetupUniversalFontIds();
#endif // ndef DISABLED_LEGACY_ENGINE
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
// May be nullptr.
// configs_size (argc) is the number of elements in configs.
// vars_vec is an optional vector of variables to set.
// vars_values is an optional corresponding vector of values for the variables
// in vars_vec.
// If set_only_non_debug_params is true, only params that do not contain
// "debug" in the name will be set.
int Tesseract::init_tesseract_internal(const std::string &arg0, const std::string &textbase,
                                       const std::string &language, OcrEngineMode oem,
                                       char **configs, int configs_size,
                                       const std::vector<std::string> *vars_vec,
                                       const std::vector<std::string> *vars_values,
                                       bool set_only_non_debug_params, TessdataManager *mgr) {
  if (!init_tesseract_lang_data(arg0, language, oem, configs, configs_size, vars_vec,
                                vars_values, set_only_non_debug_params, mgr)) {
    return -1;
  }
  if (tessedit_init_config_only) {
    return 0;
  }
  // If only LSTM will be used, skip loading Tesseract classifier's
  // pre-trained templates and dictionary.
  bool init_tesseract = tessedit_ocr_engine_mode != OEM_LSTM_ONLY;
  program_editup(textbase, init_tesseract ? mgr : nullptr, init_tesseract ? mgr : nullptr);
  return 0; // Normal exit
}

#ifndef DISABLED_LEGACY_ENGINE

// Helper builds the all_fonts table by adding new fonts from new_fonts.
static void CollectFonts(const UnicityTable<FontInfo> &new_fonts,
                         UnicityTable<FontInfo> *all_fonts) {
  for (int i = 0; i < new_fonts.size(); ++i) {
    // UnicityTable uniques as we go.
    all_fonts->push_back(new_fonts.at(i));
  }
}

// Helper assigns an id to lang_fonts using the index in all_fonts table.
static void AssignIds(const UnicityTable<FontInfo> &all_fonts, UnicityTable<FontInfo> *lang_fonts) {
  for (int i = 0; i < lang_fonts->size(); ++i) {
    auto index = all_fonts.get_index(lang_fonts->at(i));
    lang_fonts->at(i).universal_id = index;
  }
}

// Set the universal_id member of each font to be unique among all
// instances of the same font loaded.
void Tesseract::SetupUniversalFontIds() {
  // Note that we can get away with bitwise copying FontInfo in
  // all_fonts, as it is a temporary structure and we avoid setting the
  // delete callback.
  UnicityTable<FontInfo> all_fonts;

  // Create the universal ID table.
  CollectFonts(get_fontinfo_table(), &all_fonts);
  for (auto &sub_lang : sub_langs_) {
    CollectFonts(sub_lang->get_fontinfo_table(), &all_fonts);
  }
  // Assign ids from the table to each font table.
  AssignIds(all_fonts, &get_fontinfo_table());
  for (auto &sub_lang : sub_langs_) {
    AssignIds(all_fonts, &sub_lang->get_fontinfo_table());
  }
  font_table_size_ = all_fonts.size();
}

#endif // ndef DISABLED_LEGACY_ENGINE

void Tesseract::end_tesseract() {
  end_recog();
}

/* Define command type identifiers */

enum CMD_EVENTS { ACTION_1_CMD_EVENT, RECOG_WERDS, RECOG_PSEUDO, ACTION_2_CMD_EVENT };
} // namespace tesseract
