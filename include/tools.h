#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _TOOLS
#define _TOOLS
#define NO_MAX_BUF 0 /* Pour fonction Read_Input */
#endif // _TOOLS

char * Read_Input(int MAX_BUF_SIZE);

char ** Cut_String(char * str, const char * d_block);

void Array_Free(void * array,int n_dim);
