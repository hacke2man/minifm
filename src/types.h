#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <termios.h>
#include <git2.h>

#define true 1
#define false 0

typedef enum{
  NORMAL = 1,
  INSERT = 2,
  VISUAL = 4,
} mode;

typedef struct s_fileAttrib {
  char * name;
  unsigned int gitStatus;
  unsigned int lastModified;
  unsigned int permissions;
} t_fileAttrib;

typedef struct gitState {
  git_repository * repo;
  git_index * index;
  git_status_options * opts;
  git_status_list * statuses;
} t_gitState;

//TODO: add config struct
typedef struct state{
  //data
  t_gitState * gitState;
  mode mode;
  int * dirCount;
  int * selected;
  char * msg;
  char * cwd;
  t_fileAttrib ** fileAttribArray;
  FILE * tty;
  int topOfSelection;
  struct termios oldt;
  struct termios newt;
  //config
  int viewHidden;
  int viewRange;
} t_state;

typedef struct termLine {
  int invertText;
  int textColourBg;
  int textColourFg;
  char * text;
  int invertNum;
  int numFg;
  int numBg;
  int lineNum;
  int numPadding;
  int gitStatus;
} t_termLine;

typedef struct {
  mode mode;
  char * combo;
  int (*function)(t_state * state);
} t_action;

struct actionNode {
  t_action * action;
  struct actionNode * nextNode;
  struct actionNode * tail;
};

void freeFileAttrib(t_fileAttrib * fileAttrib);

#ifdef __cplusplus
}
#endif

#endif
