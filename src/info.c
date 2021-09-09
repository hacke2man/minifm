#include <dirent.h>
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
  if(isDir((*aval)->name))
    ascore -= 10 ;

  t_fileAttrib ** bval = (t_fileAttrib **) b;
  int bscore = 0;
  if(isDir((*bval)->name))
    bscore -= 10;

  if(strcmp((*aval)->name, (*bval)->name) >= 0)
    ascore++;
  else
    bscore++;

  return ascore - bscore;
}

void SetFileGitStatus(t_state * state, const char * fileName, unsigned int status)
{
  char * tmp = strchr(fileName, '/');
  int inDir = false;
  if(*tmp == '/')
  {
    inDir = true;
    *tmp = '\0';
  }

  int i = 0;
  for(; i < *state->dirCount; i++)
  {
    if(strcmp(fileName, state->fileAttribArray[i]->name) == 0)
      state->fileAttribArray[i]->gitStatus = status | state->fileAttribArray[i]->gitStatus;
  }

  if(inDir)
    *tmp = '/';
}

//FIXME: make this work deep in git repo
//FIXME: modify dir status, by its files
void SetGitStatus(t_state * state)
{
  const git_status_entry * entry;
  size_t count = git_status_list_entrycount(state->gitState->statuses);

  for (size_t i=0; i<count; ++i) {
    entry = git_status_byindex(state->gitState->statuses, i);
    SetFileGitStatus(state, entry->index_to_workdir->new_file.path, entry->status);
  }
}

//change proccess dir
//TODO: notice deleted/added files after process is launched
//possible do this by having an update string array so it can keep
//track of everything on its own
void updateDirList(t_state * state)
{
  int viewHidden = state->viewHidden;
  t_fileAttrib ** fileAttribArray = state->fileAttribArray;

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
      free(fileAttribArray[ind]);
      fileAttribArray[ind] = malloc(sizeof(t_fileAttrib));
      fileAttribArray[ind]->name = malloc(sizeof(ent->d_name));
      memset(fileAttribArray[ind]->name, '\0', sizeof(ent->d_name));
      strcpy(fileAttribArray[ind]->name, ent->d_name);
      fileAttribArray[ind]->gitStatus = 0;
      ind++;
    }
    ent = readdir(dir);
  }
  closedir(dir);
  qsort(fileAttribArray, dircount, sizeof(char *), compFunc);

  git_status_options opts = GIT_STATUS_OPTIONS_INIT;
  state->gitState->opts = &opts;
  state->gitState->statuses = NULL;
  int error = git_status_list_new(&state->gitState->statuses, state->gitState->repo, &opts);
  if(error != 0)
  {
    printf("no status list\n");
    getchar();
  }
  SetGitStatus(state);
}

//determin how many remaining files can be drawn
int getEnd(t_state * state)
{
  if (*state->dirCount < state->viewRange ||
  *state->selected + state->viewRange/2 > *state->dirCount)
    return *state->dirCount;
  else if(*state->selected < state->viewRange/2)
  {
    return state->viewRange;
  } else {
    return *state->selected + state->viewRange/2;
  }
}

//determin how many previous files can be drawn
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
