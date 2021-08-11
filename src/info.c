#include "info.h"
#include <stdio.h>

int getEnd(int selected, int total)
{
  if (total < 10 || selected + 5 > total)
    return total;
  else
    return selected < 5 ? 10 : selected + 5;
}

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

int countDir(char * path)
{
  static int count = 0;
  count++;
  DIR * dir;
  if(path == NULL || strlen(path) == 0)
    dir =opendir(".");
  else
    dir = opendir(path);

  struct dirent * ent;

  int dirCount = 0;
  while ((ent = readdir(dir))) {
    if(ent->d_name[0] != '.')
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
