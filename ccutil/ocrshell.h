/**********************************************************************
 * File:			ocrshell.h
 * Description:	Code for the OCR side of the OCR API.
 * Author:		Hewlett-Packard Co
 *
 * (C) Copyright 1996, Hewlett-Packard Co.
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

#ifndef           OCRSHELL_H
#define           OCRSHELL_H

/**********************************************************************
 * This file contains code for the OCR side of the HP OCR interface.
 * The code is designed to be used with either an ANSI C or C++ compiler.
 * The structures are designed to allow them to be used with any
 * structure alignment upto 8.
 **********************************************************************/

#include          "ocrclass.h"

#define EUC_FORMAT_MASK   0xe0

/**********************************************************************
 * ocr_open_shm
 *
 * Attempt to connect to the shared memory segment and semaphores used
 * in talking to the OCR engine. Called from OCR engine.
 * The parameters are the command line arguments in order.
 * The final parameter is a return value indicating the user-requested
 * language.					The value will be LANGE_NONE if the user wishes to use
 * the default.
 **********************************************************************/
#ifdef __MAC__
inT16 ocr_open_shm(uinT16 *lang);
#else
inT16 ocr_open_shm(                       /*open the shm */
                   const char *shm_h,     /*handle of shm */
                   const char *shm_size,  /*size of shm segment */
                   const char *mutex_h,   /*hp mutex */
                   const char *ocr_h,     /*ocr semaphore */
                   const char *hp_h,      /*hp semaphore */
                   const char *lang_str,  /*language */
                   uinT16 *lang           /*required language */
                  );
#endif

/**********************************************************************
 * ocr_append_fontinfo
 *
 * Initialize one of the font descriptors.
 **********************************************************************/

inT16 ocr_append_fontinfo(                    /*put info into shm */
                          uinT16 language,    /*default language */
                          uinT8 font_family,  /*serif/not, fixed/not */
                          uinT8 char_set,     /*character set standard */
                          uinT8 pitch,        /*fixed or prop */
                          const char *name    /*plain ascii name */
                         );

/**********************************************************************
 * ocr_setup_startinfo
 *
 * Setup the info on the OCR engine. Uses 16 bit chars to name the
 * engine.
 **********************************************************************/

inT16 ocr_setup_startinfo(                       /*put info into shm */
                          uinT32 protocol,       /*interface version */
                          uinT16 language,       /*default language */
                          const uinT16 *name,    /*name of engine */
                          const uinT16 *version  /*version of engine */
                         );

/**********************************************************************
 * ocr_setup_startinfo_ansi
 *
 * Setup the info on the OCR engine. Uses 8 bit chars to name the
 * engine.
 **********************************************************************/

inT16 ocr_setup_startinfo_ansi(                     /*put info into shm */
                               uinT32 protocol,     /*interface version */
                               uinT16 language,     /*default language */
                               const char *name,    /*name of engine */
                               const char *version  /*version of engine */
                              );

/**********************************************************************
 * ocr_get_first_image_strip
 *
 * Wait for the master to send the first image strip and return a
 * pointer to it. The result is NULL if it is time to exit.
 **********************************************************************/

                                 /*get image strip */
ESTRIP_DESC *ocr_get_first_image_strip();

/**********************************************************************
 * ocr_get_next_image_strip
 *
 * Wait for the master to send the next image strip and return a
 * pointer to it. The result is NULL if it is time to exit.
 **********************************************************************/

                                 /*get image strip */
ESTRIP_DESC *ocr_get_next_image_strip();

/**********************************************************************
 * ocr_setup_monitor
 *
 * Setup the progress monitor. Call before starting the recognize task.
 **********************************************************************/

ETEXT_DESC *ocr_setup_monitor();  /*setup monitor */

/**********************************************************************
 * ocr_char_space
 *
 * Return the number of chars that can be fitted into the buffer.
 **********************************************************************/

inT32 ocr_char_space();  /*put char into shm */

/**********************************************************************
 * ocr_append_char
 *
 * Add a character to the output. Returns OKAY if successful, HPERR
 * if there was insufficient room in the buffer.
 **********************************************************************/

inT16 ocr_append_char(                              /*put char into shm */
                      uinT16 char_code,             /*character itself */
                      inT16 left,                   /*of char (-1) */
                      inT16 right,                  /*of char (-1) */
                      inT16 top,                    /*of char (-1) */
                      inT16 bottom,                 /*of char (-1) */
                      inT16 font_index,             /*what font (-1) */
                      uinT8 confidence,             /*0=perfect, 100=reject (0/100) */
                      uinT8 point_size,             /*of char, 72=i inch, (10) */
                      inT8 blanks,                  /*no of spaces before this char (1) */
                      uinT8 enhancement,            /*char enhancement (0) */
                      OCR_CHAR_DIRECTION text_dir,  /*rendering direction (OCR_CDIR_RIGHT_LEFT) */
                      OCR_LINE_DIRECTION line_dir,  /*line rendering direction (OCR_LDIR_DOWN_RIGHT) */
                      OCR_NEWLINE_TYPE nl_type      /*type of newline (if any) (OCR_NL_NONE) */
                     );

/**********************************************************************
 * ocr_send_text
 *
 * Send the text to the host and wait for the ack.
 * Use this function after a sequence of ocr_append_text calls to
 * actually sent the text to the master process.
 * Set more to come TRUE if there is more text in this page, FALSE
 * if the OCR engine is now ready to receive another image.
 **********************************************************************/

inT16 ocr_send_text(                    /*send shm */
                    BOOL8 more_to_come  /*any text left */
                   );

/**********************************************************************
 * ocr_shutdown
 *
 * Closedown communications with the HP side and free up handles.
 **********************************************************************/

inT16 ocr_shutdown();  /*closedown */

/**********************************************************************
 * ocr_error
 *
 * Inform the HP side of an error.
 * The OCR engine should do any cleanup of its own and exit aferwards.
 * Uses the current state to determine how to send it and cleanup.
 **********************************************************************/

void ocr_error(                   /*send an error code */
               OCR_ERR_CODE code  /*error code */
              );
#endif
