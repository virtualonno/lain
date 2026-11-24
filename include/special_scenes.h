#ifndef SPECIAL_SCENES_H
#define SPECIAL_SCENES_H

#include <sys/types.h>

extern u_char gSphereCollapseDone;

void InitEndRollSpheres(void);
void StepAndSortEndRollSpheres(int stepDelay);
void StepEndRollSpheresCollapse(void);
void InitGateScene(void);
void UpdateGateScene(void);

void RotateTimColumnsRight(void);

#endif
