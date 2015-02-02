#include <3ds.h>
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
}

void updatePlayer(player_s* p, room_s* r)
{
	if(!p)return;

	md2InstanceUpdate(&p->gunInstance);
	collideObjectRoom(&p->object, r);
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

		md2StartDrawing();
		gsTranslate(1.3, -1.65, -3.1);
		gsRotateY(p->tempAngle.y);
		gsRotateX(-p->tempAngle.x);
		gsRotateY(0.0);
		gsRotateX(-0.1);
		gsRotateZ(M_PI/2);
		gsRotateY(-M_PI/2);
		gsScale(1.0f/16, 1.0f/16, 1.0f/16);
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
