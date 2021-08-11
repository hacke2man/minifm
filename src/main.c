#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "info.h"
#include "action.h"

void draw(char ** out, int dirCount, int selected)
{
  char color_str[255] = {'\0'};
  int color;
  char lineNum[255];
  for(int i = getStart(selected,  dirCount);
  i < getEnd(selected,  dirCount);
  i++)
  {
    color = 37;
    sprintf(color_str, "");
    sprintf(lineNum, "\e[0;90m%d\e[0m", selected - i > 0 ? selected - i : (selected - i) * (0 - 1));

    if(isDir(out[i]))
    {
      color = 34;
      sprintf(color_str, "\e[%dm" , color);
    }

    if(i == selected)
    {
      sprintf(color_str, "\e[%d;30m" , color + 10);
      sprintf(lineNum, "\e[30;100m%d", i + 1);
    }

    printf("\033[K%s %s%s\e[0m\n\r", lineNum, color_str, out[i]);
  }
  printf("\033[%dA", getEnd(selected, dirCount) - getStart(selected, dirCount));
}

void input(char ** out, int * selected, int * dirCount, char * cwd)
{
  char chr = getchar();
  switch(chr)
  {
    case 27:
      system ("/bin/stty -raw");
      system ("/bin/stty echo");
      printf("\e[?25h");
      getcwd(cwd, sizeof(cwd));
      printf("%s\n", cwd);
      exit(0);
      break;
    case 'j':
      *selected = *selected < *dirCount - 1 ? *selected + 1 : *selected;
      break;
    case 'k':
      *selected = *selected > 0 ? *selected - 1 : *selected;
      break;
    //FIXME: make .. work
    case '\r':
      if(isDir(out[*selected]) && *selected >= 0 && *selected < *dirCount)
      {
        char * sel = malloc(sizeof(out[*selected]));
        strcpy(sel, out[*selected]);
        *selected = 0;

        for(int i = 0; i < *dirCount; i++)
          free(out[i]);

        printf("\033[J");
        *dirCount = countDir(sel);

        changeDir(sel, out);
      } else if (*selected >= 0 && *selected < *dirCount)
      editFile(out[*selected]);
      break;
    case 'b':
      for(int i = 0; i < *dirCount; i++)
      free(out[i]);

      *dirCount = countDir(".");
      changeDir("..", out);
      break;
  }
}

int main(int argc, char * argv[]) {
  system ("/bin/stty raw");
  system ("/bin/stty -echo");
  printf("\e[?25l");

  int dirCount = countDir(argv[1]);
  int selected = 0;
  char ** out = malloc(sizeof(char *) * 10000);
  char cwd[256];
  getcwd(cwd, sizeof(cwd));
  

  changeDir(argv[1], out);
  while(1){
    draw(out, dirCount, selected);
    input(out, &selected, &dirCount, cwd);
  }
}
