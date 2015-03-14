#ifndef LIGHTMAP_H
#define LIGHTMAP_H

#include "light_room.h"

typedef struct
{
	vect3Df_s lmPos, lmSize;
	bool rot;
}lightMapCoordinates_s;

typedef struct
{
	vect3Di_s lmSize;
	u8* buffer;
	lightMapCoordinates_s* coords;
	int num;
}lightMapData_s;

#endif
