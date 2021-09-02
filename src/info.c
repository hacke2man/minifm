#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "info.h"

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
//TODO: notice deleted/added files after process is launched
//possible do this by having an update string array so it can keep
//track of everything on its own
void updateDirList(t_state * state)
{
  int viewHidden = state->viewHidden;
  char ** bufferArray = state->bufferArray;

  int dircount = countDir(state);
  DIR * dir;
  dir = opendir(state->cwd);
  if(!dir)
    exit(1);

  dircount = countDir(state);

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
      bufferArray[ind] = malloc(sizeof(ent->d_name));
      memset(bufferArray[ind], '\0', sizeof(ent->d_name));
      strcpy(bufferArray[ind], ent->d_name);
      ind++;
    }
    ent = readdir(dir);
  }
  closedir(dir);
  qsort(bufferArray, dircount, sizeof(char *), compFunc);
}

//determin how many remaining files can be drawn
//TODO: allow for custom view range
int getEnd(t_state * state)
{
  if (*state->dirCount < state->viewRange ||
  *state->selected + state->viewRange/2 > *state->dirCount)
    return *state->dirCount;
  else
    return *state->selected < state->viewRange/2 ?
    state->viewRange
    :
    *state->selected + state->viewRange/2;
}

//determin how many previous files can be drawn
//TODO: allow for custom view range
int getStart(t_state * state)
{
  if (*state->selected > state->viewRange/2)
  {
    if (*state->dirCount - *state->selected >=  state->viewRange/2)
      return *state->selected - state->viewRange/2;
    if(*state->dirCount - state->viewRange < 0)
      return 0;
    else
      return *state->dirCount - state->viewRange;
  }
  else
    return 0;
}

#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

//count files in dir
int countDir(t_state * state)
{
  int viewHidden = state->viewHidden;
  static int count = 0;
  count++;
  DIR * dir;
  dir = opendir(".");

  struct dirent * ent;

  int dirCount = 0;
  while ((ent = readdir(dir))) {
    if(ent->d_name[0] != '.' || viewHidden)
    // if(ent->d_name[0] != '.' || viewHidden)
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
