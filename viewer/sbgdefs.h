/* STARBASE_ID:sb.c.h   286.1   07/06/89   22:09:56 */

/*    (c) Copyright Hewlett-Packard Company, 1985.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
    All rights are reserved.  Copying or other
    reproduction of this program except for archival
    purposes is prohibited without the prior
    written consent of Hewlett-Packard Company.

            RESTRICTED RIGHTS LEGEND

    Use, duplication, or disclosure by the U.S. Government
    is subject to restrictions as set forth in
    subdivision (b) (3) (ii) of the Rights in Technical
    Data and Computer Software clause at
    52.227-7013.

    HEWLETT-PACKARD COMPANY
    Fort Collins, Colorado                              */

/************************************************************************
 *
 *  definitions and types to be included with STARBASE application
 *  programs
 *
 ***********************************************************************/

#ifndef _STARBASE_INCLUDED       /* allow multiple inclusions */
#define _STARBASE_INCLUDED

/*  kinds of graphics devices  */
#define OUTDEV        0
#define INDEV       1
#define OUTINDEV      2
#define OUTMETA       3
#define INMETA        4

/*  clear control parameters  */
#define CLEAR_VDC_EXTENT    1
#define CLEAR_VIEWPORT      17
#define CLEAR_CLIP_RECTANGLE  33
#define CLEAR_DISPLAY_SURFACE 65
#define CLEAR_ALL_BANKS     128
#define CLEAR_ZBUFFER     256

/*  clip control parameters  */
#define CLIP_TO_RECT    1
#define CLIP_TO_VDC     2
#define CLIP_OFF      3
#define CLIP_TO_VIEWPORT  4

/*  gopen open_mode parameter masks  */
#define SPOOLED    0x01          /* if this bit is on, output is spooled            */
//#define INIT     0x02                                                                 /* if this bit is on, device initialization occurs */
#define THREE_D    0x04          /* if this bit is on, all transformations are 3D   */
#define RESET_DEVICE 0x08        /* if this bit is on, hard reset including p1 & p2 */
#define MODEL_XFORM  0x10        /* if this bit is on, matrix stack in modeling mode */
#define INT_XFORM    0x20        /* if this bit is on, matrix stack in modeling mode */
#define FLOAT_XFORM  0x40        /* if this bit is on, matrix stack in modeling mode */

/* color map modes set with shade_mode */
#define CMAP_NORMAL     0
#define CMAP_MONOTONIC    1
#define CMAP_FULL     4

/* double_buffer mode used to draw into the same buffer that is displayed */
#define DFRONT 4
/* double_buffer mode used to not clear buffer when switched */
#define SUPPRESS_CLEAR 8

/* light source types set with light_source */
#define DIRECTIONAL   0
#define POSITIONAL    1
#define ATTEN_LIGHT   2
#define SPOT_LIGHT    4
#define CONE_LIGHT    8

/* vertex orders set with vertex_format */
#define CLOCKWISE     0x0000
#define COUNTER_CLOCKWISE 0x0001
#define UNIT_NORMALS      0x0200

/* set p1 p2 units */
#define FRACTIONAL    0
#define METRIC      1

/* mapping modes */
#define ISOTROPIC     0
#define DISTORT     1

/* Starbase linetypes */
#define SOLID         0
#define DASH        1
#define DOT         2
#define DASH_DOT      3
#define DASH_DOT_DOT    4
#define LONG_DASH       5
#define CENTER_DASH     6
#define CENTER_DASH_DASH  7

/* wide endpoint types */
#define SQUARE        0
#define ROUNDED     1

/* depth cue models */
#define DC_MIN    2
#define DC_COLOR  4

/* distance modes for line_width, hatch_spacing */
#define VDC_UNITS 0
#define WC_UNITS  1
#define MC_UNITS  WC_UNITS
#define DC_UNITS  2

/* polygon interior styles */
#define INT_HOLLOW    0
#define INT_SOLID   1
#define INT_PATTERN   2
#define INT_HATCH   3
#define INT_OUTLINE   4
#define INT_POINT     5

/* Matrix concatenation types */
#define PRE   0
#define POST  1
#define REPLACE 0
#define PUSH  1

/* Viewing matrix change types */
#define REPLACE_VW    0
#define PRE_CONCAT_VW 1
#define POST_CONCAT_VW  2

/* character switching modes */
#define ISO_7BIT  0
#define ISO_8BIT  1
#define HP_8BIT   2

/* text precision types */
#define STRING_TEXT   0
#define CHARACTER_TEXT  1
#define STROKE_TEXT   2

/* text transformation types */
#define VDC_TEXT        0
#define WORLD_COORDINATE_TEXT 1
#define TOS_TEXT        2
#define ANNOTATION_TEXT     3

/*  text alignment enumerated types */
#define TA_LEFT           0
#define SB_TA_CENTER         1
#define TA_RIGHT          2
#define TA_CONTINUOUS_HORIZONTAL  3
#define TA_NORMAL_HORIZONTAL    4

#define TA_TOP          0
#define TA_CAP          1
#define TA_HALF         2
#define TA_BASE         3
#define SB_TA_BOTTOM       4
#define TA_CONTINUOUS_VERTICAL  5
#define TA_NORMAL_VERTICAL    6

/*  character path and line path enumerated types  */
#define PATH_RIGHT      0
#define PATH_LEFT     1
#define PATH_UP       2
#define PATH_DOWN     3

/*  input device class enumerated types  */
#define ALL     0
#define LOCATOR   1
#define CHOICE    4

/*  event queue states */
#define EMPTY_NO_OVERFLOW   0
#define EMPTY_OVERFLOW      1
#define NOT_EMPTY_NO_OVERFLOW 2
#define NOT_EMPTY_OVERFLOW    3

/*  event message link enumerated types */
#define SIMULTANEOUS_EVENT_FOLLOWS  0
#define SINGLE_EVENT        1

/* gerr printing flags */
#define NO_ERROR_PRINTING 0
#define PRINT_ERRORS    1
#define PRINT_WARNINGS    2

/* arc close_types  */
#define NO_CHORD  0
#define PIE     1
#define CHORD   2

/* spline orders and rationalities  */
#define NONRATIONAL 0
//#define RATIONAL  1
#define LINEAR    2
#define QUADRATIC 3
#define CUBIC   4
#define QUARTIC   5
#define QUINTIC   6
#define DC_VALUES 0
#define VDC_VALUES  2
#define STEP_SIZE 3

/* transform_point modes */
#define MC_TO_WC    0
#define MC_TO_WORLD   0
#define MC_TO_VDC   1
#define WC_TO_VDC   2
#define WORLD_TO_VDC  2
#define WC_TO_MC    3
#define WORLD_TO_MC   3
#define VDC_TO_MC     4
#define VDC_TO_WC   5
#define VDC_TO_WORLD  5
#define INTVDC_TO_DC  6
#define DC_TO_INTVDC  7

/* view_camera projection types */
#define CAM_PERSPECTIVE 0.0
#define CAM_PARALLEL 1.0

/* plane printing modes */
#define ALL_PLANES    -1
#define PIXEL_MAJOR   -1
#define PLANE_MAJOR   -2

/* hatch types */
#define PARALLEL_HATCH  0
#define CROSSHATCH    1

/* highlight attribute types */
#define HL_COLOR  1
#define HL_STYLE  2

/* Hardware cursor control types */
#define REQUEST_HW_CURSOR 1
#define REQUEST_SW_CURSOR 2
#define FORCE_HW_CURSOR 3
#define REQUEST_HW_ECHO 1
#define REQUEST_SW_ECHO 2
#define FORCE_HW_ECHO 3

/* cgm encoding types */
#define CGM_BINARY    1
#define CGM_CHARACTER 2
#define CGM_CLEAR_TEXT  3

/* GLOBAL gescapes */
#define SWITCH_SEMAPHORE  0
#define READ_COLOR_MAP    1
#define BLINK_PLANES    2
#define BLINK_INDEX     3

/* GLOBAL raster gescapes */
#define R_GET_FRAME_BUFFER  20
#define R_LOCK_DEVICE     21
#define R_UNLOCK_DEVICE     22
#define R_GET_WINDOW_INFO   23
#define R_FULL_FRAME_BUFFER 24
#define R_ALLOC_OFFSCREEN   25
#define R_FREE_OFFSCREEN    26
#define R_BIT_MODE        27
#define R_BIT_MASK        28
#define R_DEF_FILL_PAT      29
#define R_OVERLAY_ECHO  30
#define R_OV_ECHO_COLORS  31
#define R_DEF_ECHO_TRANS  32
#define R_TRANSPARENCY_INDEX 33
#define R_LINE_TYPE     34
#define R_ECHO_FG_BG_COLORS 35
#define R_DMA_MODE 36
#define R_ECHO_MASK 37
#define R_ECHO_CONTROL 38
#define R_OFFSCREEN_ALLOC 1106
#define R_OFFSCREEN_FREE 1107

/* HPGL gescapes */
#define HPGL_WRITE_BUFFER 100
#define HPGL_SET_PEN_NUM  101
#define HPGL_SET_PEN_SPEED  102
#define HPGL_SET_PEN_WIDTH  103
#define HPGL_READ_BUFFER  104

/* HPGL2 gescapes */
#define HPGL2_SET_MEDIA_TYPE    105
#define HPGL2_LOGICAL_PEN_WIDTH 106
#define HPGL2_CUTTER_CONTROL    107
#define HPGL2_REPLOT            108
#define HPGL2_FONT_TYPEFACE     109
#define HPGL2_ADAPTIVE_LINES    110
#define HPGL2_SET_QUALITY       111
#define HPGL2_SET_CMAP_SIZE     112
#define HPGL2_FONT_WEIGHT       113
#define HPGL2_FONT_POSTURE      114

/* HP26XX gescapes */
#define HP26_PRINT_ESC    200
#define HPTERM_PRINT_ESC  200
#define HPTERM_640x400    201

/* 98700 gescapes */
#define GB_NONE 300

/* 98710 gescapes */
#define GA_NONE 400

/* 300l gescapes */
#define TC_HALF_PIXEL 500

/* HIL and keyboard gescapes */
#define ENABLE_AUTO_PROMPT  600
#define DISABLE_AUTO_PROMPT 601
#define PROMPT_ON       602
#define PROMPT_OFF      603
#define TRIGGER_ON_RELEASE  604
#define IGNORE_RELEASE    605
#define REPORT_PROXIMITY  606
#define IGNORE_PROXIMITY  607
#define ENABLE_ACKNOWLEDGE  608
#define DISABLE_ACKNOWLEDGE 609
#define SET_ACCELERATION  610    /* hil acceleration multiplier */

/* GKSM gescapes */
#define GKSM_WRITE_ITEM     700
#define GKSM_GET_ITEM_TYPE    701
#define GKSM_READ_ITEM      702
#define GKSM_SKIP_ITEM      703
#define GKSM_INQ_COLOR_NDCES  704
#define GKSM_INQ_PAT_REP    705

/* 98721 gescapes */
#define TRANSPARENCY      800
#define ZBUFFER_ALLOC     801
#define LS_OVERFLOW_CONTROL   802
#define PATTERN_FILL      803
#define ZWRITE_ENABLE     804
#define ZSTATE_SAVE       805
#define ZSTATE_RESTORE      806

/* SMDpixel and SMDpixel3 gescapes */
#define SMD_SUPPLY_MEM_BUFF   900
#define SMD_GET_MEM_REQUIRED  901
#define SMD_DEFINE_XY     902
#define SMD_DEFINE_DEPTH    903
#define SMD_ALLOCATE_MEMORY   904

/* Xn gescapes */
#define XN_INPUT_RAW    1000
/* raw mode (TRUE) returns LK201 keycodes
                                   cooked mode (FALSE) returns ASCII */
#define XN_KEY_RELEASE    1001   /* turn on/off key release events */
#define XN_BUTTON_RELEASE   1002 /* turn on/off button release events */

/* 98549/49/50/56 gescapes */
#define GR2D_MASK_ENABLE      1100
#define GR2D_MASK_RULE        1101
#define GR2D_DEF_MASK       1102
#define GR2D_FILL_PATTERN     1103
#define GR2D_OVERLAY_TRANSPARENT  1104
#define GR2D_REPLICATE        1105
#define GR2D_ALLOC_OFFSCREEN    1106
#define GR2D_FREE_OFFSCREEN     1107
#define GR2D_PLANE_MASK       1108
#define GR2D_INQ_CLIST_ADDR     1109
#define GR2D_LOAD_CLIST       1110
#define GR2D_CONVEX_POLYGONS    1112

/* hp98730/31 driver gescapes */
#define PAN_AND_ZOOM                1200
#define OVERLAY_BLEND               1201
#define IMAGE_BLEND                 1202
#define SET_BANK_CMAP               1203
#define GAMMA_CORRECTION            1204
#define INQ_GAMMA_CORRECTION        1205
#define FULL_COLOR_INDEX          1206
#define POLYGON_TRANSPARENCY        1207
#define CLIP_OVERFLOW               1208

/* hp98704/05 gescapes  */
#define SET_REPLACEMENT_RULE        1250

/* hpcgm driver gescapes */
#define CGMESC_ENCODING    1300
#define CGMESC_ESCAPE_ELT  1301
#define CGMESC_MET_NAME    1302
#define CGMESC_PIC_NAME    1303
#define CGMESC_FONT_IX     1304
#define CGMESC_MESSAGE     1305
#define CGMESC_APPL_DATA   1306
#define CGMESC_VDC_PREC    1307
#define  CGMESC_TOP_MODE    1308

typedef union
{
  int i[64];
  float f[64];
  char c[255];
} gescape_arg;

typedef struct
{
  float refx, refy, refz;
  float camx, camy, camz;
  float upx, upy, upz;
  float field_of_view;
  float front, back;
  float projection;
} camera_arg;
#endif                           /* _STARBASE_INCLUDED */
