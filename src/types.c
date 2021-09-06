#include "types.h"
#include <stdlib.h>

void freeFileAttrib(t_fileAttrib * fileAttrib)
{
  free(fileAttrib->name);
  free(fileAttrib);
}
