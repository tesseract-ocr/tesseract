/**********************************************************************
 * File:        imgtiff.h  (Formerly tiff.h)
 * Description: Header file for tiff format image reader/writer.
 * Author:      Ray Smith
 * Created:     Mon Jun 11 15:19:41 BST 1990
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

#ifndef           IMGTIFF_H
#define           IMGTIFF_H

#include          "host.h"
#include          "bitstrm.h"

inT8 open_tif_image(               //read header
                    int fd,        //file to read
                    inT32 *xsize,  //size of image
                    inT32 *ysize,
                    inT8 *bpp,     //bits per pixel
                    inT8 *photo,   //interpretation
                    inT32 *res     //resolution
                   );
inT8 read_tif_image(                //read whole image
                    int fd,         //file to read
                    uinT8 *pixels,  //pixels of image
                    inT32 xsize,    //size of image
                    inT32 ysize,
                    inT8 bpp,       //bits per pixel
                    inT32           //bytes per line
                   );
inT32 read_eol(                    //read end of line
               R_BITSTREAM *bits,  //bitstream to read
               uinT16 &code        //current code
              );
inT8 write_moto_tif(                //write whole image
                    int fd,         //file to write on
                    uinT8 *pixels,  //image pixels
                    inT32 xsize,    //size of image
                    inT32 ysize,
                    inT8 bpp,       //bits per pixel
                    inT8 photo,
                    inT32 res       //resolution
                   );
inT8 write_intel_tif(                //write whole image
                     int fd,         //file to write on
                     uinT8 *pixels,  //image pixels
                     inT32 xsize,    //size of image
                     inT32 ysize,
                     inT8 bpp,       //bits per pixel
                     inT8 photo,
                     inT32 res       //resolution
                    );
inT8 write_inverse_tif(                //write whole image
                       int fd,         //file to write on
                       uinT8 *pixels,  //image pixels
                       inT32 xsize,    //size of image
                       inT32 ysize,
                       inT8 bpp,       //bits per pixel
                       inT8 photo,
                       inT32 res       //resolution
                      );
inT8 write_tif_image(                //write whole image
                     int fd,         //file to write on
                     uinT8 *pixels,  //image pixels
                     inT32 xsize,    //size of image
                     inT32 ysize,
                     inT8 bpp,       //bits per pixel
                     inT32 res,      //resolution
                     inT16 type,     //format type
                     inT16 photo     //metric interp
                    );
#endif
