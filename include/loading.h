#ifndef LOADING_H
#define LOADING_H

extern short gChromaQuant;
extern short gLumaQuant;

int RunP2Screen(void);
int StepTimLoadSequence(int discardPart14);
void LoadLoadingImage(void);
void LoadAltLoadingImage(void);
int DrawLoadingFrame(int resetFrame);

#endif
