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
#include "permnum.h"
#include "stopper.h"
#include "adaptmatch.h"
#include "intmatcher.h"
#include "chop.h"
#include "efio.h"
#include "danerror.h"
#include "globals.h"

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

//extern "C" {
#include          "callnet.h"    //phils nn stuff
//}
#include          "notdll.h"     //phils nn stuff

#define VARDIR        "configs/" /*variables files */
                                 //config under api
#define API_CONFIG      "configs/api_config"
#define EXTERN

EXTERN STRING_VAR (tessedit_char_blacklist, "",
                   "Blacklist of chars not to recognize");
EXTERN STRING_VAR (tessedit_char_whitelist, "",
                   "Whitelist of chars to recognize");
EXTERN BOOL_EVAR (tessedit_write_vars, FALSE, "Write all vars to file");
EXTERN BOOL_VAR (tessedit_tweaking_tess_vars, FALSE,
"Fiddle tess config values");

EXTERN double_VAR (tweak_garbage, 1.5, "Tess VAR");
EXTERN double_VAR (tweak_ok_word, 1.25, "Tess VAR");
EXTERN double_VAR (tweak_good_word, 1.1, "Tess VAR");
EXTERN double_VAR (tweak_freq_word, 1.0, "Tess VAR");
EXTERN double_VAR (tweak_ok_number, 1.4, "Tess VAR");
EXTERN double_VAR (tweak_good_number, 1.1, "Tess VAR");
EXTERN double_VAR (tweak_non_word, 1.25, "Tess VAR");
EXTERN double_VAR (tweak_CertaintyPerChar, -0.5, "Tess VAR");
EXTERN double_VAR (tweak_NonDictCertainty, -2.5, "Tess VAR");
EXTERN double_VAR (tweak_RejectCertaintyOffset, 1.0, "Tess VAR");
EXTERN double_VAR (tweak_GoodAdaptiveMatch, 0.125, "Tess VAR");
EXTERN double_VAR (tweak_GreatAdaptiveMatch, 0.10, "Tess VAR");
EXTERN INT_VAR (tweak_ReliableConfigThreshold, 2, "Tess VAR");
EXTERN INT_VAR (tweak_AdaptProtoThresh, 230, "Tess VAR");
EXTERN INT_VAR (tweak_AdaptFeatureThresh, 230, "Tess VAR");
EXTERN INT_VAR (tweak_min_outline_points, 6, "Tess VAR");
EXTERN INT_VAR (tweak_min_outline_area, 2000, "Tess VAR");
EXTERN double_VAR (tweak_good_split, 50.0, "Tess VAR");
EXTERN double_VAR (tweak_ok_split, 100.0, "Tess VAR");

extern inT16 XOFFSET;
extern inT16 YOFFSET;
extern int NO_BLOCK;

                                 //progress monitor
ETEXT_DESC *global_monitor = NULL;

void init_tesseract_lang_data(const char *arg0,
                   const char *textbase,
                   const char *language,
                   const char *configfile,
                   int configc,
                   const char *const *configv) {
  FILE *var_file;
  static char c_path[MAX_PATH];  //path for c code

  // Set the basename, compute the data directory and read C++ configs.
  main_setup(arg0, textbase, configc, configv);
  debug_window_on.set_value (FALSE);

  if (tessedit_write_vars) {
    var_file = fopen ("edited.cfg", "w");
    if (var_file != NULL) {
      print_variables(var_file);
      fclose(var_file);
    }
  }
  strcpy (c_path, datadir.string ());
  c_path[strlen (c_path) - strlen (m_data_sub_dir.string ())] = '\0';
  demodir = c_path;

  // Set the language data path prefix
  language_data_path_prefix = datadir;
  if (language != NULL)
    language_data_path_prefix += language;
  else
    language_data_path_prefix += "eng";
  language_data_path_prefix += ".";

  // Load the unichar set
  STRING unicharpath = language_data_path_prefix;
  unicharpath += "unicharset";
  if (!unicharset.load_from_file(unicharpath.string())) {
    cprintf("Unable to load unicharset file %s\n", unicharpath.string());
    exit(1);
  }
  if (unicharset.size() > MAX_NUM_CLASSES) {
    cprintf("Error: Size of unicharset is greater than MAX_NUM_CLASSES\n");
    exit(1);
  }
  // Set the white and blacklists (if any)
  unicharset.set_black_and_whitelist(tessedit_char_blacklist.string(),
                                     tessedit_char_whitelist.string());
}

int init_tesseract(const char *arg0,
                   const char *textbase,
                   const char *language,
                   const char *configfile,
                   int configc,
                   const char *const *configv) {
  init_tesseract_lang_data (arg0, textbase, language,
    configfile, configc, configv);

  start_recog(configfile, textbase);

  set_tess_tweak_vars();

  if (tessedit_use_nn)           //phils nn stuff
    init_net();
  return 0;                      //Normal exit
}

// init the LM component
int init_tesseract_lm(const char *arg0,
                   const char *textbase,
                   const char *language,
                   const char *configfile,
                   int configc,
                   const char *const *configv) {
  init_tesseract_lang_data (arg0, textbase, language,
    configfile, configc, configv);

  init_permute();

  return 0;                      //Normal exit
}

void end_tesseract() {
  end_recog();
}

#ifdef _TIFFIO_
void read_tiff_image(TIFF* tif, IMAGE* image) {
  tdata_t buf;
  uint32 image_width, image_height;
  uint16 photometric;
  inT16 bpp;
  inT16 samples_per_pixel = 0;
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &image_width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &image_height);
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bpp);
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

/* Define command type identifiers */

enum CMD_EVENTS
{
  ACTION_1_CMD_EVENT,
  RECOG_WERDS,
  RECOG_PSEUDO,
  ACTION_2_CMD_EVENT
};


/*************************************************************************
 * set_tess_tweak_vars()
 * Set TESS vars from the tweek value - This is only really of use during search
 * of the space of tess configs - othertimes the default values are set
 *
 *************************************************************************/
void set_tess_tweak_vars() {
  if (tessedit_tweaking_tess_vars) {
    garbage = tweak_garbage;
    ok_word = tweak_ok_word;
    good_word = tweak_good_word;
    freq_word = tweak_freq_word;
    ok_number = tweak_ok_number;
    good_number = tweak_good_number;
    non_word = tweak_non_word;
    CertaintyPerChar = tweak_CertaintyPerChar;
    NonDictCertainty = tweak_NonDictCertainty;
    RejectCertaintyOffset = tweak_RejectCertaintyOffset;
    GoodAdaptiveMatch = tweak_GoodAdaptiveMatch;
    GreatAdaptiveMatch = tweak_GreatAdaptiveMatch;
    ReliableConfigThreshold = tweak_ReliableConfigThreshold;
    AdaptProtoThresh = tweak_AdaptProtoThresh;
    AdaptFeatureThresh = tweak_AdaptFeatureThresh;
    min_outline_points = tweak_min_outline_points;
    min_outline_area = tweak_min_outline_area;
    good_split = tweak_good_split;
    ok_split = tweak_ok_split;
  }
  //   if (expiry_day * 24 * 60 * 60 < time(NULL))
  //         err_exit();
}
