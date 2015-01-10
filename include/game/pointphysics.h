#ifndef PHYSICS_H
#define PHYSICS_H

#include "utils/math.h"

typedef struct
{
	vect3Df_s position, speed;
	float radius, sqRadius;
	bool contact;
}physicalPoint_s;

#endif
