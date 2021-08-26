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
 int viewHidden;
 struct termios oldt;
 struct termios newt;
} t_state;

typedef struct {
  char * combo;
  int (*function)(t_state * state);
} t_action;

struct actionNode {
  t_action * action;
  struct actionNode * nextNode;
  struct actionNode * tail;
};

#ifdef __cplusplus
}
#endif

#endif
