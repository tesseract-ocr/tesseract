/**********************************************************************
 * File:			ocrshell.cpp
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

/**********************************************************************
 * This file contains code for the OCR side of the HP OCR interface.
 * The code is designed to be used with either an ANSI C or C++ compiler.
 * The structures are designed to allow them to be used with any
 * structure alignment upto 8.
 **********************************************************************/

#include          "mfcpch.h"
#include          "ocrshell.h"
#include          "tprintf.h"
#include          <stdlib.h>

#define EXTERN

#ifdef __UNIX__
EXTERN ESHM_INFO shm;            /*info on shm */
#define TICKS       1
#endif

#ifdef __MSW32__
EXTERN ESHM_INFO shm;            /*info on shm */
#define TICKS       1000
#endif

#ifdef __MAC__

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
#pragma import on
#endif

extern volatile ESHM_INFO shm;   /*info on shm */
extern unsigned short WaitForSingleObject(  /*"C" */
                                          volatile Boolean &semaphore,
                                          unsigned long timeout);
extern unsigned short ReleaseSemaphore(  /*"C" */
                                       volatile Boolean &semaphore);
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
#pragma import reset
#endif
#define WAIT_OBJECT_0   1
#define TICKS       60
#endif

typedef enum {
  OCS_UNINIT,                    /*uninitialized */
  OCS_SETUP_SHM,                 /*shm setup done */
  OCS_SETUP_INFO,                /*startinfo sent */
  OCS_READING_STRIPS,            /*read first but more to come */
  OCS_READ_STRIPS,               /*read all but no monitor yet */
  OCS_RECOGNIZING,               /*OCR incomplete */
  OCS_SENDING_TEXT,              /*sent buffer but more to come */
  OCS_DEAD                       /*disconnected */
} OCR_STATE;

/* forward declarations - not in .h file as not needed outside this file*/
inT16 ocr_internal_shutdown();  /*closedown */
inT16 wait_for_mutex();  /*wait for HP to be ready */
inT16 wait_for_hp(               /*wait for semaphore */
                  inT32 timeout  /*in seconds */
                 );
inT16 release_mutex();  /*release mutex */
inT16 release_ocr();  /*release semaphore */

static inT32 font_count = 0;     /*number of fonts */
static inT16 lines_read = 0;     /*no read in this image */
                                 /*current state */
static OCR_STATE ocr_state = OCS_UNINIT;

#ifdef __MAC__
pascal short TerminateOCR(AppleEvent *theEvent,
                          AppleEvent *theReply,
                          long refCon) {
  ocr_internal_shutdown();
  ExitToShell();

}
#endif

/**********************************************************************
 * ocr_open_shm
 *
 * Attempt to connect to the shared memory segment and semaphores used
 * in talking to the OCR engine. Called from OCR engine.
 * The parameters are the command line arguments in order.
 **********************************************************************/
#ifdef __MAC__
inT16
ocr_open_shm (uinT16 * lang)
#else
inT16
ocr_open_shm (                   /*open the shm */
const char *shm_h,               /*handle of shm */
const char *shm_size,            /*size of shm segment */
const char *mutex_h,             /*hp mutex */
const char *ocr_h,               /*ocr semaphore */
const char *hp_h,                /*hp semaphore */
const char *lang_str,            /*language */
uinT16 * lang                    /*required language */
)
#endif
{
  font_count = 0;                /*no fonts yet */
  #ifdef __MAC__
  if (shm.OCRProcess.lowLongOfPSN && shm.OCRProcess.highLongOfPSN)
    return HPERR;
  *lang = shm.language;
  GetCurrentProcess (&shm.OCRProcess);
  if (WakeUpProcess (&shm.IPEProcess))
    ExitToShell();
  AEInstallEventHandler (kCoreEventClass, kAEQuitApplication,
    (AEEventHandlerUPP) TerminateOCR, 0, FALSE);
  #else
  if (lang != NULL)
                                 /*get language */
    *lang = (uinT16) strtol (lang_str, NULL, 10);
  #endif
  if (ocr_state != OCS_UNINIT) {
    ocr_error(OCR_ERR_BAD_STATE);
    return OCR_API_BAD_STATE;    /*incorrect state */
  }
  #ifdef __MSW32__
  shm.shm_size = strtol (shm_size, NULL, 10);
                                 /*convert to handle */
  shm.shm_hand = (HANDLE) strtol (shm_h, NULL, 10);
  shm.shm_mem = MapViewOfFile (shm.shm_hand, FILE_MAP_WRITE, 0, 0, 0);
  if (shm.shm_mem == NULL)
    return HPERR;                /*failed */
                                 /*convert to handle */
  shm.mutex = (HANDLE) strtol (mutex_h, NULL, 10);
                                 /*convert to handle */
  shm.ocr_sem = (HANDLE) strtol (ocr_h, NULL, 10);
                                 /*convert to handle */
  shm.hp_sem = (HANDLE) strtol (hp_h, NULL, 10);
  #endif

  ocr_state = OCS_SETUP_SHM;     /*record state */
  return OKAY;

}


/**********************************************************************
 * ocr_error
 *
 * Inform the HP side of an error.
 * The OCR engine should do any cleanup of its own and exit aferwards.
 * Uses the current state to determine how to send it and cleanup.
 **********************************************************************/

void ocr_error(                   /*send an error code */
               OCR_ERR_CODE code  /*error code */
              ) {
  ESTRIP_DESC *strip = (ESTRIP_DESC *) shm.shm_mem;
  /*strip info */
  ETEXT_DESC *monitor = (ETEXT_DESC *) shm.shm_mem;
  /*progress monitor */

  switch (ocr_state) {
    case OCS_UNINIT:             /*uninitialized */
    case OCS_DEAD:               /*uninitialized */
      return;                    /*can't do anything else */
    case OCS_SETUP_SHM:          /*shm setup done */
      if (font_count < 1)
        font_count = 1;
      ocr_setup_startinfo_ansi (-code, LANGE_NONE, "", "");
      /*report error */
      break;
    case OCS_SETUP_INFO:         /*startinfo sent */
      if (ocr_get_first_image_strip () == NULL)
        break;                   /*disconnected */
    case OCS_READING_STRIPS:     /*read first but more to come */
      strip->x_size = -code;     /*report error */
      release_ocr();  /*send ack */
      release_mutex();
      break;
    case OCS_READ_STRIPS:        /*read all but no monitor yet */
      monitor->count = 0;        /*chars in this buffer(-1) */
      monitor->progress = 0;     /*percent complete increasing (0-100) */
                                 /*text not complete */
      monitor->more_to_come = FALSE;
      monitor->ocr_alive = TRUE; /*ocr sets to 1, hp 0 */
      monitor->err_code = -code; /*report error */
      monitor->cancel = FALSE;   /*0=continue, 1=cancel */
      release_ocr();  /*send ack */
      break;
    case OCS_RECOGNIZING:        /*OCR incomplete */
    case OCS_SENDING_TEXT:       /*sent buffer but more to come */
      monitor->err_code = -code; /*report error */
      release_ocr();  /*send ack */
  }
  ocr_internal_shutdown();  /*get ready for exit */
}


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
                         ) {
  EOCR_DESC *desc;               /*ocr engine info */
  int index;                     /*char index */
  inT32 font_index;              /*which font */

  if (ocr_state != OCS_SETUP_SHM) {
    ocr_error(OCR_ERR_BAD_STATE);
    return OCR_API_BAD_STATE;    /*incorrect state */
  }

                                 /*turn to right type */
  desc = (EOCR_DESC *) shm.shm_mem;
  if (font_count >
    (inT32) ((shm.shm_size - sizeof (EOCR_DESC)) / sizeof (EFONT_DESC)))
    return OCR_API_NO_MEM;       /*insufficient space */
  font_index = font_count++;     /*add a font */
                                 /*setup structure */
  desc->fonts[font_index].language = language;
                                 /*setup structure */
  desc->fonts[font_index].font_family = font_family;
                                 /*setup structure */
  desc->fonts[font_index].char_set = char_set;
                                 /*setup structure */
  desc->fonts[font_index].pitch = pitch;
  if (name != NULL) {
    for (index = 0; index < MAX_FONT_NAME && name[index] != 0; index++)
      desc->fonts[font_index].name[index] = name[index];
  }
  else
    index = 0;
  desc->fonts[font_index].name[index] = 0;
  return OKAY;
}


/**********************************************************************
 * ocr_setup_startinfo
 *
 * Setup the info on the OCR engine. Uses 16 bit chars to name the
 * engine.
 **********************************************************************/

inT16 ocr_setup_startinfo(                       /*put info into shm */
                          inT32 protocol,        /*interface version */
                          uinT16 language,       /*default language */
                          const uinT16 *name,    /*name of engine */
                          const uinT16 *version  /*version of engine */
                         ) {
  EOCR_DESC *desc;               /*ocr engine info */
  int index;                     /*char index */
  inT16 result;                  /*from open */

  if (ocr_state != OCS_SETUP_SHM || font_count < 1) {
    ocr_error(OCR_ERR_BAD_STATE);
    return OCR_API_BAD_STATE;    /*incorrect state */
  }

                                 /*turn to right type */
  desc = (EOCR_DESC *) shm.shm_mem;
  desc->protocol = protocol;     /*setup structure */
  desc->font_count = font_count;
  desc->language = language;
  for (index = 0; index < MAX_OCR_NAME && name[index] != 0; index++)
    desc->name[index] = name[index];
  desc->name[index] = 0;
  for (index = 0; index < MAX_OCR_VERSION && version[index] != 0; index++)
    desc->version[index] = version[index];
  desc->version[index] = 0;

  result = release_ocr ();
  if (result != OKAY)
    return result;
  ocr_state = OCS_SETUP_INFO;    /*record state */
  return OKAY;
}


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
                              ) {
  EOCR_DESC *desc;               /*ocr engine info */
  int index;                     /*char index */
  inT16 result;                  /*from open */

  if (ocr_state != OCS_SETUP_SHM || font_count < 1) {
    ocr_error(OCR_ERR_BAD_STATE);
    return OCR_API_BAD_STATE;    /*incorrect state */
  }

                                 /*turn to right type */
  desc = (EOCR_DESC *) shm.shm_mem;
  desc->protocol = protocol;     /*setup structure */
  desc->font_count = font_count;
  desc->language = language;
  for (index = 0; index < MAX_OCR_NAME && name[index] != 0; index++)
    desc->name[index] = name[index];
  desc->name[index] = 0;
  for (index = 0; index < MAX_OCR_VERSION && version[index] != 0; index++)
    desc->version[index] = version[index];
  desc->version[index] = 0;

  result = release_ocr ();
  if (result != OKAY)
    return result;
  ocr_state = OCS_SETUP_INFO;    /*record state */
  return OKAY;
}


/**********************************************************************
 * ocr_get_first_image_strip
 *
 * Wait for the master to send the first image strip and return a
 * pointer to it. The result is NULL if it is time to exit.
 **********************************************************************/

ESTRIP_DESC *ocr_get_first_image_strip() {  /*get image strip */
  ESTRIP_DESC *strip;            /*strip info */
  inT16 result;                  /*of wait/release */

  if (ocr_state != OCS_SETUP_INFO) {
    tprintf ("Bad state reading strip");
    ocr_error(OCR_ERR_BAD_STATE);
    return NULL;                 /*incorrect state */
  }

                                 /*strip info */
  strip = (ESTRIP_DESC *) shm.shm_mem;
  lines_read = 0;

  result = wait_for_mutex ();
  if (result != OKAY) {
    tprintf ("Mutax wait failed reading strip");
    return NULL;                 /*HP dead */
  }
  result = release_mutex ();
  if (result != OKAY) {
    tprintf ("Mutax release failed reading strip");
    return NULL;                 /*HP dead */
  }
  result = wait_for_hp (READIM_TIMEOUT);
  if (result != OKAY) {
    tprintf ("Wait for HP failed reading strip");
    return NULL;                 /*HP dead */
  }
  lines_read = strip->strip_size;/*lines read so far */
  if (lines_read < strip->y_size)
                                 /*record state */
      ocr_state = OCS_READING_STRIPS;
  else
    ocr_state = OCS_READ_STRIPS;
  if (strip->x_size == 0 || strip->y_size == 0)
    return NULL;                 /*end of job */

  return strip;
}


/**********************************************************************
 * ocr_get_next_image_strip
 *
 * Wait for the master to send the next image strip and return a
 * pointer to it. The result is NULL if it is time to exit.
 **********************************************************************/

ESTRIP_DESC *ocr_get_next_image_strip() {  /*get image strip */
  ESTRIP_DESC *strip;            /*strip info */
  inT16 result;                  /*of wait/release */

  if (ocr_state != OCS_READING_STRIPS) {
    ocr_error(OCR_ERR_BAD_STATE);
    return NULL;                 /*incorrect state */
  }

                                 /*strip info */
  strip = (ESTRIP_DESC *) shm.shm_mem;
  result = release_ocr ();
  if (result != OKAY)
    return NULL;                 /*HP dead */
  result = wait_for_hp (READIM_TIMEOUT);
  if (result != OKAY)
    return NULL;                 /*HP dead */
                                 /*lines read so far */
  lines_read += strip->strip_size;
  if (lines_read < strip->y_size)
                                 /*record state */
      ocr_state = OCS_READING_STRIPS;
  else
    ocr_state = OCS_READ_STRIPS;

  return strip;
}


/**********************************************************************
 * ocr_setup_monitor
 *
 * Setup the progress monitor. Call before starting the recognize task.
 **********************************************************************/

ETEXT_DESC *ocr_setup_monitor() {  /*setup monitor */
  ETEXT_DESC *monitor;           /*progress monitor */

                                 /*text info */
  monitor = (ETEXT_DESC *) shm.shm_mem;
  monitor->count = 0;            /*chars in this buffer(-1) */
  monitor->progress = 0;         /*percent complete increasing (0-100) */
  monitor->more_to_come = TRUE;  /*text not complete */
  monitor->ocr_alive = TRUE;     /*ocr sets to 1, hp 0 */
  monitor->err_code = 0;         /*used by ocr_error */
  monitor->cancel = FALSE;       /*0=continue, 1=cancel */


//by jetsoft
//the sem functions are old and were meant for an hp product
 // if (release_ocr () != OKAY)
   // return NULL;                 /*release failed */

  ocr_state = OCS_RECOGNIZING;   /*record state */
  return monitor;
}


/**********************************************************************
 * ocr_char_space
 *
 * Return the number of chars that can be fitted into the buffer.
 **********************************************************************/

inT32 ocr_char_space() {  /*put char into shm */
  ETEXT_DESC *buf;               /*text buffer */
  int result;

                                 /*progress info */
  buf = (ETEXT_DESC *) shm.shm_mem;
  if (buf == NULL)
    return 0;

  result =
    (shm.shm_size - sizeof (ETEXT_DESC)) / sizeof (EANYCODE_CHAR) -
    buf->count + 1;

  //      while (buf->hp_alive==-1)
  //              Sleep(50);                                                                              /*wait for HP*/

  return result;
}


/**********************************************************************
 * ocr_append_char
 *
 * Add a character to the output. Returns OKAY if successful, OCR_API_NO_MEM
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
                     ) {
  ETEXT_DESC *buf;               /*text buffer */
  int index;                     /*char index */
  inT16 result;                  /*of callback */

  if (ocr_state != OCS_RECOGNIZING && ocr_state != OCS_SENDING_TEXT) {
    ocr_error(OCR_ERR_BAD_STATE);
    return OCR_API_BAD_STATE;    /*incorrect state */
  }

  if (char_code == ' ' || char_code == '\n' || char_code == '\r'
    || char_code == '\t')
    return OCR_API_BAD_CHAR;     /*illegal char */

                                 /*progress info */
  buf = (ETEXT_DESC *) shm.shm_mem;

  result =
    (shm.shm_size - sizeof (ETEXT_DESC)) / sizeof (EANYCODE_CHAR) -
    buf->count;
  if (result < 1)
    return OCR_API_NO_MEM;       /*insufficient room */

  index = buf->count++;          /*count of chars */
                                 /*setup structure */
  buf->text[index].char_code = char_code;
  buf->text[index].left = left;  /*setup structure */
  buf->text[index].right = right;/*setup structure */
  buf->text[index].top = top;    /*setup structure */
                                 /*setup structure */
  buf->text[index].bottom = bottom;
                                 /*setup structure */
  buf->text[index].font_index = font_index;
                                 /*setup structure */
  buf->text[index].confidence = confidence;
                                 /*setup structure */
  buf->text[index].point_size = point_size;
                                 /*setup structure */
  buf->text[index].blanks = blanks;
  if (nl_type == OCR_NL_NONE) {
    if (text_dir == OCR_CDIR_TOP_BOTTOM || text_dir == OCR_CDIR_BOTTOM_TOP)
      buf->text[index].formatting = (text_dir << 5) | 128;
    /*setup structure */
    else
                                 /*setup structure */
      buf->text[index].formatting = text_dir << 5;
  }
  else {
    buf->text[index].formatting = (nl_type << 6) | (line_dir << 5);
    /*setup structure */
  }
  buf->text[index].formatting |= enhancement & (~EUC_FORMAT_MASK);
  return OKAY;
}


/**********************************************************************
 * ocr_send_text
 *
 * Send the text to the host and wait for the ack.
 * Use this function after a sequence of ocr_append_char calls to
 * actually sent the text to the master process.
 * Set more to come TRUE if there is more text in this page, FALSE
 * if the OCR engine is now ready to receive another image.
 **********************************************************************/

inT16 ocr_send_text(                    /*send shm */
                    BOOL8 more_to_come  /*any text left */
                   ) {
  ETEXT_DESC *buf;               /*text buffer */

  if (ocr_state != OCS_RECOGNIZING && ocr_state != OCS_SENDING_TEXT) {
    ocr_error(OCR_ERR_BAD_STATE);
    return OCR_API_BAD_STATE;    /*incorrect state */
  }

                                 /*progress info */
  buf = (ETEXT_DESC *) shm.shm_mem;

                                 /*setup structure */
  buf->more_to_come = more_to_come;
  if (more_to_come) {
    if ((buf->text[buf->count - 1].formatting >> 6) != OCR_NL_NEWLINE
    && (buf->text[buf->count - 1].formatting >> 6) != OCR_NL_NEWPARA) {
                                 /*force line end */
      buf->text[buf->count - 1].formatting &= 63;
      buf->text[buf->count - 1].formatting |= OCR_NL_NEWLINE << 6;
    }
  }
  else {
    if (buf->count < 1)
      ocr_append_char ('~', -1, -1, -1, -1, 0, 100, 10, 0,
        0, OCR_CDIR_RIGHT_LEFT, OCR_LDIR_DOWN_RIGHT,
        OCR_NL_NEWPARA);
    /*dummy character */
    else if ((buf->text[buf->count - 1].formatting >> 6) != OCR_NL_NEWPARA) {
                                 /*force para end */
      buf->text[buf->count - 1].formatting &= 63;
      buf->text[buf->count - 1].formatting |= OCR_NL_NEWPARA << 6;
    }
  }

  if (release_ocr () != OKAY)
    return HPERR;                /*release failed */
  if (wait_for_hp (READTEXT_TIMEOUT) != OKAY)
    return HPERR;
  if (more_to_come) {
    buf->count = 0;              /*setup structure */
    ocr_state = OCS_SENDING_TEXT;/*record state */
  }
  else
    ocr_state = OCS_SETUP_INFO;  /*record state */
  return OKAY;
}


/**********************************************************************
 * ocr_shutdown
 *
 * Closedown communications with the HP side and free up handles.
 **********************************************************************/

inT16 ocr_shutdown() {  /*closedown */
  #ifdef __MAC__
  shm.OCRProcess.lowLongOfPSN = kNoProcess;
  shm.OCRProcess.highLongOfPSN = 0;
  #endif
  ocr_error(OCR_ERR_CLEAN_EXIT);  /*signal exit */

  return OKAY;
}


/**********************************************************************
 * ocr_internal_shutdown
 *
 * Free up handles or whatever to clean up without attempting to communicate.
 **********************************************************************/

inT16 ocr_internal_shutdown() {  /*closedown */
  ocr_state = OCS_DEAD;          /*record state */
  #ifdef __MSW32__
  if (shm.shm_mem != NULL) {
    UnmapViewOfFile (shm.shm_mem);
    CloseHandle (shm.shm_hand);  /*no longer used */
    CloseHandle (shm.mutex);     /*release handles */
    CloseHandle (shm.ocr_sem);
    CloseHandle (shm.hp_sem);
    shm.shm_mem = NULL;
  }
  #elif defined (__MAC__)
  shm.OCRProcess.lowLongOfPSN = kNoProcess;
  shm.OCRProcess.highLongOfPSN = 0;
  #endif
  return OKAY;
}


/**********************************************************************
 * wait_for_mutex
 *
 * Wait for the HP side to release its mutex.
 * The return value is HPERR if the HP side has terminated.
 **********************************************************************/

inT16 wait_for_mutex() {  /*wait for HP to be ready */
  inT16 result = HPERR;          /*return code */
  #if defined (__MSW32__) || defined (__MAC__)
  result = WaitForSingleObject (shm.mutex, (unsigned long) -1)
  /*wait for thread to move */
                                 /*bad if timeout */
    == WAIT_OBJECT_0 ? OKAY : HPERR;
  #endif
  if (result != OKAY)
    ocr_internal_shutdown();
  return result;
}


/**********************************************************************
 * wait_for_hp
 *
 * Wait for the HP side to release its semaphore.
 * The return value is HPERR if the timeout (in seconds) elapsed.
 **********************************************************************/

inT16 wait_for_hp(               /*wait for semaphore */
                  inT32 timeout  /*in seconds */
                 ) {
  inT16 result = HPERR;          /*return code */
  #if defined (__MSW32__) || defined (__MAC__)
                                 /*wait for thread to move */
  result = WaitForSingleObject (shm.hp_sem, timeout * TICKS)
                                 /*bad if timeout */
    == WAIT_OBJECT_0 ? OKAY : HPERR;
  #endif
  if (result != OKAY)
    ocr_internal_shutdown();
  return result;
}


/**********************************************************************
 * release_mutex
 *
 * Release the HP mutex.
 * The return value is OKAY if the call succeeds.
 **********************************************************************/

inT16 release_mutex() {  /*release mutex */
  inT16 result = HPERR;          /*return code */
  #ifdef __MSW32__
                                 /*release it */
  result = ReleaseMutex (shm.mutex) ? OKAY : HPERR;
  #elif defined (__MAC__)
                                 /*release it */
  result = ReleaseSemaphore (shm.mutex) ? OKAY : HPERR;
  #endif
  if (result != OKAY)
    ocr_internal_shutdown();
  return result;
}


/**********************************************************************
 * release_ocr
 *
 * Release the OCR semaphore.
 * The return value is OKAY if the call succeeds.
 **********************************************************************/

inT16 release_ocr() {  /*release semaphore */
  inT32 timeout;                 //time allowed

  timeout = RELEASE_TIMEOUT * TICKS;
  #ifdef __MSW32__

//jetsoft
// this stuff is old and no longer applies

  return OKAY;
//

  BOOL result = 0;               //of release
  do {
                                 //release it
    result = ReleaseSemaphore (shm.ocr_sem, 1, NULL);
    if (result == FALSE) {
      timeout -= 50;
      Sleep (50);
    }
  }
  while (result == FALSE && timeout > 0);
  if (!result)
    ocr_internal_shutdown();
  return OKAY;
  #elif defined (__MAC__)
  inT16 result = HPERR;          /*return code */
                                 /*release it */
  result = ReleaseSemaphore (shm.ocr_sem) ? OKAY : HPERR;

  if (result != OKAY)
    ocr_internal_shutdown();
  return result;
  #elif defined (__UNIX__)
  return 0;
  #endif
}
