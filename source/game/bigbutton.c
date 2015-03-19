#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/bigbutton.h"
#include "gfx/gs.h"

bigButton_s bigButton[NUMBIGBUTTONS];

md2_model_t bigButtonModel;
texture_s bigButtonTexture, bigButtonActiveTexture;

void initBigButtons(void)
{
	int i;
	for(i=0;i<NUMBIGBUTTONS;i++)
	{
		bigButton[i].used=false;
		bigButton[i].room=NULL;
		bigButton[i].id=i;
	}
	
	md2ReadModel(&bigButtonModel, "button1.md2");
	
	textureLoad(&bigButtonTexture, "button1.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&bigButtonActiveTexture, "button1b.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
}

void exitBigButtons(void)
{
	md2FreeModel(&bigButtonModel);
	textureFree(&bigButtonTexture);
	textureFree(&bigButtonActiveTexture);
}

void initBigButton(bigButton_s* bb, room_s* r, vect3Di_s pos)
{
	if(!bb || !r)return;
	
	bb->room=r;
	
	md2InstanceInit(&bb->modelInstance, &bigButtonModel, &bigButtonTexture);
	
	{//for collisions
		rectangle_s rec;
		rectangle_s* recp;
		rec.material=NULL;
		
		rec.position=vaddi(pos,vect3Di(-1,1,-1));
		rec.size=vect3Di(2,0,2);
		rec.normal=vect3Df(0,1.0f,0);
		recp=addRoomRectangle(r, rec);
		if(recp){recp->hide=true;recp->collides=true;bb->surface=recp;}

		rec.position=vaddi(pos,vect3Di(-1,0,-1));
		rec.size=vect3Di(2,1,0);
		rec.normal=vect3Df(0,0,-1.0f);
		recp=addRoomRectangle(r, rec);
		if(recp){recp->hide=true;recp->collides=true;}

		rec.position=vaddi(pos,vect3Di(-1,1,1));
		rec.size=vect3Di(2,-1,0);
		rec.normal=vect3Df(0,0,1.0f);
		recp=addRoomRectangle(r, rec);
		if(recp){recp->hide=true;recp->collides=true;}

		rec.position=vaddi(pos,vect3Di(-1,0,-1));
		rec.size=vect3Di(0,1,2);
		rec.normal=vect3Df(-1.0f,0,0);
		recp=addRoomRectangle(r, rec);
		if(recp){recp->hide=true;recp->collides=true;}

		rec.position=vaddi(pos,vect3Di(1,1,-1));
		rec.size=vect3Di(0,-1,2)
		;rec.normal=vect3Df(1.0f,0,0);
		recp=addRoomRectangle(r, rec);
		if(recp){recp->hide=true;recp->collides=true;}

	}
	
	// pos=vect(pos.x+r->position.x, pos.y, pos.z+r->position.y);
	bb->position=convertRectangleVector(pos);
	
	initActivator(&bb->activator);
	bb->active=false;
	
	bb->used=true;
}

bigButton_s* createBigButton(room_s* r, vect3Di_s position)
{
	if(!r)return NULL;
	int i;
	for(i=0;i<NUMBIGBUTTONS;i++)
	{
		if(!bigButton[i].used)
		{
			initBigButton(&bigButton[i], r, position);
			return &bigButton[i];
		}
	}
	return NULL;
}

void drawBigButton(bigButton_s* bb)
{
	if(!bb || !bb->used)return;

	gsPushMatrix();
		gsSwitchRenderMode(md2GsMode);

		gsTranslate(bb->position.x, bb->position.y, bb->position.z);

		bb->modelInstance.texture = bb->active?&bigButtonActiveTexture:&bigButtonTexture;
		md2InstanceDraw(&bb->modelInstance);
	gsPopMatrix();
}

void drawBigButtons(void)
{
	int i;
	for(i=0;i<NUMBIGBUTTONS;i++)
	{
		if(bigButton[i].used)
		{
			drawBigButton(&bigButton[i]);
		}
	}
}

void updateBigButton(bigButton_s* bb)
{
	if(!bb || !bb->used)return;
	
	bb->active=false;
	if(bb->surface && bb->surface->touched)bb->active=true;
	// if(bb->surface->AARid>0)
	// {
	// 	if(!bb->active)bb->active=aaRectangles[bb->surface->AARid].touched;
	// 	aaRectangles[bb->surface->AARid].touched=false;
	// }
	if(bb->active)
	{
		md2InstanceChangeAnimation(&bb->modelInstance,1,false);
		useActivator(&bb->activator);
	}else{
		md2InstanceChangeAnimation(&bb->modelInstance,0,false);
		unuseActivator(&bb->activator);
	}
	
	md2InstanceUpdate(&bb->modelInstance);
}

void updateBigButtons(void)
{
	int i;
	for(i=0;i<NUMBIGBUTTONS;i++)
	{
		if(bigButton[i].used)
		{
			updateBigButton(&bigButton[i]);
		}
	}
}
