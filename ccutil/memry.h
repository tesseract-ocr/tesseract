/**********************************************************************
 * File:        memry.h  (Formerly memory.h)
 * Description: Header file for basic memory allocation/deallocation.
 * Author:      Ray Smith
 * Created:     Tue May  8 16:03:48 BST 1990
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

#ifndef           MEMRY_H
#define           MEMRY_H

#include          <stddef.h>
#include          "host.h"

#define JUSTCHECKS      0        /*just check consistency */
#define MEMCHECKS     1          /*report totals */
#define FULLMEMCHECKS   2        /*report on all blocks */

#ifdef __MSW32__
#define NEWDELETE										/*replace new & delete*/\
	void					*operator new(				/*fast allocator*/\
	size_t					size,						/*size of object*/\
	const char*				file=NULL,					/*filename*/\
	inT32					line=0)						/*line number*/\
	{\
		return alloc_struct(size);						/*simple to do*/\
	}\
\
	void					operator delete(			/*fast destructor*/\
	void					*deadstruct,				/*thing to free*/\
	size_t					size)						/*sizeof struct*/\
	{\
		free_struct(deadstruct,size);					/*free it*/\
	}\

#define NEWDELETE2(name)								/*replace new & delete*/\
	void					*operator new(				/*fast allocator*/\
	size_t					size,						/*size of object*/\
	const char*				file=NULL,					/*filename*/\
	inT32					line=0)						/*line number*/\
	{\
		return alloc_struct(size,#name);				/*simple to do*/\
	}\
\
	void					operator delete(			/*fast destructor*/\
	void					*deadstruct,				/*thing to free*/\
	size_t					size)						/*sizeof struct*/\
	{\
		free_struct(deadstruct,size,#name);				/*free it*/\
	}\


#undef NEWDELETE
#define NEWDELETE
#undef NEWDELETE2
#define NEWDELETE2(name)

#else
#define NEWDELETE										/*replace new & delete*/\
	void					*operator new(				/*fast allocator*/\
	size_t					size)						/*size of object*/\
	{\
		return alloc_struct(size);						/*simple to do*/\
	}\
\
	void					operator delete(			/*fast destructor*/\
	void					*deadstruct,				/*thing to free*/\
	size_t					size)						/*sizeof struct*/\
	{\
		free_struct(deadstruct,size);					/*free it*/\
	}\

#define NEWDELETE2(name)								/*replace new & delete*/\
	void					*operator new(				/*fast allocator*/\
	size_t					size)						/*size of object*/\
	{\
		return alloc_struct(size,#name);				/*simple to do*/\
	}\
\
	void					operator delete(			/*fast destructor*/\
	void					*deadstruct,				/*thing to free*/\
	size_t					size)						/*sizeof struct*/\
	{\
		free_struct(deadstruct,size,#name);				/*free it*/\
	}\

#endif
/**********************************************************************
 * ALLOC_2D_ARRAY
 *
 * Create a dynamic 2D array.
 **********************************************************************/

#define ALLOC_2D_ARRAY(x,y,mem,ptrs,type)				/*make 2d array*/\
{ \
	inT32					TMP_i; \
	mem=(type*)alloc_mem((x)*(y)*sizeof(type));			/*get memory*/\
	ptrs=(type**)alloc_mem((x)*sizeof(type*));			/*get ptrs*/\
	for (TMP_i=0;TMP_i<(x);TMP_i++)\
		ptrs[TMP_i]=mem+(y)*TMP_i;						/*set ptrs*/\
} \

/**********************************************************************
 * FREE_2D_ARRAY
 *
 * Destroy a 2D array created by ALLOC_2D_ARRAY
 **********************************************************************/

#define FREE_2D_ARRAY(mem,ptrs)							/*free a 2D array*/\
{ \
	free_mem(mem);										/*free the memory*/\
	free_mem(ptrs);										/*and the ptrs*/\
} \

/**********************************************************************
 * ALLOC_BIG_2D_ARRAY
 *
 * Create a dynamic 2D array. Use a memory allocator that allows
 * allocation of bigger chunks.
 **********************************************************************/

#define ALLOC_BIG_2D_ARRAY(x,y,mem,ptrs,type)			/*make 2d array*/\
{ \
	inT32					TMP_i; \
	mem=(type*)alloc_big_mem((x)*(y)*sizeof(type));		/*get memory*/\
	ptrs=(type**)alloc_big_mem((x)*sizeof(type*));		/*get ptrs*/\
	for (TMP_i=0;TMP_i<(x);TMP_i++)\
		ptrs[TMP_i]=mem+(y)*TMP_i;						/*set ptrs*/\
} \

/**********************************************************************
 * FREE_BIG_2D_ARRAY
 *
 * Destroy a 2D array created by ALLOC_BIG_2D_ARRAY
 **********************************************************************/

#define FREE_BIG_2D_ARRAY(mem,ptrs)						/*free a 2D array*/\
{ \
	free_big_mem(mem);									/*free the memory*/\
	free_big_mem(ptrs);									/*and the ptrs*/\
} \

extern DLLSYM void check_mem(                     //check consistency
                             const char *string,  //context message
                             inT8 level           //level of check
                            );
                                 //allocate string
extern DLLSYM char *alloc_string(inT32 count  //no of chars required
                                );
extern DLLSYM void free_string(              //free a string
                               char *string  //string to free
                              );
                                 //allocate memory
extern DLLSYM void *alloc_struct (
inT32 count,                     //no of chars required
const char *name = NULL          //class name
);
extern DLLSYM void free_struct ( //free a structure
void *deadstruct,                //structure to free
inT32 count,                     //no of bytes
const char *name = NULL          //class name
);
extern DLLSYM void *alloc_mem_p(             //allocate permanent space
                                inT32 count  //block size to allocate
                               );
extern DLLSYM void *alloc_mem(             //get some memory
                              inT32 count  //no of bytes to get
                             );
                                 //get some memory
extern DLLSYM void *alloc_big_mem(inT32 count  //no of bytes to get
                                 );
                                 //get some memory
extern DLLSYM void *alloc_big_zeros(inT32 count  //no of bytes to get
                                   );
extern DLLSYM void free_mem(                //free mem from alloc_mem
                            void *oldchunk  //chunk to free
                           );
extern DLLSYM void free_big_mem(                //free mem from alloc_mem
                                void *oldchunk  //chunk to free
                               );
#endif
