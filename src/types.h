#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <termios.h>

typedef struct state{
 int * dirCount;
 int * selected;
 char ** bufferArray;
 char * cwd;
 FILE * tty;
 struct termios oldt;
 struct termios newt;
} t_state;

#ifdef __cplusplus
}
#endif

#endif
