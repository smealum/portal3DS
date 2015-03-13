#ifndef WALLDOOR_H
#define WALLDOOR_H

#include "gfx/md2.h"
#include "utils/math.h"
#include "game/room.h"
#include "game/portal.h"
#include "game/player.h"

typedef struct
{
	vect3Df_s position;
	vect3Di_s gridPosition;
	u8 orientation;
	// rectangle_s* walls;
	md2_instance_t modelInstance;
	material_s* frameMaterial;
	rectangle_s* rectangle;
	// elevator_s elevator;
	bool override;
	bool used;
}wallDoor_s;

extern wallDoor_s entryWallDoor;
extern wallDoor_s exitWallDoor;

void initWallDoors(void);
void freeWallDoors(void);
void updateWallDoors(player_s* pl);
void setupWallDoor(room_s* r, wallDoor_s* wd, vect3Di_s position, u8 orientation);
void drawWallDoors(void);

#endif
