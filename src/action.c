#include <limits.h>
#include "action.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "string.h"
#include "info.h"
#include "actionPlumbing.h"
#include "core.h"

int toggleHidden(t_state * state)
{
  float newsel = (float)*state->selected / (float)*state->dirCount;
  state->viewHidden = !state->viewHidden;
  *state->dirCount = countDir(state);
  state->selected = malloc(sizeof(int) * *state->dirCount + 1);

  free(state->selected);
  for(int i = 1; i < *state->dirCount + 1; i++)
  {
    state->selected[i] = -1;
  }
  *state->selected = (int)(newsel * *state->dirCount);
  
  updateDirList(state);
  return 0;
}

//Match user input against file list. output file name if user hits enter
//TODO: highlight matches, and posible remove the files that do not
int Search(t_state * state)
{
  char ** bufferArray = state->bufferArray;
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
      return 0;
    else if(tmp[0] == '\r')
    {
      enter(state);
      return 1;
    }

    strcat(search, tmp);
    for(int i = 0; i < *dirCount; i++)
    {
      for(int j = 0; j < strlen(bufferArray[i]); j++)
      {
        if(bufferArray[i][j] == search[0])
        {
          currentScore = matchScore(&search[j], &bufferArray[i][j]);

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
    numMatch = 0;
  }
}

//output name of selected file and exit program
int enter(t_state * state)
{
  char ** bufferArray = state->bufferArray;
  int * selected = state->selected;
  int * dirCount = state->dirCount;
  char * cwd = state->cwd;
  FILE * tty = state->tty;

  char * sel = malloc(sizeof(char) * (strlen(bufferArray[*selected]) + 1));
  strcpy(sel, bufferArray[*selected]);
  *selected = 0;

  for(int i = 0; i < *dirCount; i++)
    free(bufferArray[i]);

  fprintf(tty, "\033[J");
  strcat(cwd, "/");
  strcat(cwd, sel);

  fprintf(tty, "\e[?25h");
  printf("%s", cwd);
  return 1;
}

int moveDown(t_state * state)
{
  int * selected = state->selected;
  int * dirCount = state->dirCount;
  *selected = *selected < *dirCount - 1 ? *selected + 1 : *selected;
  return 0;
}

int moveUp(t_state * state)
{
  int * selected = state->selected;
  *selected = *selected > 0 ? *selected - 1 : *selected;
  return 0;
}

int exitProgram(t_state * state)
{
  return 1;
}

void freeAction(t_action * action)
{
  free(action->combo);
  free(action->function);
}

int gotoTop(t_state * state)
{
  int * selected = state->selected;
  *selected = 0;
  return 0;
}

int gotoBottom(t_state * state){
  int * selected = state->selected;
  int * dirCount = state->dirCount;
  *selected = *dirCount - 1;
  return 0;
}

int backDir(t_state * state)
{
  char ** bufferArray = state->bufferArray;
  int * dirCount = state->dirCount;
  FILE * tty = state->tty;
  for(int i = 0; i < *dirCount; i++)
    free(bufferArray[i]);

  fprintf(tty, "\033[J");
  tcsetattr(STDIN_FILENO, TCSANOW, &state->oldt);
  fprintf(tty, "\e[?25h");
  printf("..");
  exit(0);
}

int removeFile(t_state * state)
{
  // printf("%s",state->bufferArray[*state->selected]);
  remove(state->bufferArray[*state->selected]);
  updateDirList(state);
  return 1;
}

int yank(t_state * state)
{
  char yankListPath[PATH_MAX];
  sprintf(yankListPath, "%s/mfm/yanklist", getenv("XDG_DATA_HOME"));
  FILE * yankList = fopen(yankListPath, "w");
  fprintf(yankList, "%s/%s", state->cwd, state->bufferArray[*state->selected]);
  return 1;
}

int put(t_state * state)
{
  char yankListPath[PATH_MAX];
  sprintf(yankListPath, "%s/mfm/yanklist", getenv("XDG_DATA_HOME"));
  FILE * yankList = fopen(yankListPath, "r");
  char putFile[PATH_MAX];
  fgets(putFile, PATH_MAX, yankList);

  char command[2*PATH_MAX + 1];
  sprintf(command, "cp -r %s %s", putFile, state->cwd);
  system(command);

  // updateDirList(state);
  return 1;
}

int compareInt(const void * a, const void * b)
{
  return *(int *)a - *(int *)b;
}

//TODO: change to mode
//FIXME: randomly crashes program
int Visual(t_state * state)
{
  char ** bufferArray = state->bufferArray;
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
  int topOfSelection = 0;
  while(1)
  {
    tmp[0] = getchar();

    int i = 0;
    for(; selected[i] != -1; i++){}
    if(tmp[0] == 27)
    {
      selected[1] = -1;
      for(int i = 1; i < *dirCount; i++)
        selected[i] = -1;
      return 0;

    } else if(tmp[0] == '\r') {
      int i = 0;
      for(; selected[i] != -1; i++){
        printf("%d,", selected[i]);
      }
      getchar();
      return 0;

    } else if ( tmp[0] == 'j')
    {
      if(topOfSelection && selected[i-1] < *dirCount)
      {
        if(selected[1] == -1)
        {
          topOfSelection = !topOfSelection;
          selected[i] = *state->selected + i;
        } else {
          for(int i = 0; selected[i] != -1; i++)
            selected[i] = selected[i + 1];
        }

      } else if(selected[i-1] < *dirCount - 1) {
        selected[i] = *state->selected + i;
      }

    } else if ( tmp[0] == 'k') {
      if(topOfSelection && *selected > 0)
      {
        int tmp = selected[0];
        int tmp2;
        selected[0] = selected[0] - 1;
        int i = 1;
        for(; selected[i] != -1; i++)
        {
          tmp2 = selected[i];
          selected[i] = tmp;
          tmp = tmp2;
        }
        tmp2 = selected[i];
        selected[i] = tmp;

      } else if(*selected > 0) {
        if(selected[i-1] == *selected)
        {
          topOfSelection = !topOfSelection;
        int tmp = selected[0];
        int tmp2;
        selected[0] = selected[0] - 1;
        int i = 1;
        for(; selected[i] != -1; i++)
        {
          tmp2 = selected[i];
          selected[i] = tmp;
          tmp = tmp2;
        }
        tmp2 = selected[i];
        selected[i] = tmp;

        } else {
          int i = 0;
          for(; selected[i] != -1; i++){}
            selected[i - 1] = -1;
        }
      }

    }else if(tmp[0] == 'o')
      topOfSelection = !topOfSelection;
    draw(state);
  }
  return 0;
}
