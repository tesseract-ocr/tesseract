/*====================================================================*
 -  Copyright (C) 2010 Leptonica.  All rights reserved.
 -  This software is distributed in the hope that it will be
 -  useful, but with NO WARRANTY OF ANY KIND.
 -  No author or distributor accepts responsibility to anyone for the
 -  consequences of using this software, or for whether it serves any
 -  particular purpose or works at all, unless he or she says so in
 -  writing.  Everyone is granted permission to copy, modify and
 -  redistribute this source code, for commercial or non-commercial
 -  purposes, with the following restrictions: (1) the origin of this
 -  source code must not be misrepresented; (2) modified versions must
 -  be plainly marked as such; and (3) this notice may not be removed
 -  or altered from any source or modified source distribution.
 *====================================================================*/

#ifdef _WIN32
#ifndef  LEPTONICA_LEPTWIN_H
#define  LEPTONICA_LEPTWIN_H

#include "allheaders.h"
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

LEPT_DLL extern HBITMAP pixGetWindowsHBITMAP( PIX *pixs );

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* LEPTONICA_LEPTWIN_H */
#endif /* _WIN32 */
