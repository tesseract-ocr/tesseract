/**************************************************************************
 * Filename:
              emalloc.c
**       Purpose:
              Routines for trapping memory allocation errors.
**       Author:
              Dan Johnson
              HP-UX 6.2
              HP-UX 6.2
**       History:
              4/3/89, DSJ, Created.
**
**  (c) Copyright Hewlett-Packard Company, 1988.
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
** http://www.apache.org/licenses/LICENSE-2.0
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
******************************************************************************/
/*----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------*/
#include "emalloc.h"
#include "danerror.h"
#include <stdlib.h>

/*----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------*/
/**
 * This routine attempts to allocate the specified number of
 * bytes.  If the memory can be allocated, a pointer to the
 * memory is returned.  If the memory cannot be allocated, or
 * if the allocation request is negative or zero,
 * an error is trapped.
 * @param Size number of bytes of memory to be allocated
 * @return Pointer to allocated memory.
 * @note Exceptions:
 * - #NOTENOUGHMEMORY  unable to allocate Size bytes
 * - #ILLEGALMALLOCREQUEST negative or zero request size
 * @note History: 4/3/89, DSJ, Created.
*/
void *Emalloc(int Size) {
  void *Buffer;

  if (Size <= 0)
    DoError (ILLEGALMALLOCREQUEST, "Illegal malloc request size");
  Buffer = (void *) malloc (Size);
  if (Buffer == NULL) {
    DoError (NOTENOUGHMEMORY, "Not enough memory");
    return (NULL);
  }
  else
    return (Buffer);

}                                /* Emalloc */


/*---------------------------------------------------------------------------*/
void *Erealloc(void *ptr, int size) {
  void *Buffer;

  if (size < 0 || (size == 0 && ptr == NULL))
    DoError (ILLEGALMALLOCREQUEST, "Illegal realloc request size");

  Buffer = (void *) realloc (ptr, size);
  if (Buffer == NULL && size != 0)
    DoError (NOTENOUGHMEMORY, "Not enough memory");
  return (Buffer);

}                                /* Erealloc */


/*---------------------------------------------------------------------------*/
void Efree(void *ptr) { 
  if (ptr == NULL)
    DoError (ILLEGALMALLOCREQUEST, "Attempted to free NULL ptr");

  free(ptr); 

}                                /* Efree */
