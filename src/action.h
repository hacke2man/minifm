#ifndef __ACTION_H__
#define __ACTION_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include "types.h"

struct actionNode * initDefaultMappings();
int enter(t_state * state);
void draw(t_state * state);
int input(t_state * state, struct actionNode * commands);

#ifdef __cplusplus
}
#endif

#endif
