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

//TODO: make system for processing arguments
int main(int argc, char * argv[]) {
  git_libgit2_init();

  t_state * state = malloc(sizeof(t_state));
  int c;
  char cwd[PATH_MAX];
  int dirCount;

  //git
  state->gitState = malloc(sizeof(t_gitState));

  state->gitState->repo = NULL;
  int error = git_repository_open(&state->gitState->repo, ".");
  if(error != 0)
  {
    printf("no repo\n");
    getchar();
  }

  git_status_options opts = GIT_STATUS_OPTIONS_INIT;
  state->gitState->opts = &opts;

  state->gitState->statuses = NULL;
  error = git_status_list_new(&state->gitState->statuses, state->gitState->repo, &opts);
  if(error != 0)
  {
    printf("no status list\n");
    getchar();
  }

  /* git_index *index = NULL;
  error = git_repository_index(&index, repo);
  if(error != 0)
  {
    printf("welp");
    getchar();
  } */

  /* error = git_index_open(&index, ".");
  state->gitState->index = index; */

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
  updateDirList(state);

  struct actionNode * commands = initDefaultMappings();

  //program loop
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
