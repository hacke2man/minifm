#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "action.h"
#include "actionPlumbing.h"
#include "info.h"

int isCursorLine(t_state * state, int line)
{
  int * selected = state->selected;
  if(state->topOfSelection)
  {
    if(line == *selected)
      return 1;
  } else {
    int j = 0;
    for(; selected[j] != -1; j++);

    if(line == selected[j - 1])
      return 1;
  }
  return 0;
}

void printLine(t_state * state, t_termLine * line)
{
  int invertNum = line->invertNum ? 7 : 0;
  int invertText = line->invertText ? 7 : 0;
  fprintf(state->tty,
  "\e[%d;%d;%dm%*d \e[%d;%d;%dm%s\e[0m\n\r",
  invertNum,
  line->numFg,
  line->numBg,
  line->numPadding,
  line->lineNum,
  invertText,
  line->textColourFg,
  line->textColourBg,
  line->text);
}

void setDefaultLine(t_termLine * line)
{
  line->invertText = 0;
  line->textColourFg = 39;
  line->textColourBg = 49;
  line->text = NULL;
  line->invertNum = 0;
  line->numBg = 49;
  line->numFg = 90;
}

int isSelected(t_state * state, int lineNum)
{
  for(int j = 0; state->selected[j] != -1; j++){
    if(lineNum == state->selected[j])
    {
      return 1;
    }
  }
  return 0;
}

//draws to terminal
//TODO: add max name langth
//TODO: themes
//TODO: colour pipes, and files with execute privilage different
//TODO: add option to show git info
void draw(t_state * state)
{
  t_termLine * line = malloc(sizeof(t_termLine));
  line->numPadding = (int)ceil(log10((double)*state->dirCount + 1));

  fprintf(state->tty, "\033[J");

  for(int i = getStart(state);
      i < getEnd(state);
      i++)
  {
    setDefaultLine(line);
    line->lineNum = *state->selected - i > 0 ? *state->selected - i : (*state->selected - i) * -1;

    if(isCursorLine(state, i))
    {
      line->lineNum = i + 1;
      line->invertNum = 1;
    }

    if(isDir(state->fileAttribArray[i]->name))
      line->textColourFg = 34;

    line->invertText = isSelected(state, i);
    line->text = state->fileAttribArray[i]->name;
    printLine(state, line);
  }
  fprintf(state->tty, "%s\n\r", state->msg);
  fprintf(state->tty, "\033[%dA", getEnd(state) - getStart(state) + 1);
  free(line);
}

int canMatch(mode mode, char * combo, struct actionNode * head)
{
  int matchable = 0;
  char * cmpCombo;

  for(int i = 0; combo[i] > 48 && combo[i] < 47; i++);

  while (head)
  {
    if(strlen(combo) <= strlen(head->action->combo) && (head->action->mode & mode) > 0)
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

//TODO: insert mode to rename files
struct actionNode * initDefaultMappings()
{
  struct actionNode * commands;

  commands = initList(initAction(NORMAL | VISUAL, "\x1b", escape));
  listQueue(commands, initAction(NORMAL, "\r", enter));
  listQueue(commands, initAction(NORMAL, "/", Search));
  listQueue(commands, initAction(NORMAL, "j", moveDown));
  listQueue(commands, initAction(NORMAL, "k", moveUp));
  listQueue(commands, initAction(NORMAL, "gg", gotoTop));
  listQueue(commands, initAction(NORMAL, "G", gotoBottom));
  listQueue(commands, initAction(NORMAL, "b", backDir));
  listQueue(commands, initAction(NORMAL, " h", toggleHidden));
  listQueue(commands, initAction(NORMAL, "V", selectOne));
  listQueue(commands, initAction(NORMAL | VISUAL, "dd", removeFile));
  listQueue(commands, initAction(NORMAL | VISUAL, "yy", yank));
  listQueue(commands, initAction(NORMAL, "p", put));
  listQueue(commands, initAction(NORMAL, "\4", halfPageDown)); //ctrl-D
  listQueue(commands, initAction(NORMAL, "\x15", halfPageUp)); //ctrl-U

  listQueue(commands, initAction(NORMAL, "v", enterVisual));
  listQueue(commands, initAction(VISUAL, "j", visualMoveDown));
  listQueue(commands, initAction(VISUAL, "k", visualMoveUp));
  listQueue(commands, initAction(VISUAL, "o", changeSelectionPos));
  listQueue(commands, initAction(VISUAL, "\r", printSelected));

  return commands;
}

int input(t_state * state, struct actionNode * commands)
{
  char tmp[2] = {' ', '\0'};
  char combo[256];
  combo[0] = '\0';
  char countStr[256];
  char msg[256];
  int countInt;

  countStr[0] = '\0';
  combo[0] = '\0';

  struct actionNode * commandPointer;
  commandPointer = commands;
  int containsNonNum = 0;
  while(1)
  {
    tmp[0] = getchar();
    if(tmp[0] == 27 && (strlen(combo) > 1 || strlen(countStr)))
    {
      countStr[0] = '\0';
      combo[0] = '\0';
      countInt = 0;
      sprintf(state->msg, "");
      draw(state);
      continue;
    }

    if(containsNonNum)
    {
      if (tmp[0] >= 48 && tmp[0] <= 57)
      {
        combo[0] = '\0';
        countStr[0] = '\0';
        countInt = 0;
        continue;
      }
    } else {
      if (tmp[0] >= 48 && tmp[0] <= 57)
      {
        strcat(countStr, tmp);
        sprintf(state->msg, "%s%s", countStr, combo);
        draw(state);
        continue;
      }
    }
    containsNonNum = 1;
    strcat(combo, tmp);

    if (!canMatch(state->mode, combo, commandPointer))
    {
      combo[0] = '\0';
      countStr[0] = '\0';
      return 0;
    }
    sprintf(state->msg, "%s%s", countStr, combo);
    draw(state);

    while(commandPointer != NULL)
    {
      if (strcmp(combo, commandPointer->action->combo) == 0 && (commandPointer->action->mode & state->mode) > 0)
      {
        countInt = strtol(countStr, NULL, 10);
        countInt = countInt > 0 ? countInt : 1;
        for(int i = 0; i < countInt; i++)
        {
          if(1 == commandPointer->action->function(state))
          {
            countStr[0] = '\0';
            combo[0] = '\0';
            sprintf(state->msg, "");
            return 1;
          }
        }

        countStr[0] = '\0';
        combo[0] = '\0';
        sprintf(state->msg, "");
        return 0;
      }
      commandPointer = commandPointer->nextNode;
    }
    commandPointer = commands;
  }
  return 0;
}
