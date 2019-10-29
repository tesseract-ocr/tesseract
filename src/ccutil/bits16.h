/**********************************************************************
 * File:        bits16.h  (Formerly bits8.h)
 * Description: Code for 8 bit field class.
 * Author:      Phil Cheatle
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

#ifndef BITS16_H
#define BITS16_H

#include <cstdint>    // for uint8_t, ...
#include <tesseract/platform.h> // for DLLSYM

class DLLSYM BITS16 {
 public:
  uint16_t val = 0;

  BITS16() = default;
  BITS16(uint16_t init) : val(init) {}

  void turn_on_bit(       // flip specified bit
      uint8_t bit_num) {  // bit to flip 0..7
    val = static_cast<uint16_t>(val | 01 << bit_num);
  }

  void turn_off_bit(      // flip specified bit
      uint8_t bit_num) {  // bit to flip 0..7
    val = static_cast<uint16_t>(val & ~(01 << bit_num));
  }

  void set_bit(         // flip specified bit
      uint8_t bit_num,  // bit to flip 0..7
      bool value) {     // value to flip to
    if (value)
      val = static_cast<uint16_t>(val | 01 << bit_num);
    else
      val = static_cast<uint16_t>(val & ~(01 << bit_num));
  }

  bool bit(                     // access bit
      uint8_t bit_num) const {  // bit to access
    return (val >> bit_num) & 01;
  }
};

#endif
