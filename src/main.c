#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <termios.h>
#include <unistd.h>
#include <git2.h>

#include "info.h"
#include "core.h"

//TODO: convert bool nums to use true and false vars

//FIXME: stop from crashing when opening empty directory
//TODO: make system for processing arguments
int main(int argc, char * argv[]) {
  git_libgit2_init();

  t_state * state = malloc(sizeof(t_state));
  int c;
  char cwd[PATH_MAX];
  int dirCount;

  //TODO: make init function
  state->viewHidden = 0;
  dirCount = countDir(state);
  state->dirCount = &dirCount;
  state->tty = fopen("/dev/tty", "w");
  state->fileAttribArray = malloc(sizeof(t_fileAttrib) * 10000);
  state->cwd = cwd;
  state->selected = malloc(sizeof(int) * dirCount + 1);
  state->mode = NORMAL;
  state->topOfSelection = 1;
  state->viewRange = 12;
  state->msg = malloc(sizeof(char) * PATH_MAX);
  state->msg[0] = '\0';

  for(int i = 1; i < dirCount + 1; i++)
  {
    state->selected[i] = -1;
  }
  state->selected[0] = 0;

  getcwd(cwd, sizeof(cwd));
  tcgetattr( STDIN_FILENO, &state->oldt);
  cfmakeraw(&state->newt);
  tcsetattr( STDIN_FILENO, TCSANOW, &state->newt);
  fprintf(state->tty, "\e[?25l");

  struct actionNode * commands = initDefaultMappings();

  //#########
  //## git ##
  //#########
  //FIXME: make this work deep in git repo
  state->gitState = malloc(sizeof(t_gitState));

  state->gitState->repo = NULL;
  int error;

  state->gitState->repoRoot = GetRepoRoot();
  error = git_repository_open(&state->gitState->repo, state->gitState->repoRoot);

  if(error != 0 || strcmp(state->gitState->repoRoot, getenv("HOME")) == 0)
  {
    state->gitState->repoRoot = NULL;
  }

  git_status_options opts = GIT_STATUS_OPTIONS_INIT;
  if(state->gitState->repoRoot)
  {
    state->gitState->opts = &opts;
    char * cwdRootDiff = malloc(sizeof(char) * PATH_MAX);
    strcpy(cwdRootDiff, &state->cwd[strlen(state->gitState->repoRoot) + 1]);
    state->gitState->cwdRootDiff = cwdRootDiff;
  }

  updateDirList(state);

  //##################
  //## program loop ##
  //##################
  int done = 0;
  while(!done){
    draw(state);
    done = input(state, commands);
  }

  fprintf(state->tty, "\033[J");
  tcsetattr(STDIN_FILENO, TCSANOW, &state->oldt);
  fprintf(state->tty, "\e[?25h");

  git_repository_free(state->gitState->repo);
  free(state->gitState);
  free(state->msg);
  free(state->selected);
  free(state);

  git_libgit2_shutdown();
}
