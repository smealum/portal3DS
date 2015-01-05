#ifndef TEXTURE_H
#define TEXTURE_H

#include <3ds.h>

#define TEXTURES_NUM (256)

typedef struct
{
	char* filename;
	u32 width, height;
	u32 param;
	u32* data;
	bool used;
}texture_s;


void textureInit();
void textureExit();

texture_s* textureCreate(const char* fn, u32 param);
int textureLoad(texture_s* t, const char* fn, u32 param);
void textureBind(texture_s* t, GPU_TEXUNIT unit);
void textureFree(texture_s* t);

#endif
