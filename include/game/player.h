#ifndef PLAYER_H
#define PLAYER_H

#include "utils/math.h"
#include "game/pointphysics.h"
#include "game/camera.h"
#include "game/room.h"

#define PLAYER_RADIUS 2.0f

typedef struct
{
	physicalPoint_s object;
	camera_s camera;
}player_s;

void initPlayer(player_s* p);
void updatePlayer(player_s* p, room_s* r);

void movePlayer(player_s* p, vect3Df_s v);
void rotatePlayer(player_s* p, vect3Df_s v);

#endif
