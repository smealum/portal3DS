#include <stdlib.h>
#include <3ds.h>
#include "game/light.h"

void initLightData(lightingData_s* ld)
{
	if(!ld)return;

	ld->type=LIGHTMAP_DATA;
	ld->size=0;
	ld->data.lightMap.buffer=NULL;
	ld->data.lightMap.coords=NULL;
}

void freeLightData(lightingData_s* ld)
{
	if(!ld)return;

	switch(ld->type)
	{
		case LIGHTMAP_DATA:
			if(ld->data.lightMap.coords)free(ld->data.lightMap.coords);
			if(ld->data.lightMap.buffer)free(ld->data.lightMap.buffer);
			ld->data.lightMap.buffer=NULL;
			ld->data.lightMap.coords=NULL;
			if(ld->data.lightMap.texture)textureFree(ld->data.lightMap.texture);
			ld->size=0;
			break;
		case VERTEXLIGHT_DATA:
			if(ld->data.vertexLighting)
			{
				int i;
				for(i=0;i<ld->size;i++)if(ld->data.vertexLighting[i].values){free(ld->data.vertexLighting[i].values);ld->data.vertexLighting[i].values=NULL;}
				free(ld->data.vertexLighting);
				ld->data.vertexLighting=NULL;
			}
			break;
	}
}

//LIGHTMAP

void initLightDataLM(lightingData_s* ld, u16 n)
{
	if(!ld)return;

	ld->type=LIGHTMAP_DATA;
	ld->size=n;
	ld->data.lightMap.lmSize=vect3Di(0,0,0);
	ld->data.lightMap.buffer=NULL;
	ld->data.lightMap.coords=(lightMapCoordinates_s*)malloc(sizeof(lightMapCoordinates_s)*n);
	ld->data.lightMap.texture=NULL;
}

// VERTEX LIGHTING

void initLightDataVL(lightingData_s* ld, u16 n)
{
	if(!ld)return;

	ld->type=VERTEXLIGHT_DATA;
	ld->size=n;
	ld->data.vertexLighting=(vertexLightingData_s*)malloc(sizeof(vertexLightingData_s)*n);
}

