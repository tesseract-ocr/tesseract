/**********************************************************************
 * File:        bitstrm.c  (Formerly bits.c)
 * Description: Bitstream read/write class member functions.
 * Author:					Ray Smith
 * Created:					Tue Feb 19 10:59:44 GMT 1991
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

#include          "mfcpch.h"     //precompiled headers
#ifdef __MSW32__
#include          <io.h>
#else
#include          <unistd.h>
#endif
#include          "fileerr.h"
#include          "bitstrm.h"

const UINT16
R_BITSTREAM::bitmasks[17] = {
  0, 1, 3, 7, 15, 31, 63, 127, 255,
  511, 1023, 2047, 4095, 8191, 16383, 32767, 65535
};

/**********************************************************************
 * R_BITSTREAM::open
 *
 * Establish a bitstream for reading.
 **********************************************************************/

UINT16 R_BITSTREAM::open(        //open for read
                         int fd  //file to read
                        ) {
  bitfd = fd;
  bufsize = read (fd, (char *) bitbuf, BITBUFSIZE * sizeof (UINT8));
  //fill buffer
  if (bufsize < 0) {
    READFAILED.error ("R_BITSTREAM::open", LOG, NULL);
    return 0;
  }
  bitword = bitbuf[0] | (bitbuf[1] << 8);
  bitindex = 2;
  bitbit = 16;
  return (UINT16) bitword;
}


/**********************************************************************
 * R_BITSTREAM::read_code
 *
 * Remove a code from the bitstream.
 **********************************************************************/

UINT16 R_BITSTREAM::read_code(              //take code out
                              UINT8 length  //length of code
                             ) {
  bitbit -= length;              //no of bits left
  bitword >>= length;            //remove bits
  while (bitbit < 16) {
                                 //get next byte
    bitword |= bitbuf[bitindex++] << bitbit;
    bitbit += 8;
    if (bitindex >= bufsize) {
      bufsize =
        read (bitfd, (char *) bitbuf, BITBUFSIZE * sizeof (UINT8));
      if (bufsize < 0) {
        READFAILED.error ("R_BITSTREAM::read_code", LOG, NULL);
        return 0;
      }
      bitindex = 0;              //newly filled buffer
    }
  }
  return (UINT16) bitword;
}


/**********************************************************************
 * R_BITSTREAM::masks
 *
 * Read a code from the static member.
 **********************************************************************/

UINT16 R_BITSTREAM::masks(             //take code out
                          INT32 index  //length of code
                         ) {
  return bitmasks[index];
}


/**********************************************************************
 * W_BITSTREAM::open
 *
 * Establish a bitstream for writing.
 **********************************************************************/

void W_BITSTREAM::open(        //open for write
                       int fd  //file to write
                      ) {
  bitfd = fd;
  bitindex = 0;
  bitword = 0;
  bitbit = 0;
}


/**********************************************************************
 * W_BITSTREAM::write_code
 *
 * Add a code to the bitstream.
 **********************************************************************/

INT8 W_BITSTREAM::write_code(              //take code out
                             UINT16 code,  //code to add
                             UINT8 length  //length of code
                            ) {
  if (length == 0) {
                                 //flushing
    if (bitbit > 0)
                                 //get last byte
      bitbuf[bitindex++] = (UINT8) bitword;
    if ((bitindex > 0) &&
      (write (bitfd, (char *) bitbuf, bitindex * sizeof (UINT8)) !=
    (INT32) (bitindex * sizeof (UINT8)))) {
      WRITEFAILED.error ("W_BITSTREAM::write_code", LOG, "Flushing");
      return -1;
    }
  }
  else {
    bitword |= code << bitbit;   //add new code
    bitbit += length;
    while (bitbit >= 8) {
                                 //get next byte
      bitbuf[bitindex++] = (UINT8) bitword;
      bitbit -= 8;
      bitword >>= 8;
      if (bitindex >= BITBUFSIZE) {
        if (write (bitfd, (char *) bitbuf, bitindex * sizeof (UINT8))
        != (INT32) (bitindex * sizeof (UINT8))) {
          WRITEFAILED.error ("W_BITSTREAM::write_code", LOG, NULL);
          return -1;
        }
        bitindex = 0;            //newly filled buffer
      }
    }
  }
  return 0;                      //success
}
