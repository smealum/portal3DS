#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/door.h"
#include "gfx/gs.h"

door_s door[NUMDOORS];
md2_model_t doorModel;
texture_s doorTexture;
SFX_s* doorOpenSFX;
SFX_s* doorCloseSFX;

void initDoors(void)
{
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		door[i].used=false;
		door[i].id=i;
	}

	md2ReadModel(&doorModel, "door.md2");
	textureLoad(&doorTexture, "door.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);

	doorOpenSFX=createSFX("door_open.raw", CSND_ENCODING_PCM16);
	doorCloseSFX=createSFX("door_close.raw", CSND_ENCODING_PCM16);
}

void exitDoors(void)
{
	md2FreeModel(&doorModel);
	textureFree(&doorTexture);
}

void initDoor(door_s* d, room_s* r, vect3Di_s position, bool orientation)
{
	if(!d || !r)return;

	//for collisions

	rectangle_s rec;
	rec.material=NULL;

	if(!orientation)
	{
		rec.position=vaddi(position,vect3Di(-1,0,0));
		rec.size=vect3Di(2,8,0);
		rec.normal=vect3Df(0,0,1.0f);
	}else{
		rec.position=vaddi(position,vect3Di(0,0,-1));
		rec.size=vect3Di(0,8,2);
		rec.normal=vect3Df(1.0f,0,0);
	}

	d->rectangle[0]=addRoomRectangle(r, rec);
	if(d->rectangle[0]){d->rectangle[0]->hide=true;d->rectangle[0]->portalable=false;d->rectangle[0]->collides=true;}

	rec.position.y+=rec.size.y;
	rec.size.y=-rec.size.y;
	d->rectangle[1]=addRoomRectangle(r, rec);
	if(d->rectangle[1]){d->rectangle[1]->hide=true;d->rectangle[1]->portalable=false;d->rectangle[1]->collides=true;}

	md2InstanceInit(&d->modelInstance, &doorModel, &doorTexture);
	d->position=convertRectangleVector(vect3Di(position.x, position.y, position.z));
	d->orientation=orientation;
	initActivatableObject(&d->ao);
	d->used=true;
}

door_s* createDoor(room_s* r, vect3Di_s position, bool orientation)
{
	if(!r)return NULL;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(!door[i].used)
		{
			initDoor(&door[i], r, position, orientation);
			return &door[i];
		}
	}
	return NULL;
}

void updateDoor(door_s* d)
{
	if(!d || !d->used)return;

	if(d->ao.active)
	{
		if(d->modelInstance.currentAnim==0)
		{
			md2InstanceChangeAnimation(&d->modelInstance, 2, false);
			md2InstanceChangeAnimation(&d->modelInstance, 1, true);
			playSFX(doorOpenSFX);
		}else if(d->modelInstance.oldAnim==1 && d->modelInstance.currentAnim==2)
		{
			if(d->rectangle[0]){d->rectangle[0]->collides=false;/*toggleAAR(d->rectangle[0]->AARid);*/}
			if(d->rectangle[1]){d->rectangle[1]->collides=false;/*toggleAAR(d->rectangle[1]->AARid);*/}
		}
	}else
	{
		if(d->modelInstance.currentAnim==2)
		{
			md2InstanceChangeAnimation(&d->modelInstance, 0, false);
			md2InstanceChangeAnimation(&d->modelInstance, 3, true);
			playSFX(doorCloseSFX);
			if(d->rectangle[0]){d->rectangle[0]->collides=true;/*toggleAAR(d->rectangle[0]->AARid);*/}
			if(d->rectangle[1]){d->rectangle[1]->collides=true;/*toggleAAR(d->rectangle[1]->AARid);*/}
		}
	}

	md2InstanceUpdate(&d->modelInstance);
	d->ao.active=false;
}

void updateDoors(void)
{
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(door[i].used)updateDoor(&door[i]);
	}
}

void drawDoor(door_s* d)
{
	if(!d || !d->used)return;

	gsPushMatrix();
		gsSwitchRenderMode(md2GsMode);

		gsTranslate(d->position.x, d->position.y, d->position.z);
		
		if(d->orientation)gsRotateY(M_PI/2);

		md2InstanceDraw(&d->modelInstance);
	gsPopMatrix();
}

void drawDoors(void)
{
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(door[i].used)drawDoor(&door[i]);
	}
}
