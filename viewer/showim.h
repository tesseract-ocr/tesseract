/**********************************************************************
 * File:        showimg.c  (Formerly showim.c)
 * Description: Interface to sbdaemon for displaying images.
 * Author:      Ray Smith
 * Created:     Mon Jun 11 16:20:34 BST 1990
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

#ifndef           SHOWIM_H
#define           SHOWIM_H

#include          "grphics.h"
#include          "img.h"

#define show_sub_image(im,xstart,ystart,xext,yext,win,xpos,ypos) (*show_func)(im,xstart,ystart,xext,yext,win,xpos,ypos)

extern void (*show_func) (IMAGE *, INT32, INT32, INT32, INT32, WINDOW, INT32,
INT32);

DLLSYM void def_show_sub_image(                //show this image
                               IMAGE *source,  //image to show
                               INT32 xstart,   //start coords
                               INT32 ystart,
                               INT32 xext,     //extent to show
                               INT32 yext,
                               WINDOW win,     //window to draw in
                               INT32 xpos,     //position to show at
                               INT32 ypos      //y position
                              );
#endif
