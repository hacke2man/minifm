#ifndef __ACTIONPLUMBING_H__
#define __ACTIONPLUMBING_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "types.h"

int matchScore(char * search, char * check);
t_action * initAction(mode mode, char * combo, int (*function)(t_state *) );
int matchScore(char * search, char * check);
int Search(t_state * state);
void listQueue(struct actionNode * commands, t_action * action);
struct actionNode * initList(t_action * action);

#ifdef __cplusplus
}
#endif

#endif
