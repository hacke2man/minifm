#include "action.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include "string.h"
#include "info.h"

void editFile(char * selected)
{
  system ("/bin/stty -raw");
  system ("/bin/stty echo");
  printf("\e[?25h");
  pid_t pid = fork();
  if(pid == 0)
  {
    execlp(getenv("EDITOR"), getenv("EDITOR"), selected, NULL);
    _exit(0);
  }else
  {
    int status;
    waitpid(pid, &status, 0);
  }
  printf("\e[?25l");
  system ("/bin/stty raw");
  system ("/bin/stty -echo");
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
