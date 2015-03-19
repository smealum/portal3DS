#ifndef PLAYER_H
#define PLAYER_H

#include "utils/math.h"
#include "game/pointphysics.h"
#include "game/camera.h"
#include "game/portal.h"
#include "game/room.h"
#include "gfx/md2.h"

#define PLAYER_RADIUS 1.2f

typedef enum
{
	PORTALGUN_IDLE = 0,
	PORTALGUN_SHOOT = 1,
	PORTALGUN_HOLD = 2,
}portalGunAnim_t;

typedef struct
{
	md2_instance_t gunInstance;
	physicalPoint_s object;
	camera_s camera;
	vect3Df_s tempAngle;
	float walkCnt1, walkCnt2;
	bool inPortal, oldInPortal;
	bool flying;
}player_s;

void playerInit(void);
void playerExit(void);

void initPlayer(player_s* p);
void updatePlayer(player_s* p, room_s* r);
void updatePlayerWalk(player_s* p, float wc1, float wc2);

void drawPlayerGun(player_s* p);

void movePlayer(player_s* p, vect3Df_s v);
void rotatePlayer(player_s* p, vect3Df_s v);

void shootPlayerGun(player_s* p, room_s* r, portal_s* portal);

#endif
