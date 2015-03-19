#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/platform.h"
#include "gfx/gs.h"

#define LOGOSIZE (192)
#define LOGOHEIGHT (64)
#define PLATFORMHEIGHT (0.3f)

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
	pf->velocity=vdivf(vnormf(vsubf(pf->destination,pf->origin)),16);
	
	pf->direction=true;
	pf->touched=false;
	pf->backandforth=true;
	
	initActivatableObject(&pf->ao);
	pf->ao.active=true;

	pf->aar=createAAR(vaddf(pf->position, vect3Df(-PLATFORMSIZE, PLATFORMHEIGHT, -PLATFORMSIZE)), vect3Df(2*PLATFORMSIZE, 0.0f, 2*PLATFORMSIZE), vect3Df(0.0f, 1.0f, 0.0f));
	
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
		gsSwitchRenderMode(md2GsMode);
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

void updatePlatform(platform_s* pf, player_s* p)
{
	if(!pf)return;

	if(pf->ao.active)
	{
		if(pf->touched && p->object.position.y>pf->position.y+p->object.radius)
		{
			p->object.position=vaddf(p->object.position,pf->velocity);
		}else if(pf->oldTouched){
			p->object.speed=vaddf(p->object.speed,pf->velocity);
		}

		if(pf->direction)
		{
			if(vdotf(vsubf(pf->position,pf->destination),pf->velocity)>0)
			{
				if(pf->backandforth)
				{
					pf->velocity=vmulf(pf->velocity,-1);
					pf->direction=false;
				}else{
					pf->velocity=vect3Df(0,0,0);
					pf->ao.active=false;
				}
			}
		}else{
			if(vdotf(vsubf(pf->position,pf->origin),pf->velocity)>0)
			{
				pf->velocity=vmulf(pf->velocity,-1);
				pf->direction=true;
			}
		}

		pf->position=vaddf(pf->position,pf->velocity);
	}
	
	pf->ao.oldActive=pf->ao.active;
	pf->oldTouched=pf->touched;
	pf->touched=false;
}

void updatePlatforms(player_s* p)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used)updatePlatform(&platform[i], p);
	}
}
