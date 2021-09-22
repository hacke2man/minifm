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

/*FIXME: make all actions work while in an empty directory
unchecked actions: enterVisual, printSelected,
visualMoveDown, visualMoveUp, changeSelectionPos, selectOne, Search
moveDown, moveUp, halfPageDown, halfPageUp, freeAction
gotoTop, gotoBottom, backDir, yank, put */

int main(int argc, char * argv[]) {
  git_libgit2_init();

  t_config * config = DefaultConfig();
  t_theme * theme = DefaultTheme();
  t_state * state = InitState(config, theme);

  // git setup
  state->gitState = malloc(sizeof(t_gitState));
  state->gitState->repo = NULL;
  state->gitState->repoRoot = GetRepoRoot();
  int error = git_repository_open(&state->gitState->repo, state->gitState->repoRoot);
  git_status_options opts = GIT_STATUS_OPTIONS_INIT;
  GitInit(state, error, &opts);

  struct actionNode * commands = initDefaultMappings();
  CheckArgs(state, argc, argv);
  updateDirList(state);

  // program loop
  int done = 0;
  while(!done){
    draw(state);
    done = input(state, commands);
  }

  FreeState(state);
  git_libgit2_shutdown();
}
