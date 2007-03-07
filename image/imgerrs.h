/**********************************************************************
 * File:        imgerrs.h  (Formerly imgerr.h)
 * Description: Definitions of errors related to IMAGE operations.
 * Author:      Ray Smith
 * Created:     Tue Aug 14 10:10:53 BST 1990
 *
 * (C) Copyright 1990, Hewlett-Packard Ltd.
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

#ifndef           IMGERRS_H
#define           IMGERRS_H

#include          "errcode.h"

const ERRCODE BADIMAGETYPE = "Unrecognized image type";
const ERRCODE CANTREADIMAGETYPE = "Can't read this image type";
const ERRCODE CANTWRITEIMAGETYPE = "Can't write this image type";
const ERRCODE IMAGEUNDEFINED = "Attempt to operate on undefined image";
const ERRCODE BADIMAGECOORDS = "Coordinates in image out of bounds";
const ERRCODE BADIMAGESEEK = "Can't seek backwards in a buffered image!";
const ERRCODE BADIMAGESIZE = "Illegal image size";
const ERRCODE BADIMAGEFORMAT = "Illegal image format";
const ERRCODE BADBPP = "Only 1,2,4,5,6,8 bpp are supported";
const ERRCODE BADWINDOW = "Convolution window must have odd dimensions";
#endif
