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
 char ** deletedFiles;
 char * cwd;
 FILE * tty;
 int viewHidden;
 struct termios oldt;
 struct termios newt;
} t_state;

typedef enum{
  NORMAL = 1,
  INSERT = 2,
  VISUAL = 4,
} mode;

typedef struct {
  mode * mode;
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
