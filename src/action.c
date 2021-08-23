#include "action.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include "string.h"
#include "info.h"

// void draw(char ** out, int dirCount, int selected, FILE * tty)
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

void editFile(char * selected, FILE * tty, struct termios oldt)
{
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fprintf(tty, "\e[?25h");

  printf("echo\n -n\n");
  pid_t pid = fork();
  if(pid == 0)
  {
    printf("");
    execlp(getenv("EDITOR"), getenv("EDITOR"), selected, NULL);
    _exit(0);
  }else
  {
    int status;
    waitpid(pid, &status, 0);
    exit(0);
  }
}

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

int enter(t_state * state)
{
  char ** out = state->bufferArray;
  int * selected = state->selected;
  int * dirCount = state->dirCount;
  char * cwd = state->cwd;
  FILE * tty = state->tty;

  if(isDir(out[*selected]) && *selected >= 0 && *selected < *dirCount)
  {
    char * sel = malloc(sizeof(out[*selected]));
    strcpy(sel, out[*selected]);
    *selected = 0;

    for(int i = 0; i < *dirCount; i++)
      free(out[i]);

    fprintf(tty, "\033[J");
    strcat(cwd, "/");
    strcat(cwd, sel);
    // *dirCount = countDir(cwd);

    // tcsetattr(STDIN_FILENO, TCSANOW, &state->oldt);
    fprintf(tty, "\e[?25h");
    printf("%s", cwd);
    return 0;
    // changeDir(cwd, out);
  } else if (*selected >= 0 && *selected < *dirCount)
    editFile(out[*selected], tty, state->oldt);
    return 1;
}

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

// void Search(char ** out, int * dirCount, int * selected, char * cwd, FILE * tty)
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
      draw(state);
      break;
    }
    numMatch = 0;
  }
}
