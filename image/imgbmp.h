/**********************************************************************
 * File:			imgbmp.c  (Formerly lz.c)
 * Description:	bmp image reader/writer.
 * Author:		Ray Smith
 * Created:		Tue Jan 06 20:15:52 GMT 1998
 *
 * (C) Copyright 1998, Ray Smith.
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

#ifndef           IMGBMP_H
#define           IMGBMP_H

#include          "host.h"

inT8 open_bmp_image(               //read header
                    int fd,        //file to read
                    inT32 *xsize,  //size of image
                    inT32 *ysize,
                    inT8 *bpp,     //bits per pixel
                    inT8 *photo,
                    inT32 *res     //resolution
                   );
inT8 read_bmp_image(                //read header
                    int fd,         //file to read
                    uinT8 *pixels,  //pixels of image
                    inT32 xsize,    //size of image
                    inT32 ysize,
                    inT8 bpp,       //bits per pixel
                    inT32           //bytes per line
                   );
inT8 write_bmp_image(                //write whole image
                     int fd,         //file to write on
                     uinT8 *pixels,  //image pixels
                     inT32 xsize,    //size of image
                     inT32 ysize,
                     inT8 bpp,       //bits per pixel
                     inT8,
                     inT32           //resolution
                    );
#endif
