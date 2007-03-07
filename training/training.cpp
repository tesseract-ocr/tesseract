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

#include "training.h"
#include "debug.h"
#include "memry.h"
#include "grphics.h"
#include "evnts.h"

make_int_var    (LearningDebugLevel, 0, MakeLearningDebugLevel,
		18, 5, SetLearningDebugLevel,
		"Learning Debug Level: ");

make_int_var   (NormMethod, character, MakeNormMethod,
					15, 10, SetNormMethod,   "Normalization Method   ...")

char *demodir;                   /*demo home directory */


void						cprintf(					//Trace printf
const char					*format,...					//special message
)
{
}

char						*c_alloc_string(			//allocate string
INT32						count						//no of chars required
)
{
	return alloc_string(count);
}

void						c_free_string(				//free a string
char						*string						//string to free
)
{
	free_string(string);
}

void						*c_alloc_mem_p(				//allocate permanent space
INT32						count						//block size to allocate
)
{
	return alloc_mem_p(count);
}
void						*c_alloc_mem(				//get some memory
INT32						count						//no of bytes to get
)
{
	return alloc_mem(count);
}
void						c_free_mem(					//free mem from alloc_mem
void						*oldchunk					//chunk to free
)
{
	free_mem(oldchunk);
}
void						c_check_mem(				//check consistency
const char			*string,					//context message
INT8						level						//level of check
)
{
	check_mem(string,level);
}

void*						c_alloc_struct(				//allocate memory
INT32						count,						//no of chars required
const char*					name						//class name
)
{
	return alloc_struct(count,name);
}
void						c_free_struct(				//free a structure
void*						deadstruct,					//structure to free
INT32						count,						//no of bytes
const char*					name						//class name
)
{
	free_struct(deadstruct,count,name);
}

void						c_make_current(				/*move pen*/
void*						win
)
{
	WINDOW					window=(WINDOW)win;

	window->Make_picture_current();
}

void						reverse32(
void*						ptr
)
{
	char					tmp;
	char*					cptr=(char*)ptr;

	tmp=*cptr;
	*cptr=*(cptr+3);
	*(cptr+3)=tmp;
	tmp=*(cptr+1);
	*(cptr+1)=*(cptr+2);
	*(cptr+2)=tmp;
}

void						reverse16(
void*						ptr
)
{
	char					tmp;
	char*					cptr=(char*)ptr;

	tmp=*cptr;
	*cptr=*(cptr+1);
	*(cptr+1)=tmp;
}





void*						c_create_window(			/*create a window*/
const char					*name,						/*name/title of window*/
INT16						xpos,						/*coords of window*/
INT16						ypos,						/*coords of window*/
INT16						xsize,						/*size of window*/
INT16						ysize,						/*size of window*/
double						xmin,						/*scrolling limits*/
double						xmax,						/*to stop users*/
double						ymin,						/*getting lost in*/
double						ymax						/*empty space*/
)
{
	return create_window(name,SCROLLINGWIN,xpos,ypos,xsize,ysize,
		xmin,xmax,ymin,ymax,TRUE,FALSE,FALSE,TRUE);
}
void						c_line_color_index(			/*set color*/
void*						win,
C_COL						index
)
{
	WINDOW					window=(WINDOW)win;

//	ASSERT_HOST(index>=0 && index<=48);
	if (index<0 || index>48)
		index=(C_COL)1;
	window->Line_color_index((COLOUR)index);
}
void						c_move(						/*move pen*/
void*						win,
double						x,
double						y
)
{
	WINDOW					window=(WINDOW)win;

	window->Move2d(x,y);
}
void						c_draw(						/*move pen*/
void*						win,
double						x,
double						y
)
{
	WINDOW					window=(WINDOW)win;

	window->Draw2d(x,y);
}
void						c_clear_window(				/*move pen*/
void*						win
)
{
	WINDOW					window=(WINDOW)win;

	window->Clear_view_surface();
}
char						window_wait(				/*move pen*/
void*						win
)
{
	WINDOW					window=(WINDOW)win;
	GRAPHICS_EVENT			event;

	await_event(window,TRUE,ANY_EVENT,&event);
	if (event.type==KEYPRESS_EVENT)
		return event.key;
	else
		return '\0';
}
