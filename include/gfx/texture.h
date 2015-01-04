#ifndef TEXTURE_H
#define TEXTURE_H

#include <3ds.h>

typedef struct
{
	u32 width, height;
	u32 param;
	u32* data;
}texture_s;

int textureLoad(texture_s* t, const char* fn, u32 param);
void textureBind(texture_s* t, GPU_TEXUNIT unit);
void textureFree(texture_s* t);

#endif
