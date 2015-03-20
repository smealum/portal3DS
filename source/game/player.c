#include <3ds.h>
#include <string.h>
#include "gfx/gs.h"
#include "gfx/texture.h"
#include "game/player.h"
#include "game/sfx.h"

#include "passthrough_vsh_shbin.h"

md2_model_t gunModel;
texture_s gunTexture;

DVLB_s* passthroughDvlb;
shaderProgram_s passthroughProgram;

const u32 rectangleBaseAddr=0x14000000;

float rectangleData[] = {1.0f, 1.0f, 0.0f,
						1.0f, -1.0f, 0.0f,
						-1.0f, -1.0f, 0.0f,
						1.0f, 1.0f, 0.0f,
						-1.0f, -1.0f, 0.0f,
						-1.0f, 1.0f, 0.0f};

u32* rectangleVertexData = NULL;

SFX_s *gunSFX1, *gunSFX2;
SFX_s *portalEnterSFX[2];
SFX_s *portalExitSFX[2];

void playerInit(void)
{
	textureLoad(&gunTexture, "portalgun.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	md2ReadModel(&gunModel, "portalgun.md2");

	//SFX
	gunSFX1=createSFX("portalgun_orange.raw", CSND_ENCODING_PCM16);
	gunSFX2=createSFX("portalgun_blue.raw", CSND_ENCODING_PCM16);

	portalEnterSFX[0]=createSFX("portal_enter1.raw", CSND_ENCODING_PCM16);
	portalEnterSFX[1]=createSFX("portal_enter2.raw", CSND_ENCODING_PCM16);

	portalExitSFX[0]=createSFX("portal_exit1.raw", CSND_ENCODING_PCM16);
	portalExitSFX[1]=createSFX("portal_exit2.raw", CSND_ENCODING_PCM16);
}

void playerExit(void)
{
	md2FreeModel(&gunModel);
	textureFree(&gunTexture);
}

void initPlayer(player_s* p)
{
	if(!p)return;

	initPhysicalPoint(&p->object, vect3Df(0,0,0), PLAYER_RADIUS);
	initCamera(&p->camera);
	md2InstanceInit(&p->gunInstance, &gunModel, &gunTexture);

	p->oldInPortal = p->inPortal = false;

	passthroughDvlb = DVLB_ParseFile((u32*)passthrough_vsh_shbin, passthrough_vsh_shbin_size);
	shaderProgramInit(&passthroughProgram);

	if(!passthroughDvlb)return;

	shaderProgramSetVsh(&passthroughProgram, &passthroughDvlb->DVLE[0]);

	rectangleVertexData = linearAlloc(sizeof(rectangleData));
	memcpy(rectangleVertexData, rectangleData, sizeof(rectangleData));

	p->flying = false;
	p->walkCnt1 = 0;
	p->walkCnt2 = 0;
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
	if(!pl || !p || !p->open || !p->target || !p->target->open)return;
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

extern vect3Df_s normGravityVector;

void updatePlayer(player_s* p, room_s* r)
{
	if(!p)return;

	md2InstanceUpdate(&p->gunInstance);
	if(!p->flying) p->object.speed = vaddf(p->object.speed, vmulf(normGravityVector, 0.04f));
	vect3Df_s prevPosition = p->object.position;
	collideObjectRoom(&p->object, r);

	int i;
	for(i=0; i < NUM_PORTALS; i++)
	{
		if(portals[i].target)checkPortalPlayerWarp(p, &portals[i]);
	}

	updateCamera(&p->camera);

	float alignment = -vdotf(vect3Df(p->camera.orientation[0][0],p->camera.orientation[0][1],p->camera.orientation[0][2]), normGravityVector);
	// printf("alignment : %f  \n",alignment);

	{
		if(alignment>0.001)
		{
			if(alignment>0.125)rotateMatrixZ((float*)p->camera.orientation, -0.5f*0.15f, true);
			else if(alignment>0.0625)rotateMatrixZ((float*)p->camera.orientation, -0.25f*0.15f, true);
			else if(alignment>0.03125)rotateMatrixZ((float*)p->camera.orientation, -0.125f*0.07f, true);
		}else if(alignment<-0.001)
		{
			if(alignment<-0.125)rotateMatrixZ((float*)p->camera.orientation, 0.5f*0.15f, true);
			else if(alignment<-0.0625)rotateMatrixZ((float*)p->camera.orientation, 0.25f*0.15f, true);
			else if(alignment<-0.03125)rotateMatrixZ((float*)p->camera.orientation, 0.125f*0.07f, true);
		}
	}
	
	// fixMatrix(c->orientation); //compensate floating point errors

	p->camera.position = vaddf(p->object.position, vect3Df(0.0f, cos(p->walkCnt1)*0.14f, 0.0f));

	if(p->flying) p->object.speed = vect3Df(0,0,0); //TEMP
	p->tempAngle = vmulf(p->tempAngle, 0.65f);
}

void updatePlayerWalk(player_s* p, float wc1, float wc2)
{
	if(!p)return;

	p->walkCnt1+=wc1;
	p->walkCnt2+=wc2;
}

extern float debugVal[];

void drawPlayerGun(player_s* p)
{
	if(!p)return;

	gsPushMatrix();
		useCamera(&p->camera);
		gsLoadIdentity();

		GPU_SetDepthTestAndWriteMask(true, GPU_ALWAYS, GPU_WRITE_DEPTH);
		gsSwitchRenderMode(-1);

		GPU_SetAttributeBuffers(
			1, // number of attributes
			(u32*)osConvertVirtToPhys(rectangleBaseAddr), // we use the start of linear heap as base since that's where all our buffers are located
			GPU_ATTRIBFMT(0, 3, GPU_FLOAT), // we want v0 (vertex position)
			0xFFE, // mask : we want v0
			0x0, // permutation : we use identity
			1, // number of buffers : we have one attribute per buffer
			(u32[]){(u32)rectangleVertexData-rectangleBaseAddr}, // buffer offsets (placeholders)
			(u64[]){0x0}, // attribute permutations for each buffer
			(u8[]){1} // number of attributes for each buffer
			);

		gsSetShader(&passthroughProgram);

		GPU_DrawArray(GPU_TRIANGLES, 6);

		GPU_SetDepthTestAndWriteMask(true, GPU_GREATER, GPU_WRITE_ALL);
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

	p->object.speed = vaddf(p->object.speed, moveCameraVector(&p->camera, v, p->flying));
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

	playSFX(gunSFX1);

	md2InstanceChangeAnimation(&p->gunInstance, PORTALGUN_IDLE, false);
	md2InstanceChangeAnimation(&p->gunInstance, PORTALGUN_SHOOT, true);

	vect3Df_s position;
	rectangle_s* rec = collideLineMapClosest(r, NULL, p->camera.position, vect3Df(-p->camera.orientation[2][0], -p->camera.orientation[2][1], -p->camera.orientation[2][2]), 1000.0f, &position, NULL);
	if(rec && rec->portalable)
	{
		portal_s oldPortal = *portal;
		vect3Df_s normal = rec->normal;
		vect3Df_s plane0 = vect3Df(p->camera.orientation[0][0], p->camera.orientation[0][1], p->camera.orientation[0][2]);
		plane0 = vnormf(vsubf(plane0, vmulf(normal, vdotf(normal, plane0))));

		position = vaddf(position, vmulf(normal, -0.05f));

		portal->position = position;

		updatePortalOrientation(portal, plane0, normal);
		isPortalOnWall(r, portal, true);

		portal->draw = true;
		portal->open = true;

		if(isPortalOnWall(r, portal, false))
		{
			ejectPortalOBBs(portal);
			ejectPortalOBBs(portal->target);
		}else{
			*portal = oldPortal;
		}
	}
}
