#include <3ds.h>
#include <string.h>
#include "gfx/gs.h"
#include "gfx/texture.h"
#include "game/player.h"
#include "game/sfx.h"

#include "passthrough_vsh_shbin.h"

md2_model_t gunModel, ratmanModel;
texture_s gunTextureOrange, gunTextureBlue, ratmanTexture;
texture_s crosshairTexture;

DVLB_s* passthroughDvlb;
shaderProgram_s passthroughProgram;

const u32 rectangleBaseAddr=0x14000000;

float rectangleData[] = {1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
						1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
						-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
						1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
						-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
						-1.0f, 1.0f, 0.0f, 0.0f, 0.0f,};

#define crosshairWidth (16.0f / 400.0f)
#define crosshairHeight (16.0f / 240.0f)

float crosshairData[] = {crosshairHeight, crosshairWidth, 0.0f, 1.0f, 1.0f,
						crosshairHeight, -crosshairWidth, 0.0f, 1.0f, 0.0f,
						-crosshairHeight, -crosshairWidth, 0.0f, 0.0f, 0.0f,
						crosshairHeight, crosshairWidth, 0.0f, 1.0f, 1.0f,
						-crosshairHeight, -crosshairWidth, 0.0f, 0.0f, 0.0f,
						-crosshairHeight, crosshairWidth, 0.0f, 0.0f, 1.0f,};

u32* rectangleVertexData = NULL;
u32* crosshairVertexData = NULL;

SFX_s *gunSFX1, *gunSFX2;
SFX_s *portalEnterSFX[2];
SFX_s *portalExitSFX[2];

void playerInit(void)
{
	md2ReadModel(&gunModel, "portalgun.md2");
	textureLoad(&gunTextureOrange, "portalgun_orange.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&gunTextureBlue, "portalgun_blue.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&crosshairTexture, "crosshair.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	
	md2ReadModel(&ratmanModel, "ratman.md2");
	textureLoad(&ratmanTexture, "ratman.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 10);

	//SFX
	gunSFX1=createSFX("portalgun_orange.raw", SOUND_FORMAT_16BIT);
	gunSFX2=createSFX("portalgun_blue.raw", SOUND_FORMAT_16BIT);

	portalEnterSFX[0]=createSFX("portal_enter1.raw", SOUND_FORMAT_16BIT);
	portalEnterSFX[1]=createSFX("portal_enter2.raw", SOUND_FORMAT_16BIT);

	portalExitSFX[0]=createSFX("portal_exit1.raw", SOUND_FORMAT_16BIT);
	portalExitSFX[1]=createSFX("portal_exit2.raw", SOUND_FORMAT_16BIT);
}

void playerExit(void)
{
	md2FreeModel(&gunModel);
	textureFree(&gunTextureOrange);
	textureFree(&gunTextureBlue);
	textureFree(&crosshairTexture);
	md2FreeModel(&ratmanModel);
	textureFree(&ratmanTexture);
}

void initPlayer(player_s* p)
{
	if(!p)return;

	initPhysicalPoint(&p->object, vect3Df(0,0,0), PLAYER_RADIUS);
	initCamera(&p->camera);
	md2InstanceInit(&p->gunInstance, &gunModel, &gunTextureOrange);
	md2InstanceInit(&p->ratmanInstance, &ratmanModel, &ratmanTexture);

	p->ratmanInstance.speed=0.1f;

	p->oldInPortal = p->inPortal = false;

	passthroughDvlb = DVLB_ParseFile((u32*)passthrough_vsh_shbin, passthrough_vsh_shbin_size);
	shaderProgramInit(&passthroughProgram);

	if(!passthroughDvlb)return;

	shaderProgramSetVsh(&passthroughProgram, &passthroughDvlb->DVLE[0]);

	rectangleVertexData = linearAlloc(sizeof(rectangleData));
	memcpy(rectangleVertexData, rectangleData, sizeof(rectangleData));

	crosshairVertexData = linearAlloc(sizeof(crosshairData));
	memcpy(crosshairVertexData, crosshairData, sizeof(crosshairData));

	p->flying = false;
	p->life = 80;
	p->walkCnt1 = 0;
	p->walkCnt2 = 0;
}

extern OBB_s* gravityGunObject;

void warpPlayer(portal_s* p, player_s* pl)
{
	if(!p || !pl)return;
	camera_s* c = &pl->camera;

	camera_s new_camera = *c;

	if(gravityGunObject)gravityGunObject = NULL; // TEMP : TODO better solution

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
	md2InstanceUpdate(&p->ratmanInstance);
	if(!p->flying) p->object.speed = vaddf(p->object.speed, vmulf(normGravityVector, 0.04f));
	vect3Df_s prevPosition = p->object.position;
	collideObjectRoom(&p->object, r);

	if(p->inPortal && !p->oldInPortal)playSFX(portalEnterSFX[rand()%2]);
	else if(!p->inPortal && p->oldInPortal)playSFX(portalExitSFX[rand()%2]);

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

	if(vmagf(p->object.speed) < 0.03f || !p->object.contact)md2InstanceChangeAnimation(&p->ratmanInstance, 0, false);

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
			2, // number of attributes
			(u32*)osConvertVirtToPhys(rectangleBaseAddr), // we use the start of linear heap as base since that's where all our buffers are located
			GPU_ATTRIBFMT(0, 3, GPU_FLOAT)|GPU_ATTRIBFMT(1, 2, GPU_FLOAT), // we want v0 (vertex position) and v1 (texcoord)
			0xFFC, // mask : we want v0 and v1
			0x10, // permutation : we use identity
			1, // number of buffers : we have one attribute per buffer
			(u32[]){(u32)rectangleVertexData-rectangleBaseAddr}, // buffer offsets (placeholders)
			(u64[]){0x10}, // attribute permutations for each buffer
			(u8[]){2} // number of attributes for each buffer
			);

		gsSetShader(&passthroughProgram);

		GPU_DrawArray(GPU_TRIANGLES, 6);

		GPU_SetDepthTestAndWriteMask(true, GPU_ALWAYS, GPU_WRITE_ALL);

		GPUCMD_AddWrite(GPUREG_ATTRIBBUFFER0_CONFIG0, (u32)crosshairVertexData-rectangleBaseAddr);
		textureBind(&crosshairTexture, GPU_TEXUNIT0);

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

void drawPlayer(player_s* p)
{
	if(!p)return;
	
	gsPushMatrix();
		camera_s* c=&p->camera;
		gsTranslate(p->object.position.x,p->object.position.y-4.0f,p->object.position.z);
		
		float m[9];

		vect3Df_s v1 = moveCameraVector(c, vect3Df(1.0f, 0.0f, 0.0f), false);
		vect3Df_s v2 = moveCameraVector(c, vect3Df(0.0f, 0.0f, 1.0f), false);

		m[0]=v1.x;m[3]=v1.y;m[6]=v1.z;
		m[1]=0.0f;m[4]=1.0f;m[7]=0.0f;
		m[2]=v2.x;m[5]=v2.y;m[8]=v2.z;

		gsMultMatrix3(m);

		gsScale(1.2f,1.2f,1.2f);

		gsRotateY(M_PI);

		md2InstanceDraw(&p->ratmanInstance);
	gsPopMatrix();
}

void movePlayer(player_s* p, vect3Df_s v)
{
	if(!p)return;

	if(p->object.contact)
	{
		if(fabs(v.z)>0.0f && fabs(v.z)>fabs(v.x))md2InstanceChangeAnimation(&p->ratmanInstance, 3, false);
		else if((v.x)<0.0f)md2InstanceChangeAnimation(&p->ratmanInstance, 4, false);
		else if((v.x)>0.0f)md2InstanceChangeAnimation(&p->ratmanInstance, 5, false);
	}

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
