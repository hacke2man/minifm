#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "action.h"
#include "actionPlumbing.h"
#include "info.h"
#include <git2.h>
#include <limits.h>
#include <string.h>

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

enum statusFlags {
  IX_MODIFIED = 1,
  IX_MODIFIED_B = 1 << 1,
  WT_MODIFIED = 1 << 8,
};

char * PrintStatus(char * statusString, unsigned int status)
{
  char wtChar = ' ';
  char ixChar = ' ';

  if((status & WT_MODIFIED) > 0)
    wtChar = 'M';

  if((status & IX_MODIFIED) > 0)
    ixChar = 'M';

  //HACK: idk why this is how it is
  if((status & IX_MODIFIED_B) > 0)
    ixChar = 'M';

  char tmp[256];
  tmp[0] = '\0';

  sprintf(tmp, "\e[31m%c\e[32m%c", wtChar, ixChar);
  strcat(statusString, tmp);
  return statusString;
}

void printLine(t_state * state, t_termLine * line)
{
  int invertNum = line->invertNum ? 7 : 0;
  int invertText = line->invertText ? 7 : 0;
  char * statusString;
  statusString = malloc(sizeof(char) * 255);
  statusString[0] = '\0';

  fprintf(state->tty,
  "\e[%dm%s%*d \e[%dm%s%-*.*s\e[0m %s\e[0m\n\r",
  invertNum,
  line->numEscCode,
  line->numPadding,
  line->lineNum,
  invertText,
  line->textEscCode,
  line->nameLength,
  line->nameLength,
  line->text,
  PrintStatus(statusString, line->gitStatus));

  free(statusString);
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
void draw(t_state * state)
{
  t_termLine * line = malloc(sizeof(t_termLine));
  t_theme * theme = state->theme;
  line->numPadding = (int)ceil(log10((double)*state->dirCount + 1));
  fprintf(state->tty, "\033[J");
  line->nameLength = state->config->nameLength;
  line->numEscCode = theme->numberLine;

  for(int i = getStart(state);
      i < getEnd(state);
      i++)
  {
    line->invertText = 0;
    line->invertNum = 0;
    line->textEscCode = theme->normal;

    line->lineNum = *state->selected - i > 0 ? *state->selected - i : (*state->selected - i) * -1;

    if(isCursorLine(state, i))
    {
      line->lineNum = i + 1;
      line->invertNum = 1;
    }

    if(access(state->fileAttribArray[i]->name, X_OK) != -1)
      line->textEscCode = theme->executable;

    switch(state->fileAttribArray[i]->fileMode) {
      case REGULAR:
        break;
      case FIFO:
        line->textEscCode = theme->pipe;
        break;
      case DIRECTORY:
        line->textEscCode = theme->directory;
        break;
      case EXACUTABLE:
      line->textEscCode = theme->executable;
        break;
    }

    line->invertText = isSelected(state, i);
    line->text = state->fileAttribArray[i]->name;
    line->gitStatus = state->fileAttribArray[i]->gitStatus;
    printLine(state, line);
  }
  fprintf(state->tty, "%s\n\r", state->msg);
  fprintf(state->tty, "\033[%dA", getEnd(state) - getStart(state) + 1);
  free(line);
}

int CheckFlag(t_state * state, char arg) {
  switch (arg) {
    case 'a':
      toggleHidden(state);
      break;
    case 'h':
      printf(
          "Minifm help\n\r"
          "     -a show hidden files\n\r"
          "     -h show this\n\r"
          "Controls:\n\r"
          "j            move down\n\r"
          "k            move up\n\r"
          "/            search\n\r"
          "enter        output file path\n\r"
          "escape       close program \n\r"
          "G            goto bottom\n\r"
          "gg           goto top\n\r"
          "b            output parent dir\n\r"
          "v            visual mode\n\r"
          "^H           toggle hidden\n\r"
          "dd           delete file\n\r"
          "yy           yank file(s)\n\r"
          "p            put file(s)\n\r"
          "V            select one\n\r"
          "o            swap visual position\n\r"
          );
      return 1;
      break;
    default:
      break;
  }
  return 0;
}

//TODO: remove trailing / if exists
int CheckArgs(t_state * state, int argc, char * argv[]) {
  int line;
  for(int i = 0; i < argc; i++)
  {
    if(*argv[i] == '-') {
      if(strlen(argv[i]) > 1) {
        for(int j = 1; j < strlen(argv[i]); j++) {
          if(sscanf(&argv[i][j], "%d", &line) == 1) {
            for(int k = 0; k < line - 1; k++)
              moveDown(state);
            break;
          }
          if(CheckFlag(state, argv[i][j]) == 1) {
            return 1;
          }
        }
      }
    } else {
      if(isDir(argv[i]))
      {
        state->cwd = argv[1];
        strcpy(state->cwd, argv[1]);
        *state->dirCount = countDir(state);
      }
    }
  }
  return 0;
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
  listQueue(commands, initAction(NORMAL, "\x8", toggleHidden));
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
