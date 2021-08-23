#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <termios.h>

#include "info.h"
#include "action.h"
#include "types.h"


// int input(char ** out, int * selected, int * dirCount, char * cwd, FILE * tty)
int input(t_state * state)
{
  char ** out = state->bufferArray;
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
    case '\r':
      if(enter(state) == 0)
        return 1;
      break;
    case 'b':

    for(int i = 0; i < *dirCount; i++)
      free(out[i]);

    fprintf(tty, "\033[J");

    tcsetattr(STDIN_FILENO, TCSANOW, &state->oldt);
    fprintf(tty, "\e[?25h");
    printf("cd\n..\n");
    exit(0);
    // changeDir(cwd, out);
      break;
    case '/':
      Search(state);
    break;
  }

  return 0;
}

int main(int argc, char * argv[]) {
  int done = 0;
  FILE * tty = fopen("/dev/tty", "w");
  int c;
  static struct termios oldt, newt;
  tcgetattr( STDIN_FILENO, &oldt);
  /* newt = oldt;
  newt.c_lflag &= ~(ICANON); */
  cfmakeraw(&newt);
  tcsetattr( STDIN_FILENO, TCSANOW, &newt);
  fprintf(tty, "\e[?25l");

  int dirCount = countDir(argv[1]);
  int selected = 0;
  char ** out = malloc(sizeof(char *) * 10000);
  char cwd[256];
  getcwd(cwd, sizeof(cwd));
  
  t_state * state = malloc(sizeof(t_state));
  state->cwd = cwd;
  state->bufferArray = out;
  state->dirCount = &dirCount;
  state->newt = newt;
  state->oldt = oldt;
  state->selected = &selected;
  state->tty = tty;

  changeDir(argv[1], out);
  while(!done){
    draw(state);
    done = input(state);
  }

  fprintf(tty, "\033[J");
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fprintf(tty, "\e[?25h");
  // printf("echo\n-n\n");
  // printf("%s", cwd);
   
  exit(0);
}
