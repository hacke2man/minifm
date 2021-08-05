#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

//TODO: use array
char * listDir(char * path)
{
  char * out = malloc(sizeof(char) * 10000);
  DIR * dir;
  if(path == NULL || strlen(path) == 0)
    dir =opendir(".");
  else
    dir = opendir(path);

  if(dir == NULL)
  {
    strcat(out, path);
    strcat(out, " cannot be opened");
    return out;
  }

  struct dirent * ent;
  ent = readdir(dir);
  while (ent != NULL) {
    strcat(out, ent->d_name);
    strcat(out, "\n");
    ent = readdir(dir);
  }

  closedir(dir);
  return out;
}

int main(int argc, char * argv[]) {
  printf("%s", listDir(argv[1]));
  return 0;
}
