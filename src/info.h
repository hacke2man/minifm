#ifndef __INFO_H__
#define __INFO_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "types.h"

int countDir(t_state * state);
void updateDirList(t_state * state);
int isDir(char * folder);
int getEnd(int selected, int total);
int getStart(int selected, int total);

#ifdef __cplusplus
}
#endif

#endif
