/**********************************************************************
 * File:        bitstrm.h  (Formerly bits.h)
 * Description: R_BITSTREAM and W_BITSTREAM class definitions.
 * Author:					Ray Smith
 * Created:					Tue Feb 19 10:44:22 GMT 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#ifndef           BITSTRM_H
#define           BITSTRM_H

#include          "host.h"

#define BITBUFSIZE      8192     //bitstream buffer

class DLLSYM R_BITSTREAM
{
  private:
    int bitfd;                   //file descriptor
    inT32 bitindex;              //current byte
    uinT32 bitword;              //current word
    inT32 bitbit;                //current bit
    inT32 bufsize;               //size of buffer
    uinT8 bitbuf[BITBUFSIZE];    //bitstream buffer
                                 //for reading codes
    static const uinT16 bitmasks[17];

  public:

    R_BITSTREAM() {
    };                           //Null constructor

    uinT16 open(          //open to read
                int fd);  //file to read

    uinT16 read_code(                //read a code
                     uinT8 length);  //bits to lose
    uinT16 masks(               //read a code
                 inT32 index);  //bits to lose
};

class DLLSYM W_BITSTREAM
{
  private:
    int bitfd;                   //file descriptor
    inT32 bitindex;              //current byte
    uinT32 bitword;              //current word
    inT32 bitbit;                //current bit
    uinT8 bitbuf[BITBUFSIZE];    //bitstream buffer

  public:
    W_BITSTREAM() {
    };                           //Null constructor

    void open(          //open to write
              int fd);  //file to write

    inT8 write_code(                //write a code
                    uinT16 code,    //code to write
                    uinT8 length);  //bits to lose
};
#endif
