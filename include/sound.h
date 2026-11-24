#ifndef SOUND_H
#define SOUND_H

void InitSound(int reset);
void SoundVSync(void);
void PlaySound(int program);
void StartSound(int program);
void StopSound(void);
void PlayMusic(void);
void FadeOutMusic(void);
void FadeInMusic(void);
void SelectMusic(int index);
void StopGridMusic(void);

#endif
