#ifndef CUBES_H
#define CUBES_H

#include "utils/math.h"
#include "game/room.h"
#include "gfx/md2.h"
#include "physics/physics.h"

#define NUMCUBEDISPENSERS (8)

typedef struct
{
	vect3Df_s position;
	md2_instance_t modelInstance;
	bool companion;
	bool active, oldActive;
	rectangle_s* openingRectangle;
	OBB_s* currentCube;
	bool used;
	u8 id;
}cubeDispenser_s;

void initCubes(void);
void exitCubes(void);
void drawCubeDispensers(void);
void updateCubeDispensers(void);
void resetCubeDispenserCube(cubeDispenser_s* cd);
cubeDispenser_s* createCubeDispenser(room_s* r, vect3Di_s pos, bool companion);

#endif