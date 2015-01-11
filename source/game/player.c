#include <3ds.h>
#include "game/player.h"

void initPlayer(player_s* p)
{
	if(!p)return;

	initPhysicalPoint(&p->object, vect3Df(0,0,0), PLAYER_RADIUS);
	initCamera(&p->camera);
}

void updatePlayer(player_s* p, room_s* r)
{
	if(!p)return;

	collideObjectRoom(&p->object, r);
	updateCamera(&p->camera);

	p->camera.position = p->object.position;
	
	p->object.speed = vect3Df(0,0,0); //TEMP
}

void movePlayer(player_s* p, vect3Df_s v)
{
	if(!p)return;

	p->object.speed = vaddf(p->object.speed, moveCameraVector(&p->camera, v));
}

void rotatePlayer(player_s* p, vect3Df_s v)
{
	if(!p)return;

	rotateCamera(&p->camera, v);
}
