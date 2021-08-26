#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <termios.h>
#include <dirent.h>

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
  int selected = *state->selected;
  int dirCount = *state->dirCount;
  char * cwd = state->cwd;
  FILE * tty = state->tty;
  char color_str[255] = {'\0'};
  int color;
  char lineNum[255];
  fprintf(tty, "\033[J");
  for(int i = getStart(selected,  dirCount);
  i < getEnd(selected,  dirCount);
  i++)
  {
    color = 37;
    sprintf(color_str, "");
    sprintf(lineNum, "\e[0;90m%d\e[0m", selected - i > 0 ? selected - i : (selected - i) * (0 - 1));

    if(isDir(bufferArray[i]))
    {
      color = 34;
      sprintf(color_str, "\e[%dm" , color);
    }

    if(i == selected)
    {
      sprintf(color_str, "\e[%d;30m" , color + 10);
      sprintf(lineNum, "\e[30;100m%d", i + 1);
    }

    fprintf(tty, "\033[K%s %s%s\e[0m\n\r", lineNum, color_str, bufferArray[i]);
  }
  fprintf(tty, "\033[%dA", getEnd(selected, dirCount) - getStart(selected, dirCount));
}

//callback for comparing file names
int compFunc(const void * a, const void * b)
{
  char ** aval = (char **) a;
  int ascore = 0;
  if(isDir(*aval))
    ascore -= 10 ;

  char ** bval = (char **) b;
  int bscore = 0;
  if(isDir(*bval))
    bscore -= 10;

  if(strcmp(*aval, *bval) >= 0)
    ascore++;
  else
    bscore++;
  
  return ascore - bscore;
}

//change proccess dir
void changeDir(char * sel, char  ** bufferArray )
{
  int dircount = countDir(sel);
  DIR * dir;
  if( sel == NULL || strlen(sel) == 0)
    dir =opendir(".");
  else
    dir = opendir(sel);

  dircount = countDir(sel);
  struct dirent * ent;
  ent = readdir(dir);

  int ind = 0;
  while(ent != NULL){
    if(ent->d_name[0] != '.')
    {
      bufferArray[ind] = malloc(sizeof(ent->d_name));
      memset(bufferArray[ind], '\0', sizeof(ent->d_name));
      strcpy(bufferArray[ind], ent->d_name);
      ind++;
    }
    ent = readdir(dir);
  }
  closedir(dir);
  chdir(sel);
  qsort(bufferArray, dircount, sizeof(char *), compFunc);
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
   
  exit(0);
}
