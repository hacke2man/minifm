#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "info.h"
#include <signal.h>
#include <git2.h>

//callback for comparing file names
int compFunc(const void * a, const void * b)
{
  t_fileAttrib ** aval = (t_fileAttrib **) a;
  int ascore = 0;
  ascore -= (*aval)->fileMode * 10;

  t_fileAttrib ** bval = (t_fileAttrib **) b;
  int bscore = 0;
  bscore -= (*bval)->fileMode * 10;

  if(strcmp((*aval)->name, (*bval)->name) >= 0)
    ascore++;
  else
    bscore++;

  return ascore - bscore;
}

file_mode CheckMode(char * fileName) {
  struct stat st;
  stat(fileName, &st);

  if (S_ISREG(st.st_mode))
    return REGULAR;
  if (S_ISFIFO(st.st_mode))
    return FIFO;
  if (S_ISDIR(st.st_mode))
    return DIRECTORY;
  if (access(fileName, X_OK))
    return DIRECTORY;
  return -1;
}

//change proccess dir
//possible do this by having an update string array so it can keep
//track of everything on its own
void updateDirList(t_state * state)
{
  int viewHidden = state->config->viewHidden;
  t_fileAttrib ** fileAttribArray = state->fileAttribArray;
  char fileName[PATH_MAX];

  int dircount = countDir(state);
  DIR * dir;
  dir = opendir(state->cwd);
  if(!dir)
    exit(1);

  int tmp = *state->selected;
  free(state->selected);
  state->selected = malloc(sizeof(int) * dircount + 1);
  state->selected[0] = tmp;
  for(int i = 1; i <= countDir(state); i++)
    state->selected[i] = -1;

  struct dirent * ent;
  ent = readdir(dir);
  int ind = 0;
  while(ent != NULL){

    if(ent->d_name[0] != '.' || viewHidden)
    {
      free(fileAttribArray[ind]);
      fileAttribArray[ind] = malloc(sizeof(t_fileAttrib));
      fileAttribArray[ind]->name = malloc(sizeof(ent->d_name));
      memset(fileAttribArray[ind]->name, '\0', sizeof(ent->d_name));
      strcpy(fileAttribArray[ind]->name, ent->d_name);
      fileAttribArray[ind]->gitStatus = 0;
      sprintf(fileName, "%s/%s", state->cwd, fileAttribArray[ind]->name);
      fileAttribArray[ind]->fileMode = CheckMode(fileName);
      ind++;
    }
    ent = readdir(dir);
  }
  closedir(dir);
  qsort(fileAttribArray, dircount, sizeof(char *), compFunc);
}

//determin how many remaining files can be drawn
int getEnd(t_state * state)
{
  if (*state->dirCount < state->config->viewRange ||
  *state->selected + state->config->viewRange/2 > *state->dirCount)
    return *state->dirCount;
  else if(*state->selected < state->config->viewRange/2)
  {
    return state->config->viewRange;
  } else {
    return *state->selected + state->config->viewRange/2;
  }
}

//determin how many previous files can be drawn
int getStart(t_state * state)
{
  if (*state->selected > state->config->viewRange/2)
  {
    if (*state->dirCount - *state->selected >=  state->config->viewRange/2)
      return *state->selected - state->config->viewRange/2;
    if(*state->dirCount - state->config->viewRange < 0)
      return 0;
    else
      return *state->dirCount - state->config->viewRange;
  }
  else
    return 0;
}

#include <dirent.h>
#include <string.h>

//count files in dir
int countDir(t_state * state)
{
  int viewHidden = state->config->viewHidden;
  static int count = 0;
  count++;
  DIR * dir;
  dir = opendir(state->cwd);

  struct dirent * ent;

  int dirCount = 0;
  while ((ent = readdir(dir))) {
    if(ent->d_name[0] != '.' || viewHidden)
      dirCount++;
  }
  closedir(dir);
  return dirCount;
}

int isDir(char * folder)
{
  struct stat sb;

  if (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode)) {
    return 1;
  } else {
    return 0;
  }
}
