#ifndef SFX_H
#define SFX_H

#define NUMSFX (32)

typedef struct
{
	u8* data;
	u32 size;
	CSND_ENCODING format;
	bool used;
}SFX_s;

void initSound(void);
void exitSound(void);
void initSFX(SFX_s* s);
void loadSFX(SFX_s* s, char* filename, CSND_ENCODING format);
SFX_s* createSFX(char* filename, CSND_ENCODING format);
void playSFX(SFX_s* s);

#endif
