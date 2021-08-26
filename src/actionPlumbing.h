#ifndef __ACTIONPLUMBING_H__
#define __ACTIONPLUMBING_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "types.h"

t_action * initAction(char * combo, int (*function)(t_state *) );
int matchScore(char * search, char * check);
int Search(t_state * state);
int canMatch(char * combo, struct actionNode * head);
void listQueue(struct actionNode * commands, t_action * action);
struct actionNode * initList(t_action * action);

#ifdef __cplusplus
}
#endif

#endif
