#include <stdlib.h>
#include <stdio.h>
#include <3ds.h>
#include "gfx/texture.h"
#include "gfx/lodepng.h"

u8 tileOrder[] = {0,1,8,9,2,3,10,11,16,17,24,25,18,19,26,27,4,5,12,13,6,7,14,15,20,21,28,29,22,23,30,31,32,33,40,41,34,35,42,43,48,49,56,57,50,51,58,59,36,37,44,45,38,39,46,47,52,53,60,61,54,55,62,63};

texture_s textures[TEXTURES_NUM];

unsigned long htonl(unsigned long v)
{
	u8* v2=(u8*)&v;
	return (v2[0]<<24)|(v2[1]<<16)|(v2[2]<<8)|(v2[3]);
}
int totaltextures;

void textureInit()
{
	int i;
	for(i=0; i<TEXTURES_NUM; i++)
	{
		textures[i].used=false;
	}
	totaltextures=0;
}

void textureExit()
{
	int i;
	for(i=0; i<TEXTURES_NUM; i++)
	{
		if(textures[i].used)
		{
			textureFree(&textures[i]);
			textures[i].used=false;
		}
	}
}

texture_s* textureCreate(const char* fn, u32 param, int mipmap)
{
	if(!fn)return NULL;

	int i;
	for(i=0; i<TEXTURES_NUM; i++)
	{
		if(!textures[i].used)
		{
			if(!textureLoad(&textures[i], fn, param, mipmap))return &textures[i];
			else return NULL;
		}
	}
	return NULL;
}

texture_s* textureCreateBuffer(u32* buffer, int width, int height, u32 param, int mipmap)
{
	if(!buffer)return NULL;

	int i;
	for(i=0; i<TEXTURES_NUM; i++)
	{
		if(!textures[i].used)
		{
			if(!textureLoadBuffer(&textures[i], buffer, width, height, param, mipmap))return &textures[i];
			else return NULL;
		}
	}
	return NULL;
}

void tileImage8(u8* src, u8* dst, int width, int height)
{
	if(!src || !dst)return;

	int i, j, k, l;
	l=0;
	for(j=0; j<height; j+=8)
	{
		for(i=0; i<width; i+=8)
		{
			for(k=0; k<8*8; k++)
			{
				int x=i+tileOrder[k]%8;
				int y=j+(tileOrder[k]-(x-i))/8;
				u8 v=src[x+(height-1-y)*width];
				dst[l++]=v;
			}
		}
	}
}

void tileImage32(u32* src, u32* dst, int width, int height)
{
	if(!src || !dst)return;

	int i, j, k, l;
	l=0;
	for(j=0; j<height; j+=8)
	{
		for(i=0; i<width; i+=8)
		{
			for(k=0; k<8*8; k++)
			{
				int x=i+tileOrder[k]%8;
				int y=j+(tileOrder[k]-(x-i))/8;
				u32 v=src[x+(height-1-y)*width];
				dst[l++]=htonl(v);
			}
		}
	}
}

// downscales by a factor of 2 on width and 2 on height
void downscaleImage(u8* data, int width, int height)
{
	if(!data || !width || !height)return;

	int i, j;
	for(j=0; j<height; j++)
	{
		for(i=0; i<width; i++)
		{
			const u32 offset = (i+j*width)*4;
			const u32 offset2 = (i*2+j*2*width*2)*4;
			data[offset+0] = (data[offset2+0+0] + data[offset2+4+0] + data[offset2+width*4*2+0] + data[offset2+(width*2+1)*4+0]) / 4;
			data[offset+1] = (data[offset2+0+1] + data[offset2+4+1] + data[offset2+width*4*2+1] + data[offset2+(width*2+1)*4+1]) / 4;
			data[offset+2] = (data[offset2+0+2] + data[offset2+4+2] + data[offset2+width*4*2+2] + data[offset2+(width*2+1)*4+2]) / 4;
			data[offset+3] = (data[offset2+0+3] + data[offset2+4+3] + data[offset2+width*4*2+3] + data[offset2+(width*2+1)*4+3]) / 4;
		}
	}
}

// downscales by a factor of 2 on width and 2 on height
void downscaleImage8(u8* data, int width, int height)
{
	if(!data || !width || !height)return;

	int i, j;
	for(j=0; j<height; j++)
	{
		for(i=0; i<width; i++)
		{
			const u32 offset = (i+j*width);
			const u32 offset2 = (i*2+j*2*width*2);
			data[offset] = (data[offset2] + data[offset2+4] + data[offset2+width*2] + data[offset2+(width*2+1)]) / 4;
		}
	}
}

int textureLoad(texture_s* t, const char* fn, u32 param, int mipmap)
{
	if(!t || !fn || t->used)return -1;
	if(mipmap < 0)return -1;

			totaltextures++;

	t->data=NULL;
	t->filename=NULL;
	t->format=GPU_RGBA8;

	u32* buffer;
	unsigned int error=lodepng_decode32_file((unsigned char**)&buffer, (unsigned int*)&t->width, (unsigned int*)&t->height, fn);
	if(error){printf("error %u: %s\n", error, lodepng_error_text(error)); return -2;}

	int l=0; for(l=0; !(t->height&(1<<l)) && !(t->width&(1<<l)); l++);

	if(mipmap>l)mipmap=l;

	u32 size = 4*t->width*t->height;
	size = ((size - (size >> (2*(mipmap+1)))) * 4) / 3; //geometric progression
	t->data=linearMemAlign(size, 0x80); //textures need to be 0x80 byte aligned
	if(!t->data){free(buffer); return -3;}

	tileImage32(buffer, t->data, t->width, t->height);

	u32 offset = t->width*t->height;
	int level = 0;
	int w = t->width/2, h = t->height/2;
	for(level = 0; level < mipmap; level++)
	{
		downscaleImage((u8*)buffer, w, h);
		tileImage32(buffer, &t->data[offset], w, h);
		offset += w*h;
		w /= 2; h /= 2;
	}

	free(buffer);
	
	t->param=param;
	t->filename=malloc(strlen(fn)+1);
	if(t->filename)strcpy(t->filename, fn);

	t->mipmap=mipmap;
	t->used=true;

	printf("successfully read texture\n");

	return 0;
}

int textureLoadBuffer(texture_s* t, u32* buffer, int width, int height, u32 param, int mipmap)
{
	if(!buffer || !t || t->used)return -1;
	if(mipmap < 0)return -1;

			totaltextures++;

	t->data=NULL;
	t->filename=NULL;

	t->width = width;
	t->height = height;

	t->format = GPU_A8;

	int l=0; for(l=0; !(t->height&(1<<l)) && !(t->width&(1<<l)); l++);

	if(mipmap>l)mipmap=l;

	u32 size = t->width*t->height;
	size = ((size - (size >> (2*(mipmap+1)))) * 4) / 3; //geometric progression
	// t->data=linearMemAlign(size, 0x80); //textures need to be 0x80 byte aligned
	t->data=linearMemAlign(size, 0x1000); //textures need to be 0x80 byte aligned (apparently not enough, likely depends on texture width, TODO : figure out alignment requirement)
	if(!t->data){return -3;}
	tileImage8(buffer, t->data, t->width, t->height);

	u32 offset = t->width*t->height;
	int level = 0;
	int w = t->width/2, h = t->height/2;
	for(level = 0; level < mipmap; level++)
	{
		downscaleImage8((u8*)buffer, w, h);
		tileImage8(buffer, &t->data[offset], w, h);
		offset += w*h;
		w /= 2; h /= 2;
	}
	
	t->param=param;

	t->mipmap=mipmap;
	t->used=true;

	printf("successfully made texture\n");

	return 0;
}

void textureBind(texture_s* t, GPU_TEXUNIT unit)
{
	if(!t)return;

	GPU_SetTexture(unit, (u32*)osConvertVirtToPhys((u32)t->data), t->width, t->height, t->param, t->format);
	GPUCMD_AddWrite(GPUREG_TEXUNIT0_LOD, t->mipmap<<16);
}

void textureFree(texture_s* t)
{
	if(!t || !t->data)return;

	totaltextures--;

	linearFree(t->data);
	t->data = NULL;
	t->used = false;
}
