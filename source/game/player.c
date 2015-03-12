#include <3ds.h>
#include <string.h>
#include "gfx/gs.h"
#include "gfx/texture.h"
#include "game/player.h"

md2_model_t gunModel;
texture_s gunTexture;

void playerInit(void)
{
	textureLoad(&gunTexture, "portalgun.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	md2ReadModel(&gunModel, "portalgun.md2");
}

void playerExit(void)
{

}

void initPlayer(player_s* p)
{
	if(!p)return;

	initPhysicalPoint(&p->object, vect3Df(0,0,0), PLAYER_RADIUS);
	initCamera(&p->camera);
	md2InstanceInit(&p->gunInstance, &gunModel, &gunTexture);

	p->oldInPortal = p->inPortal = false;
}

void warpPlayer(portal_s* p, player_s* pl)
{
	if(!p || !pl)return;
	camera_s* c = &pl->camera;

	camera_s new_camera = *c;

	float tmp1[4*4], tmp2[4*4];
	transposeMatrix44(p->target->matrix, tmp1);
	new_camera.position = vaddf(p->target->position, warpPortalVector(p, vsubf(c->position, p->position)));
	multMatrix44((float*)new_camera.orientation, p->matrix, tmp2);
	rotateMatrixY(tmp1, M_PI, true);
	multMatrix44(tmp2, tmp1, (float*)new_camera.orientation);

	memcpy(new_camera.modelview, new_camera.orientation, sizeof(mtx44));
	translateMatrix((float*)new_camera.modelview, -new_camera.position.x, -new_camera.position.y, -new_camera.position.z);

	pl->object.position = new_camera.position;
	pl->object.speed = warpPortalVector(p, pl->object.speed);
	*c = new_camera;
}

void checkPortalPlayerWarp(player_s* pl, portal_s* p)
{
	if(!pl || !p)return;
	vect3Df_s v;
	float x, y, z;
	bool r=isPointInPortal(p, pl->object.position, &v, &x, &y, &z);
	if(r)
	{
		// printf("z : %f\n",p->oldPlayerZ);
		if(p->oldPlayerR && (z>=0.0f && p->oldPlayerZ<=0.0f))
		{
			// currentPortal=p;
			warpPlayer(p, pl);
			// gravityGunTarget=-1;
		}

		pl->oldInPortal=pl->inPortal;

		if(fabs(z) < PLAYER_RADIUS)pl->inPortal=true;
		else pl->inPortal=false;
	}
	p->oldPlayerR = r;
	p->oldPlayerZ = z;
}

void updatePlayer(player_s* p, room_s* r)
{
	if(!p)return;

	md2InstanceUpdate(&p->gunInstance);
	collideObjectRoom(&p->object, r);

	int i;
	for(i=0; i < NUM_PORTALS; i++)
	{
		if(portals[i].target)checkPortalPlayerWarp(p, &portals[i]);
	}

	updateCamera(&p->camera);

	p->camera.position = p->object.position;
	
	p->object.speed = vect3Df(0,0,0); //TEMP
	p->tempAngle = vmulf(p->tempAngle, 0.65f);
}

extern float debugVal[];

void drawPlayerGun(player_s* p)
{
	if(!p)return;

	gsPushMatrix();
		useCamera(&p->camera);
		gsLoadIdentity();

		gsSwitchRenderMode(md2GsMode);
		gsTranslate(1.3, -1.65, -3.1);
		gsRotateY(p->tempAngle.y);
		gsRotateX(-p->tempAngle.x);
		gsRotateX(-0.1);
		gsRotateZ(M_PI/2);
		gsRotateY(-M_PI/2);
		gsRotateX(-M_PI/2);
		gsScale(3.0f/8, 3.0f/8, 3.0f/8);
		md2InstanceDraw(&p->gunInstance);
	gsPopMatrix();
}

void movePlayer(player_s* p, vect3Df_s v)
{
	if(!p)return;

	p->object.speed = vaddf(p->object.speed, moveCameraVector(&p->camera, v));
}

void rotatePlayer(player_s* p, vect3Df_s v)
{
	if(!p)return;

	p->tempAngle = vaddf(p->tempAngle, v);
	rotateCamera(&p->camera, v);
}

void shootPlayerGun(player_s* p, room_s* r, portal_s* portal)
{
	if(!p)return;
	if(p->gunInstance.currentAnim == PORTALGUN_SHOOT)return;

	md2InstanceChangeAnimation(&p->gunInstance, PORTALGUN_IDLE, false);
	md2InstanceChangeAnimation(&p->gunInstance, PORTALGUN_SHOOT, true);

	vect3Df_s position;
	rectangle_s* rec = collideLineMapClosest(r, NULL, p->camera.position, vect3Df(-p->camera.orientation[2][0], -p->camera.orientation[2][1], -p->camera.orientation[2][2]), 1000.0f, &position, NULL);
	if(rec)
	{
		vect3Df_s normal = rec->normal;
		vect3Df_s plane0 = vect3Df(p->camera.orientation[0][0], p->camera.orientation[0][1], p->camera.orientation[0][2]);
		plane0 = vnormf(vsubf(plane0, vmulf(normal, vdotf(normal, plane0))));

		position = vaddf(position, vmulf(normal, -0.05f));

		portal->position = position;
		updatePortalOrientation(portal, plane0, normal);
	}
}
