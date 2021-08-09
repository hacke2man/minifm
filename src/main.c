#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

void editFile(char * selected)
{
  system ("/bin/stty -raw");
  system ("/bin/stty echo");
  printf("\e[?25h");
  pid_t pid = fork();
  if(pid == 0)
  {
    execlp("/bin/nvim", "/bin/nvim", selected, NULL);
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

int isDir(char * folder)
{
  struct stat sb;

  if (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode)) {
    return 1;
  } else {
    return 0;
  }
}

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
  while (ent = readdir(dir)) {
    dirCount++;
  }
  closedir(dir);
  return dirCount;
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

  for(int i = 0; i < dircount; i++) {
    out[i] = malloc(sizeof(ent->d_name));
    strcpy(out[i], ent->d_name);
    ent = readdir(dir);
  }
  closedir(dir);
  chdir(sel);
}

int getStart(int selected, int total)
{
  if (selected > 5)
    return total - selected < 5 ? total - 10 : selected - 5;
  else
    return 0;
}

int getEnd(int selected, int total)
{
  if (total < 10 || selected + 5 > total)
    return total;
  else
    return selected < 5 ? 10 : selected + 5;
}

int main(int argc, char * argv[]) {
  int dirCount = countDir(argv[1]);
  char * out[dirCount];
  DIR * dir;
  changeDir(argv[1], out);

  system ("/bin/stty raw");
  system ("/bin/stty -echo");
  printf("\e[?25l");
  char lineNum[255];
  int selected = 0;
  char color_str[255] = {'\0'};
  int color;
  char chr;
  char path[PATH_MAX];
  path[0] = '\0';
  while(1){
    for(int i = getStart( selected,  dirCount);
    i < getEnd( selected,  dirCount);
    i++)
    {
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

      printf("\033[K%s %s%s\e[0m\n\r", lineNum, color_str, out[i]);
    }
    printf("\033[%dA", getEnd(selected, dirCount) - getStart(selected, dirCount));
    chr = getchar();
    switch(chr)
    {
      case 27:
        system ("/bin/stty -raw");
        system ("/bin/stty echo");
        printf("\e[?25h");
        printf("\033[%dA\033[J", dirCount);
        exit(0);
        break;
      case 'j':
        selected = selected < dirCount - 1 ? selected + 1 : selected;
        break;
      case 'k':
        selected = selected > 0 ? selected - 1 : selected;
        break;
        //FIXME: make .. work
      case '\r':
        if(isDir(out[selected]) && selected >= 0 && selected < dirCount)
        {
          char * sel = malloc(sizeof(out[selected]));
          strcpy(sel, out[selected]);
          selected = 0;

          for(int i = 0; i < dirCount; i++)
            free(out[i]);

          printf("\033[J");
          dirCount = countDir(sel);

          changeDir(sel, out);
        } else if (selected >= 0 && selected < dirCount)
          editFile(out[selected]);
        break;
      case 'b':
        for(int i = 0; i < dirCount; i++)
        free(out[i]);

        dirCount = countDir("..");
        changeDir("..", out);
        break;
    }
  }
}
