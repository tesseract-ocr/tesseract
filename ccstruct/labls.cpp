/**********************************************************************
 * File:        labls.c  (Formerly labels.c)
 * Description: Attribute definition tables
 * Author:					Sheelagh Lloyd?
 * Created:
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "mfcpch.h"
#include          "hpdsizes.h"
#include          "labls.h"

/******************************************************************************
 * TEXT REGIONS
 *****************************************************************************/
DLLSYM inT32 tn[NUM_TEXT_ATTR] = {
  3,                             //T_HORIZONTAL
  4,                             //T_TEXT
  2,                             //T_SERIF
  2,                             //T_PROPORTIONAL
  2,                             //T_NORMAL
  2,                             //T_UPRIGHT
  2,                             //T_SOLID
  3,                             //T_BLACK
  2,                             //T_NOTUNDER
  2,                             //T_NOTDROP
};

DLLSYM char tlabel[NUM_TEXT_ATTR][4][MAXLENGTH] = { {
                                 //T_HORIZONTAL
    "Horizontal",
    "Vertical",
    "Skew",
    ""
  },
  {                              //T_TEXT
    "Text",
    "Table",
    "Form",
    "Mixed"
  },
  {                              //T_SERIF
    "Serif",
    "Sans-serif",
    "",
    ""
  },
  {                              //T_PROPORTIONAL
    "Proportional",
    "Fixed pitch",
    "",
    ""
  },
  {                              //T_NORMAL
    "Normal",
    "Bold",
    "",
    ""
  },
  {                              //T_UPRIGHT
    "Upright",
    "Italic",
    "",
    ""
  },
  {                              //T_SOLID
    "Solid",
    "Outline",
    "",
    ""
  },
  {                              //T_BLACK
    "Black",
    "White",
    "Coloured",
    ""
  },
  {                              //T_NOTUNDER
    "Not underlined",
    "Underlined",
    "",
    ""
  },
  {                              //T_NOTDROP
    "Not drop caps",
    "Drop Caps",
    "",
    ""
  }
};

DLLSYM inT32 bn[NUM_BLOCK_ATTR] = {
  4,                             //G_MONOCHROME
  2,                             //I_MONOCHROME
  2,                             //I_SMOOTH
  3,                             //R_SINGLE
  3,                             //R_BLACK
  3,                             //S_BLACK
  2                              //W_TEXT
};

DLLSYM inT32 tvar[NUM_TEXT_ATTR];
DLLSYM inT32 bvar[NUM_BLOCK_ATTR];
DLLSYM char blabel[NUM_BLOCK_ATTR][4][MAXLENGTH] = { {
                                 //G_MONOCHROME

  /****************************************************************************
   *  GRAPHICS
   ***************************************************************************/
    "Monochrome ",
    "Two colour ",
    "Spot colour",
    "Multicolour"
  },

  /****************************************************************************
   *  IMAGE
   ***************************************************************************/
  {                              //I_MONOCHROME
    "Monochrome ",
    "Colour     ",
    "",
    ""
  },
  {                              //I_SMOOTH
    "Smooth     ",
    "Grainy     ",
    "",
    ""
  },

  /****************************************************************************
   *  RULES
   ***************************************************************************/
  {                              //R_SINGLE
    "Single  ",
    "Double  ",
    "Multiple",
    ""
  },
  {                              //R_BLACK
    "Black   ",
    "White   ",
    "Coloured",
    ""
  },

  /****************************************************************************
   *  SCRIBBLE
   ***************************************************************************/
  {                              //S_BLACK
    "Black   ",
    "White   ",
    "Coloured",
    ""
  },
  /****************************************************************************
   *  WEIRD
   ***************************************************************************/
  {                              //W_TEXT
    "No text      ",
    "Contains text",
    "",
    ""
  }
};

DLLSYM char backlabel[NUM_BACKGROUNDS][MAXLENGTH] = {
  "White",                       //B_WHITE
  "Black",                       //B_BLACK
  "Coloured",                    //B_COLOURED
  "Textured",                    //B_TEXTURED
  "Patterned",                   //B_PATTERNED
  "Gradient fill",               //B_GRADIENTFILL
  "Image",                       //B_IMAGE
  "Text"                         //B_TEXT
};
