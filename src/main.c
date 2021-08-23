#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <termios.h>

#include "info.h"
#include "action.h"
#include "types.h"

//TODO: implement multi char commands
//TODO: make gg move to top of list
//TODO: implement system for adding counts to commands
//TODO: make dd delete file under cursor
//TODO: implement yank, and put
//TODO: visual mode
//TODO: insert mode to rename files
//TODO: hide/show hidden files
int input(t_state * state)
{
  char ** bufferArray = state->bufferArray;
  int * selected = state->selected;
  int * dirCount = state->dirCount;
  char * cwd = state->cwd;
  FILE * tty = state->tty;
  char chr = getchar();

  switch(chr)
  {
    case 27:
      return 1;
      break;
    case 'j':
      *selected = *selected < *dirCount - 1 ? *selected + 1 : *selected;
      break;
    case 'k':
      *selected = *selected > 0 ? *selected - 1 : *selected;
      break;
    case 'g':
      *selected = 0;
      break;
    case 'G':
      *selected = *dirCount - 1;
      break;
    case '\r':
      if(enter(state) == 0)
      return 1;
      break;
    case 'b':

      for(int i = 0; i < *dirCount; i++)
        free(bufferArray[i]);

      fprintf(tty, "\033[J");
      tcsetattr(STDIN_FILENO, TCSANOW, &state->oldt);
      fprintf(tty, "\e[?25h");
      printf("..");
      exit(0);
      break;
    case '/':
      Search(state);
      return 1;
      break;
  }

  return 0;
}

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
