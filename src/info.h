#ifndef __INFO_H__
#define __INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

int countDir(char * path, int viewHidden);
void updateDirList(char * sel, char  ** bufferArray, int viewHidden);
int isDir(char * folder);
int getEnd(int selected, int total);
int getStart(int selected, int total);

#ifdef __cplusplus
}
#endif

#endif
