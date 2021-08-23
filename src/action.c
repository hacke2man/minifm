#include "action.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include "string.h"
#include "info.h"

//draws to terminal
//TODO: add option to show git info
void draw(t_state * state)
{
  char ** out = state->bufferArray;
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

    fprintf(tty, "\033[K%s %s%s\e[0m\n\r", lineNum, color_str, out[i]);
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
void changeDir(char * sel, char  ** out )
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
      out[ind] = malloc(sizeof(ent->d_name));
      memset(out[ind], '\0', sizeof(ent->d_name));
      strcpy(out[ind], ent->d_name);
      ind++;
    }
    ent = readdir(dir);
  }
  closedir(dir);
  chdir(sel);
  qsort(out, dircount, sizeof(char *), compFunc);
}

//output name of selected file and exit program
int enter(t_state * state)
{
  char ** out = state->bufferArray;
  int * selected = state->selected;
  int * dirCount = state->dirCount;
  char * cwd = state->cwd;
  FILE * tty = state->tty;

  char * sel = malloc(sizeof(out[*selected]));
  strcpy(sel, out[*selected]);
  *selected = 0;

  for(int i = 0; i < *dirCount; i++)
    free(out[i]);

  fprintf(tty, "\033[J");
  strcat(cwd, "/");
  strcat(cwd, sel);

  fprintf(tty, "\e[?25h");
  printf("%s", cwd);
  return 0;
}

//assings a score based on how many letters match in strings
int matchScore(char * search, char * check)
{
  int score = 0;
  for(int i = 0; i < strlen(check); i++)
  {
    if(search[i] == check[i])
    {
      score++;
    } else {
      break;
    }
  }
  return score;
}

//Match user input against file list. output file name if only one match
//or user hits enter
//TODO: highlight matches, and posible remove the files that do not
void Search(t_state * state)
{
  char ** out = state->bufferArray;
  int * selected = state->selected;
  int * dirCount = state->dirCount;
  char * cwd = state->cwd;
  FILE * tty = state->tty;
  char tmp[2] = {' ', '\0'};
  char search[256];
  search[0] = '\0';
  int bestScore = 0;
  int bestMatchIndex = 0;
  int currentScore = 0;
  int numMatch = 0;
  while(1)
  {
    tmp[0] = getchar();
    if(tmp[0] == 27)
    break;
    else if(tmp[0] == '\r')
    {
      enter(state);
      break;
    }

    strcat(search, tmp);
    for(int i = 0; i < *dirCount; i++)
    {
      for(int j = 0; j < strlen(out[i]); j++)
      {
        if(out[i][j] == search[0])
        {
          currentScore = matchScore(&search[j], &out[i][j]);

          if (currentScore > bestScore)
          {
            bestScore = currentScore;
            bestMatchIndex = i;
            *selected = bestMatchIndex;
            draw(state);
            numMatch++;
          } else if (currentScore == bestScore)
          numMatch++;
        }
      }
    }
    if (numMatch == 1)
    {
      enter(state);
      return;
    }
    numMatch = 0;
  }
}
