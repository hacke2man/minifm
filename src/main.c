#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <termios.h>

#include "info.h"
#include "core.h"

//TODO: convert bool nums to use true and false vars

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
  state->mode = NORMAL;
  state->topOfSelection = 1;
  state->viewRange = 12;
  state->msg = malloc(sizeof(char) * PATH_MAX);
  state->msg[0] = '\0';

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

  free(state->msg);
  free(state->selected);
  free(state);
   
  exit(0);
}
