#ifndef LIGHTING_H
#define LIGHTING_H

#include "utils/math.h"
#include "gfx/texture.h"

typedef enum
{
	LIGHTMAP_DATA,
	VERTEXLIGHT_DATA
}lightingData_t;

typedef struct
{
	vect3Di_s lmPos, lmSize;
	bool rot;
}lightMapCoordinates_s;

typedef struct
{
	vect3Di_s lmSize;
	u8* buffer;
	lightMapCoordinates_s* coords;
	int num;
	texture_s* texture;
}lightMapData_s;

typedef struct
{
	u8 width, height;
	u8* values;
}vertexLightingData_s;

typedef struct
{
	lightingData_t type;
	union{
		lightMapData_s lightMap;
		vertexLightingData_s* vertexLighting;
	}data;
	u16 size;
}lightingData_s;

void initLightData(lightingData_s* ld);
void freeLightData(lightingData_s* ld);

void initLightDataLM(lightingData_s* ld, u16 n);
void initLightDataVL(lightingData_s* ld, u16 n);

#endif
