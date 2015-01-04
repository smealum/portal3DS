#include <stdlib.h>
#include <stdio.h>
#include <3ds.h>
#include "gfx/texture.h"
#include "gfx/lodepng.h"

u8 tileOrder[] = {0,1,8,9,2,3,10,11,16,17,24,25,18,19,26,27,4,5,12,13,6,7,14,15,20,21,28,29,22,23,30,31,32,33,40,41,34,35,42,43,48,49,56,57,50,51,58,59,36,37,44,45,38,39,46,47,52,53,60,61,54,55,62,63};

unsigned long htonl(unsigned long v)
{
	u8* v2=(u8*)&v;
	return (v2[0]<<24)|(v2[1]<<16)|(v2[2]<<8)|(v2[3]);
}

int textureLoad(texture_s* t, const char* fn, u32 param)
{
	if(!t || !fn)return -1;

	t->data=NULL;

	u32* buffer;
	unsigned int error=lodepng_decode32_file((unsigned char**)&buffer, (unsigned int*)&t->width, (unsigned int*)&t->height, fn);
	if(error){printf("error %u: %s\n", error, lodepng_error_text(error)); return -2;}

	t->data=linearMemAlign(4*t->width*t->height, 0x80); //textures need to be 0x80 byte aligned
	if(!t->data){free(buffer); return -3;}

	int i, j, k, l;
	l=0;
	for(j=0; j<t->height; j+=8)
	{
		for(i=0; i<t->width; i+=8)
		{
			for(k=0; k<8*8; k++)
			{
				int x=i+tileOrder[k]%8;
				int y=j+(tileOrder[k]-(x-i))/8;
				u32 v=buffer[x+(t->height-1-y)*t->width];
				t->data[l++]=htonl(v);
			}
		}
	}

	free(buffer);
	
	t->param=param;

	return 0;
}

void textureBind(texture_s* t, GPU_TEXUNIT unit)
{
	if(!t)return;

	GPU_SetTexture(unit, (u32*)osConvertVirtToPhys((u32)t->data),t->height,t->width,t->param,GPU_RGBA8);
}

void textureFree(texture_s* t)
{
	if(!t || !t->data)return;

	linearFree(t->data);
}
