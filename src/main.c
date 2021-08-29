#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <termios.h>

#include "info.h"
#include "action.h"
#include "types.h"
#include "actionPlumbing.h"


//draws to terminal
//TODO: add option to show git info
//TODO: refactor
void draw(t_state * state)
{
  char ** bufferArray = state->bufferArray;
  int * selected = state->selected;
  int dirCount = *state->dirCount;
  char * cwd = state->cwd;
  FILE * tty = state->tty;
  char color_str[255] = {'\0'};
  int color;
  char lineNum[255];
  fprintf(tty, "\033[J");
  for(int i = getStart(*selected,  dirCount);
      i < getEnd(*selected,  dirCount);
      i++)
  {
    color = 37;
    sprintf(color_str, "");
    sprintf(lineNum, "\e[0;90m%d\e[0m", *selected - i > 0 ? *selected - i : (*selected - i) * (0 - 1));

    if(isDir(bufferArray[i]))
    {
      color = 34;
      sprintf(color_str, "\e[%dm" , color);
    }

    for(int j = 0; selected[j] != -1; j++){
      if(i == selected[j])
      {
        sprintf(color_str, "\e[%d;30m" , color + 10);
        sprintf(lineNum, "\e[30;100m%d", i + 1);
        break;
      }
    }

    fprintf(tty, "\033[K%s %s%s\e[0m\n\r", lineNum, color_str, bufferArray[i]);
  }
  fprintf(tty, "\033[%dA", getEnd(*selected, dirCount) - getStart(*selected, dirCount));
}

//TODO: make system for processing arguments
int main(int argc, char * argv[]) {
  t_state * state = malloc(sizeof(t_state));
  int c;
  char cwd[PATH_MAX];
  int dirCount;

  state->viewHidden = 0;
  state->dirCount = &dirCount;
  state->tty = fopen("/dev/tty", "w");
  state->bufferArray = malloc(sizeof(char *) * 10000);
  state->cwd = cwd;
  state->selected = malloc(sizeof(int) * dirCount + 1);

  for(int i = 1; i < dirCount + 1; i++)
  {
    state->selected[i] = -1;
  }
  state->selected[0] = 0;

  getcwd(cwd, sizeof(cwd));
  dirCount = countDir(state);
  tcgetattr( STDIN_FILENO, &state->oldt);
  cfmakeraw(&state->newt);
  tcsetattr( STDIN_FILENO, TCSANOW, &state->newt);
  fprintf(state->tty, "\e[?25l");
  updateDirList(state);

  struct actionNode * commands = initDefaultMappings();

  //program loop
  int done = 0;
  while(!done){
    draw(state);
    done = input(state, commands);
  }

  fprintf(state->tty, "\033[J");
  tcsetattr(STDIN_FILENO, TCSANOW, &state->oldt);
  fprintf(state->tty, "\e[?25h");

  free(state->selected);
   
  exit(0);
}
