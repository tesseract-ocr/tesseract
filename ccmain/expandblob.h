#ifndef EXPANDBLOB_H
#define EXPANDBLOB_H

#include "tessclas.h"

void free_blob(register TBLOB *blob); 

void free_tree(register TESSLINE *outline); 

void free_outline(register TESSLINE *outline); 

void free_loop(register EDGEPT *startpt); 
#endif
