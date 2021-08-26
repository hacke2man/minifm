#include "action.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include "string.h"
#include "info.h"

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

typedef struct {
  char * combo;
  int (*function)(t_state * state);
} t_action;

t_action * initAction(char * combo, int (*function)(t_state *) )
{
  t_action * newAction;

  newAction = malloc(sizeof(t_action));
  newAction->combo = malloc(sizeof(int));
  newAction->combo = combo;
  int (*fp)(t_state * state) = function;
  newAction->function = malloc(sizeof(&exitProgram));
  newAction->function = function;
  return newAction;
}

void freeAction(t_action * action)
{
  free(action->combo);
  free(action->function);
}

struct actionNode {
  t_action * action;
  struct actionNode * nextNode;
  struct actionNode * tail;
};


int canMatch(char * combo, struct actionNode * head, int numActions)
{
  int matchable = 0;
  while (head)
  {
    if(strlen(combo) <= strlen(head->action->combo))
    {
      for(int j = 0; j < strlen(combo); j++)
      {
        if (combo[j] != head->action->combo[j]) 
          break;
        if(j == strlen(combo) - 1)
          matchable = 1;
      }
    }
    head = head->nextNode;
  }
  return matchable;
}

int gotoTop(t_state * state)
{
  int * selected = state->selected;
  *selected = 0;
  return 0;
}

void listQueue(struct actionNode * commands, t_action * action)
{
  commands->tail->nextNode = malloc(sizeof(struct actionNode));
  commands->tail = commands->tail->nextNode;
  commands->tail->action = action;
  commands->tail->nextNode = NULL;
}

struct actionNode * initList(t_action * action)
{
  struct actionNode * commands = malloc(sizeof(struct actionNode));
  commands->action = malloc(sizeof(t_action));
  commands->action = action;
  commands->tail = commands;
  commands->nextNode = NULL;
  return commands;
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

//TODO: implement system for adding counts to commands
//TODO: make dd delete file under cursor
//TODO: implement yank, and put
//TODO: visual mode
//TODO: insert mode to rename files
//TODO: hide/show hidden files
int input(t_state * state)
{
  int cmdNum = 3;
  struct actionNode * commands;

  commands = initList(initAction("\x1b", exitProgram));
  listQueue(commands, initAction("\r", enter));
  listQueue(commands, initAction("/", startSearch));
  listQueue(commands, initAction("j", moveDown));
  listQueue(commands, initAction("k", moveUp));
  listQueue(commands, initAction("gg", gotoTop));
  listQueue(commands, initAction("G", gotoBottom));
  listQueue(commands, initAction("b", backDir));

  char tmp[2] = {' ', '\0'};
  char combo[256];
  combo[0] = '\0';

  struct actionNode * commandPointer;
  commandPointer = commands;
  while(1)
  {
    tmp[0] = getchar();
    strcat(combo, tmp);

    if (!canMatch(combo, commandPointer, cmdNum))
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

//draws to terminal
//TODO: add option to show git info
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
