#ifndef DOOR_H
#define DOOR_H

#include "gfx/md2.h"
#include "utils/math.h"
#include "game/room.h"
#include "game/activator.h"
#include "game/sfx.h"

#define NUMDOORS (16)

typedef struct
{
	activatableObject_s ao;
	vect3Df_s position;
	md2_instance_t modelInstance;
	rectangle_s* rectangle[2];
	bool orientation;
	bool used;
	u8 id;
}door_s;

extern SFX_s* doorOpenSFX;
extern SFX_s* doorCloseSFX;

void initDoors(void);
void exitDoors(void);
door_s* createDoor(room_s* r, vect3Di_s position, bool orientation);
void updateDoors(void);
void drawDoors(void);

#endif
