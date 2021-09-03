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

int enterVisual(t_state * state)
{
  for(int i = 1; i < *state->dirCount; i++)
    state->selected[i] = -1;
  state->mode = VISUAL;
  return 0;
}

int printSelected(t_state * state)
{
  for(int i = 0; i < *state->dirCount; i++)
    printf("%d", state->selected[i]);
  return 0;
}

int visualMoveDown(t_state * state)
{
  int * selected = state->selected;
  int * dirCount = state->dirCount;
  int topOfSelection = state->topOfSelection;
  int i = 0;
  for(; selected[i] != -1; i++);

  if(topOfSelection)
  {
    if(selected[1] != -1)
    {
      int i = 1;
      for(; selected[i] != -1 ; i++)
       selected[i - 1] = selected[i];
      selected[i - 1] = -1;
    } else {
      state->topOfSelection = !state->topOfSelection;
      if(selected[i - 1] < *dirCount - 1)
      {
        visualMoveDown(state);
      }
    }
  } else {
    if(selected[i - 1] < *dirCount - 1)
    {
      selected[i] = selected[i - 1] + 1;
    }
  }
  return 0;
}

int visualMoveUp(t_state * state)
{
  int * selected = state->selected;
  int * topOfSelection = &state->topOfSelection;
  int i = 0;
  for(; selected[i] != -1; i++);


  if(*topOfSelection)
  {
    if(*selected > 0) {
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
    }

  } else {
    if(selected[i-1] == *selected)
    {
      *topOfSelection = !(*topOfSelection);
      if(*selected > 0) {
        visualMoveUp(state);
      }
    } else {
      int i = 0;
      for(; selected[i] != -1; i++);
      selected[i - 1] = -1;
    }
  }
  return 0;
}

int changeSelectionPos(t_state * state)
{
  state->topOfSelection = !state->topOfSelection;
  return 0;
}

int selectOne(t_state * state)
{
  int i = 1;
  for(; state->selected[i] != -1; i++){
    if(state->selected[i] == state->selected[0])
      break;
  }
  state->selected[i] = state->selected[0];
  return 0;
}

//Match user input against file list. output file name if user hits enter
//TODO: highlight matches, and posible remove the files that do not
//COULD: make into a mode
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

int halfPageDown(t_state * state)
{
  for(int i = 0; i < state->viewRange/2; i++)
    moveDown(state);
  return 0;
}

int halfPageUp(t_state * state)
{
  for(int i = 0; i < state->viewRange/2; i++)
    moveUp(state);
  return 0;
}

int escape(t_state * state)
{
  if(state->selected[1] != -1)
  {
    for(int i = 1; i < *state->dirCount; i++)
      state->selected[i] = -1;
    state->mode = NORMAL;
    state->topOfSelection = 1;
    return 0;
  }
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
  for(int i = 0; state->selected[i] != -1; i++)
    remove(state->bufferArray[state->selected[i]]);
  updateDirList(state);
  return 1;
}

int yank(t_state * state)
{
  FILE * yankList = fopen("/tmp/yanklist", "w");

  int i = state->mode != VISUAL && state->selected[1] != -1 ? 1 : 0;
  for(; state->selected[i] != -1; i++)
    fprintf(yankList, "%s/%s\n", state->cwd, state->bufferArray[state->selected[i]]);
  return 1;
}

//TODO: error checking
int put(t_state * state)
{
  FILE * yankList = fopen("/tmp/yanklist", "r");
  char putFile[PATH_MAX];
  char command[2*PATH_MAX + 1];
  char * newline;

  while(fgets(putFile, PATH_MAX, yankList))
  {
    newline = strchr(putFile, '\n');
    *newline = '\0';
    sprintf(command, "cp -r %s %s", putFile, state->cwd);
    system(command);
  }
  return 1;
}
