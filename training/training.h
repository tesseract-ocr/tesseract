/*
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#ifndef TRAINING_H
#define TRAINING_H

#include "host.h"
#include "callcpp.h"

typedef enum
{
baseline, character
}
NORM_METHOD;

typedef struct
{
FLOAT32						x,y;
} FPOINT;
typedef FPOINT FVECTOR;


#define INTEL				0x4949
#define MOTO				0x4d4d

#define PICO_FEATURE_LENGTH	(0.05)
#define GetPicoFeatureLength()	(PICO_FEATURE_LENGTH)

extern int LearningDebugLevel;
extern int NormMethod;

void						cprintf(					//Trace printf
const char					*format,...					//special message
);

char						*c_alloc_string(			//allocate string
inT32						count						//no of chars required
);

void						c_free_string(				//free a string
char						*string						//string to free
);

void						*c_alloc_mem_p(				//allocate permanent space
inT32						count						//block size to allocate
);

void						*c_alloc_mem(				//get some memory
inT32						count						//no of bytes to get
);

void						c_free_mem(					//free mem from alloc_mem
void						*oldchunk					//chunk to free
);

void						c_check_mem(				//check consistency
char						*string,					//context message
inT8						level						//level of check
);

void*						c_alloc_struct(				//allocate memory
inT32						count,						//no of chars required
const char*					name						//class name
);

void						c_free_struct(				//free a structure
void*						deadstruct,					//structure to free
inT32						count,						//no of bytes
const char*					name						//class name
);

void						c_make_current(				/*move pen*/
void*						win
);

void						reverse32(
void*						ptr
);

void						reverse16(
void*						ptr
);

ScrollView*					c_create_window(			/*create a window*/
const char					*name,						/*name/title of window*/
inT16						xpos,						/*coords of window*/
inT16						ypos,						/*coords of window*/
inT16						xsize,						/*size of window*/
inT16						ysize,						/*size of window*/
double						xmin,						/*scrolling limits*/
double						xmax,						/*to stop users*/
double						ymin,						/*getting lost in*/
double						ymax						/*empty space*/
);

void						c_line_color_index(			/*set color*/
void*						win,
C_COL						index
);

void						c_move(						/*move pen*/
void*						win,
double						x,
double						y
);

void						c_draw(						/*move pen*/
void*						win,
double						x,
double						y
);

void						c_clear_window(				/*move pen*/
void*						win
);

char						window_wait(				/*move pen*/
void*						win
);

#endif
