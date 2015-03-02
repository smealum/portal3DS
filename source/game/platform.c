#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/platform.h"
#include "gfx/gs.h"

#define LOGOSIZE (192)
#define LOGOHEIGHT (64)
#define PLATFORMHEIGHT (32)

platform_s platform[NUMPLATFORMS];
md2_model_t platformModel;
texture_s platformTexture;
md2_instance_t platformModelInstance;

void initPlatforms(void)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		platform[i].used=false;
		platform[i].id=i;
	}
	
	md2ReadModel(&platformModel, "platform.md2");
	textureLoad(&platformTexture, "logo.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	
	md2InstanceInit(&platformModelInstance, &platformModel, &platformTexture);
}

void exitPlatforms(void)
{
	md2FreeModel(&platformModel);
	textureFree(&platformTexture);
}

void initPlatform(platform_s* pf, room_s* r, vect3Di_s orig, vect3Di_s dest, u8 id, bool BAF)
{
	if(!pf || !r)return;
	
	// orig=vect3Df(orig.x+r->position.x, orig.y, orig.z+r->position.y);
	// dest=vect3Df(dest.x+r->position.x, dest.y, dest.z+r->position.y);

	pf->id=id;
	pf->origin=convertRectangleVector(orig);
	pf->destination=convertRectangleVector(dest);

	pf->position=pf->origin;
	pf->velocity=vdivf(vnormf(vsubf(pf->destination,pf->origin)),256);
	
	pf->direction=true;
	pf->touched=false;
	pf->backandforth=true;
	
	pf->oldactive=false;
	pf->active=true;
	
	// addPlatform(id,vmulf(pf->origin,4),vmulf(pf->destination,4),BAF); //TEMP
	
	pf->used=true;
}

platform_s* createPlatform(room_s* r, vect3Di_s orig, vect3Di_s dest, bool BAF)
{
	if(!r)return NULL;
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(!platform[i].used)
		{
			initPlatform(&platform[i],r,orig,dest,i,BAF);
			return &platform[i];
		}
	}
	return NULL;
}

void drawPlatform(platform_s* pf)
{
	if(!pf)return;

	gsPushMatrix();
		md2StartDrawing();
		gsTranslate(pf->position.x, pf->position.y, pf->position.z);
		md2InstanceDraw(&platformModelInstance);
	gsPopMatrix();
}

void drawPlatforms(void)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used)drawPlatform(&platform[i]);
	}
}

void updatePlatform(platform_s* pf)
{
	if(!pf)return;
	
	// if(pf->touched && p->object->position.y>pf->position.y+p->object->radius*2)
	// {
	// 	p->object->position=addVect(p->object->position,pf->velocity);
	// }else if(pf->oldTouched){
	// 	p->object->speed=addVect(p->object->speed,pf->velocity);
	// }
	
	// if(pf->oldactive!=pf->active)togglePlatform(pf->id, pf->active);
	
	pf->oldactive=pf->active;
	pf->velocity=vect3Df(0,0,0);
	pf->oldTouched=pf->touched;
	pf->touched=false;
}

void updatePlatforms(void)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used)updatePlatform(&platform[i]);
	}
}
