/**********************************************************************
 * File:        control.h  (Formerly control.h)
 * Description: Module-independent matcher controller.
 * Author:      Ray Smith
 * Created:     Thu Apr 23 11:09:58 BST 1992
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

/**
 * @file control.h
 * Module-independent matcher controller.
 */

#ifndef CONTROL_H
#define CONTROL_H

enum ACCEPTABLE_WERD_TYPE
{
  AC_UNACCEPTABLE,               ///< Unacceptable word
  AC_LOWER_CASE,                 ///< ALL lower case
  AC_UPPER_CASE,                 ///< ALL upper case
  AC_INITIAL_CAP,                ///< ALL but initial lc
  AC_LC_ABBREV,                  ///< a.b.c.
  AC_UC_ABBREV                   ///< A.B.C.
};

#endif
