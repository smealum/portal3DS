#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/timedbutton.h"
#include "gfx/gs.h"

#define BUTTONTIMER (7*30)
#define TIMEDBUTTON_RADIUS_OUT (TILESIZE/2)
#define TIMEDBUTTON_HEIGHT (TILESIZE*2)

timedButton_s timedButton[NUMTIMEDBUTTONS];

md2_model_t timedButtonModel;
texture_s timedButtonTexture;
texture_s timedButtonActiveTexture;

void initTimedButtons(void)
{
	int i;
	for(i=0;i<NUMTIMEDBUTTONS;i++)
	{
		timedButton[i].used=false;
		timedButton[i].room=NULL;
		timedButton[i].id=i;
	}
	
	md2ReadModel(&timedButtonModel, "button2.md2");

	textureLoad(&timedButtonTexture, "button2.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&timedButtonActiveTexture, "button2b.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
}

void exitTimedButtons(void)
{
	md2FreeModel(&timedButtonModel);

	textureFree(&timedButtonTexture);
	textureFree(&timedButtonActiveTexture);
}

void initTimedButton(timedButton_s* bb, room_s* r, vect3Di_s pos, float angle)
{
	if(!bb || !r)return;
	
	bb->room=r;
	
	md2InstanceInit(&bb->modelInstance, &timedButtonModel, &timedButtonTexture);
	
	bb->position=convertRectangleVector(pos);

	initActivator(&bb->activator);
	bb->active=false;
	bb->angle=angle;
	
	bb->used=true;
}

timedButton_s* createTimedButton(room_s* r, vect3Di_s position, float angle)
{
	if(!r)return NULL;
	int i;
	for(i=0;i<NUMTIMEDBUTTONS;i++)
	{
		if(!timedButton[i].used)
		{
			initTimedButton(&timedButton[i], r, position, angle);
			return &timedButton[i];
		}
	}
	return NULL;
}

void drawTimedButton(timedButton_s* bb)
{
	if(!bb || !bb->used)return;

	gsPushMatrix();
		gsSwitchRenderMode(md2GsMode);

		gsTranslate(bb->position.x, bb->position.y, bb->position.z);
		
		gsRotateY(bb->angle);

		md2InstanceDraw(&bb->modelInstance);
	gsPopMatrix();
}

void drawTimedButtons(void)
{
	int i;
	for(i=0;i<NUMTIMEDBUTTONS;i++)
	{
		if(timedButton[i].used)
		{
			drawTimedButton(&timedButton[i]);
		}
	}
}

void updateTimedButton(timedButton_s* bb)
{
	if(!bb || !bb->used)return;
	
	if(bb->active)
	{
		md2InstanceChangeAnimation(&bb->modelInstance,1,false);
		useActivator(&bb->activator);
		bb->active--;
	}else{
		md2InstanceChangeAnimation(&bb->modelInstance,0,false);
		unuseActivator(&bb->activator);
	}
	
	md2InstanceUpdate(&bb->modelInstance);
}

void activateTimedButton(timedButton_s* tb)
{
	if(!tb || !tb->used)return;

	tb->active=BUTTONTIMER;
}

void updateTimedButtons(void)
{
	int i;
	for(i=0;i<NUMTIMEDBUTTONS;i++)
	{
		if(timedButton[i].used)
		{
			updateTimedButton(&timedButton[i]);
		}
	}
}

float distanceLinePoint(vect3Df_s o, vect3Df_s u, vect3Df_s p)
{
	vect3Df_s v=vsubf(p, o);
	v=vsubf(v,vmulf(u,vdotf(v,u)));
	return vmagf(v);
}

timedButton_s* collideRayTimedButtons(vect3Df_s o, vect3Df_s v, float l)
{
	int i;
	for(i=0;i<NUMTIMEDBUTTONS;i++)
	{
		if(timedButton[i].used)
		{
			timedButton_s* tb=&timedButton[i];
			vect3Df_s p=timedButton[i].position;
			p.y+=TILESIZE*2;
			float d=distanceLinePoint(o,v,p);
			if(d<=(TILESIZE*3)/8)return tb;
		}
	}
	return NULL;
}

bool checkObjectTimedButtonCollision(physicalPoint_s* o, room_s* r, timedButton_s* tb)
{
	if(!o || !r || !tb)return false;

	bool ret=false;

	vect3Df_s u=vect3Df(o->position.x-tb->position.x,0,o->position.z-tb->position.z);
	float v=vmagf(u);

	if(fabs(o->position.y-tb->position.y)>TIMEDBUTTON_HEIGHT)return ret;

	if(v<o->radius+TIMEDBUTTON_RADIUS_OUT)
	{
		u=vdivf(vmulf(u,o->radius+TIMEDBUTTON_RADIUS_OUT-v),v);
		o->position=vaddf(o->position,u);
		ret=true;
	}

	return ret;
}

bool checkObjectTimedButtonsCollision(physicalPoint_s* o, room_s* r)
{
	int i;
	bool ret=false;
	for(i=0;i<NUMTIMEDBUTTONS;i++)
	{
		if(timedButton[i].used)
		{
			if(checkObjectTimedButtonCollision(o,r,&timedButton[i]))ret=true;
		}
	}
	return ret;
}

