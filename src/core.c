#include <string.h>
#include "action.h"
#include "actionPlumbing.h"
#include "info.h"

//draws to terminal
//TODO: add option to show git info
//TODO: refactor
void draw(t_state * state)
{
  char ** bufferArray = state->bufferArray;
  int * selected = state->selected;
  int dirCount = *state->dirCount;
  char * cwd = state->cwd;
  FILE * tty = state->tty;
  char color_str[255] = {'\0'};
  int color;
  char lineNum[255];
  fprintf(tty, "\033[J");
  for(int i = getStart(*selected,  dirCount);
      i < getEnd(*selected,  dirCount);
      i++)
  {
    color = 37;
    sprintf(color_str, "");
    sprintf(lineNum, "\e[0;90m%d\e[0m", *selected - i > 0 ? *selected - i : (*selected - i) * (0 - 1));

    if(isDir(bufferArray[i]))
    {
      color = 34;
      sprintf(color_str, "\e[%dm" , color);
    }

    for(int j = 0; selected[j] != -1; j++){
      if(i == selected[j])
      {
        sprintf(color_str, "\e[%d;30m" , color + 10);
        sprintf(lineNum, "\e[30;100m%d", i + 1);
        break;
      }
    }

    fprintf(tty, "\033[K%s %s%s\e[0m\n\r", lineNum, color_str, bufferArray[i]);
  }
  fprintf(tty, "\033[%dA", getEnd(*selected, dirCount) - getStart(*selected, dirCount));
}

int canMatch(mode mode, char * combo, struct actionNode * head)
{
  int matchable = 0;
  while (head)
  {
    if(strlen(combo) <= strlen(head->action->combo) && head->action->mode == mode)
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

//TODO: visual modes
//TODO: insert mode to rename files
struct actionNode * initDefaultMappings()
{
  struct actionNode * commands;

  commands = initList(initAction(NORMAL, "\x1b", exitProgram));
  listQueue(commands, initAction(NORMAL, "\r", enter));
  listQueue(commands, initAction(NORMAL, "/", Search));
  listQueue(commands, initAction(NORMAL, "j", moveDown));
  listQueue(commands, initAction(NORMAL, "k", moveUp));
  listQueue(commands, initAction(NORMAL, "gg", gotoTop));
  listQueue(commands, initAction(NORMAL, "G", gotoBottom));
  listQueue(commands, initAction(NORMAL, "b", backDir));
  listQueue(commands, initAction(NORMAL, " h", toggleHidden));
  listQueue(commands, initAction(NORMAL, "dd", removeFile));
  listQueue(commands, initAction(NORMAL, "yy", yank));
  listQueue(commands, initAction(NORMAL, "p", put));

  listQueue(commands, initAction(NORMAL, "v", enterVisual));
  listQueue(commands, initAction(VISUAL, "\x1b", escapeVisual));

  return commands;
}

//TODO: implement modes
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

      if (!canMatch(state->mode, combo, commandPointer))
      {
        return 0;
      }

      while(commandPointer != NULL)
      {
        if (strcmp(combo, commandPointer->action->combo) == 0 && commandPointer->action->mode == state->mode)
        {
          return commandPointer->action->function(state);
        }
        commandPointer = commandPointer->nextNode;
      }
      commandPointer = commands;
    }
    return 0;
}
