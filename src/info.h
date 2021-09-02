#ifndef __INFO_H__
#define __INFO_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "types.h"

int countDir(t_state * state);
void updateDirList(t_state * state);
int isDir(char * folder);
int getEnd(t_state * state);
int getStart(t_state * state);

#ifdef __cplusplus
}
#endif

#endif
