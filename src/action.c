#include "action.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "string.h"
#include "info.h"

#include "actionPlumbing.h"

int toggleHidden(t_state * state)
{
  float newsel = (float)*state->selected / (float)*state->dirCount;
  state->viewHidden = !state->viewHidden;
  *state->dirCount = countDir(state->cwd, state->viewHidden);
  *state->selected = (int)(newsel * *state->dirCount);
  
  updateDirList(state->cwd, state->bufferArray, state->viewHidden);
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

  char * sel = malloc(sizeof(bufferArray[*selected]));
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

int startSearch(t_state * state)
{
  return Search(state);
}

//TODO: make dd delete file under cursor
//TODO: implement yank, and put
//TODO: visual mode
//TODO: insert mode to rename files
//TODO: hide/show hidden files
struct actionNode * initDefaultMappings()
{
  struct actionNode * commands;

  commands = initList(initAction("\x1b", exitProgram));
  listQueue(commands, initAction("\r", enter));
  listQueue(commands, initAction("/", startSearch));
  listQueue(commands, initAction("j", moveDown));
  listQueue(commands, initAction("k", moveUp));
  listQueue(commands, initAction("gg", gotoTop));
  listQueue(commands, initAction("G", gotoBottom));
  listQueue(commands, initAction("b", backDir));
  listQueue(commands, initAction(" h", toggleHidden));
  return commands;
}

//TODO: implement system for adding counts to commands
int input(t_state * state, struct actionNode * commands)
{
  char tmp[2] = {' ', '\0'};
  char combo[256];
  combo[0] = '\0';

  struct actionNode * commandPointer;
  commandPointer = commands;
  while(1)
  {
    tmp[0] = getchar();
    strcat(combo, tmp);

    if (!canMatch(combo, commandPointer))
    {
      return 0;
    }

    while(commandPointer != NULL)
    {
      if (strcmp(combo, commandPointer->action->combo) == 0)
      {
        return commandPointer->action->function(state);
      }
      commandPointer = commandPointer->nextNode;
    }
    commandPointer = commands;
  }
  return 0;
}
