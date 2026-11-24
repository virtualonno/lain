#ifndef ABOUT_H
#define ABOUT_H

#include <sys/types.h>

void RunAboutScreen(void);
void LoadTimPartAt(int part, int x, int y, int clutX, int clutY);
void LoadTimPart(int part);
u_long *LoadTimPartClut(int part, int clutX, int clutY);

#endif
