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
int getEnd(int selected, int total)
{
  if (total < 10 || selected + 5 > total)
    return total;
  else
    return selected < 5 ? 10 : selected + 5;
}

//determin how many previous files can be drawn
//TODO: allow for custom view range
int getStart(int selected, int total)
{
  if (selected > 5)
  {
    if (total - selected >= 5) return selected - 5;
    {
      if(total - 10 < 0)
        return 0;
      else
        return total - 10;
    }
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
