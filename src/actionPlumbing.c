#include <string.h>
#include "actionPlumbing.h"
#include <stdlib.h>

//assings a score based on how many letters match in strings
int matchScore(char * search, char * check)
{
  int score = 0;
  for(int i = 0; i < strlen(check); i++)
  {
    if(search[i] == check[i])
    {
      score++;
    } else {
      break;
    }
  }
  return score;
}

t_action * initAction(char * combo, int (*function)(t_state *) )
{
  t_action * newAction;

  newAction = malloc(sizeof(t_action));
  newAction->combo = malloc(sizeof(int));
  newAction->combo = combo;
  int (*fp)(t_state * state) = function;
  newAction->function = malloc(sizeof(&function));
  newAction->function = function;
  return newAction;
}

int canMatch(char * combo, struct actionNode * head)
{
  int matchable = 0;
  while (head)
  {
    if(strlen(combo) <= strlen(head->action->combo))
    {
      for(int j = 0; j < strlen(combo); j++)
      {
        if (combo[j] != head->action->combo[j]) 
          break;
        if(j == strlen(combo) - 1)
          matchable = 1;
      }
    }
    head = head->nextNode;
  }
  return matchable;
}

void listQueue(struct actionNode * commands, t_action * action)
{
  commands->tail->nextNode = malloc(sizeof(struct actionNode));
  commands->tail = commands->tail->nextNode;
  commands->tail->action = action;
  commands->tail->nextNode = NULL;
}

struct actionNode * initList(t_action * action)
{
  struct actionNode * commands = malloc(sizeof(struct actionNode));
  commands->action = malloc(sizeof(t_action));
  commands->action = action;
  commands->tail = commands;
  commands->nextNode = NULL;
  return commands;
}

