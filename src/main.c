#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <termios.h>

#include "info.h"
#include "action.h"
#include "types.h"

//TODO: make system for processing arguments
int main(int argc, char * argv[]) {
  t_state * state = malloc(sizeof(t_state));
  int c;
  int dirCount = countDir(argv[1]);
  int selected = 0;
  char cwd[256];

  state->tty = fopen("/dev/tty", "w");
  state->dirCount = &dirCount;
  state->selected = &selected;
  state->bufferArray = malloc(sizeof(char *) * 10000);
  state->cwd = cwd;

  getcwd(cwd, sizeof(cwd));
  tcgetattr( STDIN_FILENO, &state->oldt);
  cfmakeraw(&state->newt);
  tcsetattr( STDIN_FILENO, TCSANOW, &state->newt);
  fprintf(state->tty, "\e[?25l");
  changeDir(argv[1], state->bufferArray);

  //program loop
  int done = 0;
  while(!done){
    draw(state);
    done = input(state);
  }

  fprintf(state->tty, "\033[J");
  tcsetattr(STDIN_FILENO, TCSANOW, &state->oldt);
  fprintf(state->tty, "\e[?25h");
   
  exit(0);
}
