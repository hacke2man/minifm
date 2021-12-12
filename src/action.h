#ifndef __ACTION_H__
#define __ACTION_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include "types.h"

int toggleHidden(t_state * state);
int Search(t_state * state);
int enter(t_state * state);
int moveDown(t_state * state);
int moveUp(t_state * state);
int escape(t_state * state);
void freeAction(t_action * action);
int gotoTop(t_state * state);
int gotoBottom(t_state * state);
int backDir(t_state * state);
int removeFile(t_state * state);
int yank(t_state * state);
int put(t_state * state);
int Visual(t_state * state);
int enterVisual(t_state * state);
int visualMoveDown(t_state * state);
int visualMoveUp(t_state * state);
int changeSelectionPos(t_state * state);
int printSelected(t_state * state);
int selectOne(t_state * state);
int halfPageUp(t_state * state);
int halfPageDown(t_state * state);
int MkDir(t_state * state);
int MkFile(t_state * state);

#ifdef __cplusplus
}
#endif

#endif
