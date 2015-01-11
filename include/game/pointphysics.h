#ifndef PHYSICS_H
#define PHYSICS_H

#include "utils/math.h"
#include "game/room.h"

typedef struct
{
	vect3Df_s position, speed;
	float radius, sqRadius;
	bool contact;
}physicalPoint_s;

void initPhysicalPoint(physicalPoint_s* pp, vect3Df_s position, float radius);
void updatePhysicalPoint(physicalPoint_s* pp);
void collideObjectRoom(physicalPoint_s* pp, room_s* r);

#endif
