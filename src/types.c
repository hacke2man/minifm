#include "types.h"
#include <limits.h>
#include <stdlib.h>
#include "info.h"

void freeFileAttrib(t_fileAttrib * fileAttrib)
{
  free(fileAttrib->name);
  free(fileAttrib);
}

t_theme * DefaultTheme() {
  t_theme * theme = malloc(sizeof(t_theme));
  theme->normal = "\033[49;39m";
  theme->numberLine = "\033[49;90m";
  theme->directory = "\033[49;34m";
  theme->gitWarn = "\033[49;31m";
  theme->gitAdd = "\033[49;32m";
  theme->executable = "\033[49;92m";
  theme->pipe = "\033[49;33m";
  return theme;
}

t_config * DefaultConfig() {
  t_config * config = malloc(sizeof(t_config));

  config->viewHidden = 0;
  config->viewRange = 12;
  config->nameLength = 15;

  return config;
}

#include <unistd.h>
#include <string.h>
char * GetRepoRoot()
{
  char cwd[PATH_MAX];
  char tmpPath[PATH_MAX];
  strcpy(cwd, getenv("PWD"));
  int error = 1;

  while(error)
  {
    sprintf(tmpPath, "%s/.git", cwd);
    if( access( tmpPath, F_OK ) == 0 ) {
      error = 0;
    } else {
      error = 1;
    }

    if(error)
    *strrchr(cwd, '/') = '\0';
  }

  if(strcmp(getenv("HOME"), cwd) == 0)
    cwd[0] = '\0';

  char * outString = malloc(sizeof(char) * strlen(cwd));
  strcpy(outString, cwd);
  return outString;
}

void SetSelected(t_state * state) {
  for(int i = 1; i < *state->dirCount + 1; i++)
  {
    state->selected[i] = -1;
  }
  state->selected[0] = 0;
}

t_state * InitState(t_config * config, t_theme * theme) {
  t_state * state = malloc(sizeof(t_state));
  int * dirCount = malloc(sizeof(int));
  state->theme = theme;
  state->config = config;

  state->cwd = malloc(sizeof(char) * PATH_MAX);
  getcwd(state->cwd, sizeof(char) * PATH_MAX);
  *dirCount = countDir(state);
  state->dirCount = dirCount;
  state->tty = fopen("/dev/tty", "w");
  state->fileAttribArray = malloc(sizeof(t_fileAttrib) * 10000);
  state->selected = malloc(sizeof(int) * *dirCount + 1);
  state->mode = NORMAL;
  state->topOfSelection = 1;
  state->msg = malloc(sizeof(char) * PATH_MAX);
  state->msg[0] = '\0';
  state->flags = 0;
  SetSelected(state);

  tcgetattr( STDIN_FILENO, &state->oldt);
  cfmakeraw(&state->newt);
  tcsetattr( STDIN_FILENO, TCSANOW, &state->newt);
  fprintf(state->tty, "\e[?25l");

  return state;
}

void FreeState(t_state * state) {
  fprintf(state->tty, "\033[J");
  tcsetattr(STDIN_FILENO, TCSANOW, &state->oldt);
  fprintf(state->tty, "\e[?25h");

  // HACK: crashes when i free why???
  // free(state->cwd);
  free(state->dirCount);
  free(state->config);
  free(state->msg);
  free(state->selected);
  free(state);
}
