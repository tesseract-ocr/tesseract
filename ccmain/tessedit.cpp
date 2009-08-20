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
#include          "mainblk.h"
#include          "nwmain.h"
#include          "pgedit.h"
#include          "ocrshell.h"
#include          "tprintf.h"
//#include                                      "ipeerr.h"
//#include                                                      "restart.h"
#include          "tessedit.h"
//#include                                                      "fontfind.h"
#include "permute.h"
#include "permdawg.h"
#include "stopper.h"
#include "adaptmatch.h"
#include "intmatcher.h"
#include "chop.h"
#include "efio.h"
#include "danerror.h"
#include "globals.h"
#include "tesseractclass.h"
#include "varable.h"

/*
** Include automatically generated configuration file if running autoconf
*/
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif
// Includes libtiff if HAVE_LIBTIFF is defined
#ifdef HAVE_LIBTIFF
#include "tiffio.h"

#endif

#include          "notdll.h"     //phils nn stuff

#define VARDIR        "configs/" /*variables files */
                                 //config under api
#define API_CONFIG      "configs/api_config"
#define EXTERN

EXTERN BOOL_EVAR (tessedit_write_vars, FALSE, "Write all vars to file");

ETEXT_DESC *global_monitor = NULL;  // progress monitor

namespace tesseract {

// Read a "config" file containing a set of variable, value pairs.
// Searches the standard places: tessdata/configs, tessdata/tessconfigs
// and also accepts a relative or absolute path name.
void Tesseract::read_config_file(const char *filename, bool global_only) {
  STRING path = datadir;
  path += "configs/";
  path += filename;
  FILE* fp;
  if ((fp = fopen(path.string(), "r")) != NULL) {
    fclose(fp);
  } else {
    path = datadir;
    path += "tessconfigs/";
    path += filename;
    if ((fp = fopen(path.string(), "r")) != NULL) {
      fclose(fp);
    } else {
      path = filename;
    }
  }
  read_variables_file(path.string(), global_only);
}

// Returns false if a unicharset file for the specified language was not found
// or was invalid.
// This function initializes TessdataManager. After TessdataManager is
// no longer needed, TessdataManager::End() should be called.
bool Tesseract::init_tesseract_lang_data(
    const char *arg0, const char *textbase, const char *language,
    char **configs, int configs_size, bool configs_global_only) {
  FILE *var_file;
  static char c_path[MAX_PATH];  //path for c code

  // Set the basename, compute the data directory.
  main_setup(arg0, textbase);
  debug_window_on.set_value (FALSE);

  if (tessedit_write_vars) {
    var_file = fopen ("edited.cfg", "w");
    if (var_file != NULL) {
      print_variables(var_file);
      fclose(var_file);
    }
  }
  strcpy (c_path, datadir.string());
  c_path[strlen (c_path) - strlen (m_data_sub_dir.string ())] = '\0';
  demodir = c_path;

  // Set the language data path prefix
  lang = language != NULL ? language : "eng";
  language_data_path_prefix = datadir;
  language_data_path_prefix += lang;
  language_data_path_prefix += ".";

  // Load tesseract variables from config files.
  for (int i = 0; i < configs_size; ++i) {
    read_config_file(configs[i], configs_global_only);
  }

  // Initialize TessdataManager.
  STRING tessdata_path = language_data_path_prefix + kTrainedDataSuffix;
  tessdata_manager.Init(tessdata_path.string());

  // If a language specific config file (lang.config) exists, load it in.
  if (tessdata_manager.SeekToStart(TESSDATA_LANG_CONFIG)) {
    read_variables_from_fp(tessdata_manager.GetDataFilePtr(),
                           tessdata_manager.GetEndOffset(TESSDATA_LANG_CONFIG),
                           false);
    if (global_tessdata_manager_debug_level) {
      tprintf("Loaded language config file\n");
    }
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
  if (global_tessdata_manager_debug_level) tprintf("Loaded unicharset\n");

  if (!global_tessedit_ambigs_training &&
      tessdata_manager.SeekToStart(TESSDATA_AMBIGS)) {
    unichar_ambigs.LoadUnicharAmbigs(
        tessdata_manager.GetDataFilePtr(),
        tessdata_manager.GetEndOffset(TESSDATA_AMBIGS),
        &unicharset);
    if (global_tessdata_manager_debug_level) tprintf("Loaded ambigs\n");
  }
  return true;
}

int Tesseract::init_tesseract(
    const char *arg0, const char *textbase, const char *language,
    char **configs, int configs_size, bool configs_global_only) {
  if (!init_tesseract_lang_data(arg0, textbase, language, configs,
                                configs_size, configs_global_only)) {
    return -1;
  }
  start_recog(textbase);
  tessdata_manager.End();
  return 0;                      //Normal exit
}

// Init everything except the language model
int Tesseract::init_tesseract_classifier(
    const char *arg0, const char *textbase, const char *language,
    char **configs, int configs_size, bool configs_global_only) {
  if (!init_tesseract_lang_data (arg0, textbase, language, configs,
                                 configs_size, configs_global_only)) {
    return -1;
  }
  // Dont initialize the permuter.
  program_editup(textbase, false);
  tessdata_manager.End();
  return 0;
}

// init the LM component
int Tesseract::init_tesseract_lm(const char *arg0,
                   const char *textbase,
                   const char *language) {
  init_tesseract_lang_data(arg0, textbase, language, NULL, 0, false);
  getDict().init_permute();
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

#ifdef _TIFFIO_
void read_tiff_image(TIFF* tif, IMAGE* image) {
  tdata_t buf;
  uint32 image_width, image_height;
  uint16 photometric;
  inT16 bpp;
  inT16 samples_per_pixel = 0;
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &image_width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &image_height);
  if (!TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bpp))
    bpp = 1;  // Binary is default if no value provided.
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel);
  TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
  if (samples_per_pixel > 1)
    bpp *= samples_per_pixel;
  // Tesseract's internal representation is 0-is-black,
  // so if the photometric is 1 (min is black) then high-valued pixels
  // are 1 (white), otherwise they are 0 (black).
  uinT8 high_value = photometric == 1;
  image->create(image_width, image_height, bpp);
  IMAGELINE line;
  line.init(image_width);

  buf = _TIFFmalloc(TIFFScanlineSize(tif));
  int bytes_per_line = (image_width*bpp + 7)/8;
  uinT8* dest_buf = image->get_buffer();
  // This will go badly wrong with one of the more exotic tiff formats,
  // but the majority will work OK.
  for (int y = 0; y < image_height; ++y) {
    TIFFReadScanline(tif, buf, y);
    memcpy(dest_buf, buf, bytes_per_line);
    dest_buf += bytes_per_line;
  }
  if (high_value == 0)
    invert_image(image);
  _TIFFfree(buf);
}
#endif
