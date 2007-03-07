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

INT8 open_tif_image(               //read header
                    int fd,        //file to read
                    INT32 *xsize,  //size of image
                    INT32 *ysize,
                    INT8 *bpp,     //bits per pixel
                    INT8 *photo,   //interpretation
                    INT32 *res     //resolution
                   );
INT8 read_tif_image(                //read whole image
                    int fd,         //file to read
                    UINT8 *pixels,  //pixels of image
                    INT32 xsize,    //size of image
                    INT32 ysize,
                    INT8 bpp,       //bits per pixel
                    INT32           //bytes per line
                   );
INT32 read_eol(                    //read end of line
               R_BITSTREAM *bits,  //bitstream to read
               UINT16 &code        //current code
              );
INT8 write_moto_tif(                //write whole image
                    int fd,         //file to write on
                    UINT8 *pixels,  //image pixels
                    INT32 xsize,    //size of image
                    INT32 ysize,
                    INT8 bpp,       //bits per pixel
                    INT8 photo,
                    INT32 res       //resolution
                   );
INT8 write_intel_tif(                //write whole image
                     int fd,         //file to write on
                     UINT8 *pixels,  //image pixels
                     INT32 xsize,    //size of image
                     INT32 ysize,
                     INT8 bpp,       //bits per pixel
                     INT8 photo,
                     INT32 res       //resolution
                    );
INT8 write_inverse_tif(                //write whole image
                       int fd,         //file to write on
                       UINT8 *pixels,  //image pixels
                       INT32 xsize,    //size of image
                       INT32 ysize,
                       INT8 bpp,       //bits per pixel
                       INT8 photo,
                       INT32 res       //resolution
                      );
INT8 write_tif_image(                //write whole image
                     int fd,         //file to write on
                     UINT8 *pixels,  //image pixels
                     INT32 xsize,    //size of image
                     INT32 ysize,
                     INT8 bpp,       //bits per pixel
                     INT32 res,      //resolution
                     INT16 type,     //format type
                     INT16 photo     //metric interp
                    );
//INT32                                                         reverse32(                                                      //reverse 32 bit int
//UINT32                                                        value                                                                   //value to reverse
//);
//INT16                                                         reverse16(                                                      //reverse 16 bit int
//UINT16                                                        value                                                                   //value to reverse
//);
#endif
