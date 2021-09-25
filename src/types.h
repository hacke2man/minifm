#ifndef __TYPES_H__
#define __TYPES_H__

#include <dirent.h>
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

typedef enum {
  FIFO,
  REGULAR,
  EXACUTABLE,
  DIRECTORY,
} file_mode;

typedef struct s_fileAttrib {
  char * name;
  file_mode fileMode;
  unsigned int gitStatus;
} t_fileAttrib;

typedef struct gitState {
  char * repoRoot;
  char * cwdRootDiff;
  git_repository * repo;
  git_index * index;
  git_status_options * opts;
  git_status_list * statuses;
} t_gitState;

typedef struct theme {
  char * normal;
  char * numberLine;
  char * gitWarn;
  char * gitAdd;
  char * directory;
  char * link;
  char * pipe;
  char * executable;
} t_theme;

typedef struct config {
  int viewHidden;
  int viewRange;
  int nameLength;
} t_config;

typedef enum {
  OUTPUT_LINE_NUM = 1
} t_flags;


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
  t_flags flags;

  //config
  t_config * config;
  t_theme * theme;
} t_state;

typedef struct termLine {
  int invertText;
  char * textEscCode;
  int textColourBg;
  int textColourFg;
  char * text;
  int invertNum;
  char * numEscCode;
  int numFg;
  int numBg;
  int lineNum;
  int numPadding;
  int nameLength;
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
t_theme * DefaultTheme();
t_config * DefaultConfig();
t_state * InitState(t_config * config, t_theme * theme);
void FreeState(t_state * state);
void GitInit(t_state * state, int error, git_status_options * opts);

#ifdef __cplusplus
}
#endif

#endif
