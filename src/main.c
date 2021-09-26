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
  t_config * config = DefaultConfig();
  t_theme * theme = DefaultTheme();
  t_state * state = InitState(config, theme);

  struct actionNode * commands = initDefaultMappings();
  updateDirList(state);
  int done;
  done = CheckArgs(state, argc, argv);

  // program loop
  while(!done){
    draw(state);
    done = input(state, commands);
  }

  FreeState(state);
}
