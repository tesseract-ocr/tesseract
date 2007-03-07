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

#include          "mfcpch.h"
#include          "grphshm.h"
#include          "showim.h"

void (*show_func) (IMAGE *, INT32, INT32, INT32, INT32, WINDOW, INT32,
INT32) = def_show_sub_image;

/**********************************************************************
 * show_sub_image
 *
 * Send the given image to the daemon and insert it in the display list.
 **********************************************************************/

DLLSYM void def_show_sub_image(                //show this image
                               IMAGE *source,  //image to show
                               INT32 xstart,   //start coords
                               INT32 ystart,
                               INT32 xext,     //extent to show
                               INT32 yext,
                               WINDOW win,     //window to draw in
                               INT32 xpos,     //position to show at
                               INT32 ypos      //y position
                              ) {
  EIGHTOP *newop;                //message structure
  INT32 y;                       //y coord
  INT32 linelength;              //bytes per line
  IMAGE dummyimage;              //used for copying to
  IMAGEOP *sendline;             //transmitted line
  INT32 structsize;              //size of structure
  INT32 destbpp;                 //destination bits per pixel

  destbpp = source->get_bpp ();  //send all images unchanged

  newop = (EIGHTOP *) getshm (sizeof (EIGHTOP));
  if (newop != NULL) {
                                 //send the fd
    newop->header.fd = win->get_fd ();
    newop->type = SHOWIMAGE;     //send the operator
    newop->param.p[0].i = xext;  //send parameters
    newop->param.p[1].i = yext;
    newop->param.p[2].i = destbpp;
    newop->param.p[3].i = xpos;
    newop->param.p[4].i = ypos;

                                 //bytes required
    linelength = COMPUTE_IMAGE_XDIM (xext, destbpp);
    linelength++;                //round up to next
    linelength &= ~1;            //multiple of 2
                                 //size of structure
    structsize = (INT32) (sizeof (IMAGEOP) + linelength - 2);
    for (y = yext - 1; y >= 0; --y) {
                                 //get space
      sendline = (IMAGEOP *) getshm (structsize);
      if (sendline != NULL) {
        sendline->header.fd = win->get_fd ();
        sendline->type = SHOWLINE;
        sendline->size = structsize;
        dummyimage.capture (sendline->line, xext, 1, (INT8) destbpp);
        //ready for copy
                                 //copy to shm
        copy_sub_image (source, xstart, ystart + y, xext, 1, &dummyimage, 0, 0, FALSE);
      }
    }
  }
}
