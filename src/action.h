#ifndef __ACTION_H__
#define __ACTION_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include "types.h"

void changeDir(char * sel, char  ** bufferArray );
void editFile(char * selected, FILE * tty, struct termios oldt);
int enter(t_state * state);
void draw(t_state * state);
void Search(t_state * state);
int input(t_state * state);

#ifdef __cplusplus
}
#endif

#endif
