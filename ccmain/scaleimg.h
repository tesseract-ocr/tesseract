/**********************************************************************
 * File:        scaleimg.h  (Formerly scaleim.h)
 * Description: Smart scaling of images.
 * Author:      Phil Cheatle
 * Created:     Wed Nov 18 16:12:03 GMT 1992
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

#ifndef           SCALEIMG_H
#define           SCALEIMG_H

void scale_image(                     //scale an image
                 IMAGE &image,        //source image
                 IMAGE &target_image  //target image
                );
void scale_image_cop_out(                      //scale an image
                         IMAGE &image,         //source image
                         IMAGE &target_image,  //target image
                         float factor,         //scale factor
                         int *hires,
                         int *lores,
                         int *oldhires,
                         int *oldlores);
#endif
