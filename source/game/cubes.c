#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/cubes.h"
#include "gfx/gs.h"

cubeDispenser_s cubeDispenser[NUMCUBEDISPENSERS];
md2_model_t cubeModel, cubeDispenserModel;
texture_s storageCubeTexture, companionCubeTexture, cubeDispenserTexture;
md2_instance_t storageCubeModelInstance, companionCubeModelInstance;

void initCubes(void)
{
	int i;
	for(i=0;i<NUMCUBEDISPENSERS;i++)
	{
		cubeDispenser[i].used=false;
		cubeDispenser[i].id=i;
	}
	
	md2ReadModel(&cubeModel, "cube.md2");
	md2ReadModel(&cubeDispenserModel, "cubedispenser.md2");

	textureLoad(&storageCubeTexture, "storagecube.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&companionCubeTexture, "companion.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&cubeDispenserTexture, "cubedispenser.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);

	md2InstanceInit(&storageCubeModelInstance, &cubeModel, &storageCubeTexture);
	md2InstanceInit(&companionCubeModelInstance, &cubeModel, &companionCubeTexture);
}

void exitCubes(void)
{
	return;
	md2FreeModel(&cubeModel);
	md2FreeModel(&cubeDispenserModel);

	textureFree(&storageCubeTexture);
	textureFree(&companionCubeTexture);
	textureFree(&cubeDispenserTexture);
}

void initCubeDispenser(room_s* r, cubeDispenser_s* cd, vect3Di_s pos, bool companion)
{
	if(!cd || !r)return;
	
	md2InstanceInit(&cd->modelInstance, &cubeDispenserModel, &cubeDispenserTexture);

	cd->openingRectangle=NULL;
	
	{//for collisions
		rectangle_s rec;
		rectangle_s* recp;
		rec.material=NULL;
		
		// TEMP DISABLED
		// rec.position=vaddi(pos,vect3Di(-1,-8,1));
		// rec.size=vect3Di(2,0,-2);
		// rec.normal=vect3Df(0,-(1),0);
		// recp=addRoomRectangle(r, rec);
		// if(recp)recp->hide=true;
		// cd->openingRectangle=recp;

		rec.position=vaddi(pos,vect3Di(-1,-8,-1));
		rec.size=vect3Di(2,8,0);
		rec.normal=vect3Df(0,0,-(1));
		recp=addRoomRectangle(r, rec);
		if(recp)recp->hide=true;

		rec.position=vaddi(pos,vect3Di(-1,0,1));
		rec.size=vect3Di(2,-8,0);
		rec.normal=vect3Df(0,0,(1));
		recp=addRoomRectangle(r, rec);
		if(recp)recp->hide=true;

		rec.position=vaddi(pos,vect3Di(-1,-8,-1));
		rec.size=vect3Di(0,8,2);
		rec.normal=vect3Df(-(1),0,0);
		recp=addRoomRectangle(r, rec);
		if(recp)recp->hide=true;

		rec.position=vaddi(pos,vect3Di(1,0,-1));
		rec.size=vect3Di(0,-8,2);
		rec.normal=vect3Df((1),0,0);
		recp=addRoomRectangle(r, rec);
		if(recp)recp->hide=true;
	}
	
	// pos=vect3Di(pos.x+r->position.x, pos.y, pos.z+r->position.y);
	cd->position=convertRectangleVector(pos);

	cd->companion=companion;
	cd->currentCube=NULL;
	
	md2InstanceChangeAnimation(&cd->modelInstance,0,false);
	
	cd->used=true;

	initActivatableObject(&cd->ao);
	cd->ao.active=true;
}

cubeDispenser_s* createCubeDispenser(room_s* r, vect3Di_s pos, bool companion)
{
	if(!r)return NULL;

	int i;
	for(i=0;i<NUMCUBEDISPENSERS;i++)
	{
		if(!cubeDispenser[i].used)
		{
			initCubeDispenser(r, &cubeDispenser[i], pos, companion);
			return &cubeDispenser[i];
		}
	}
	return NULL;
}

void drawCubeDispenser(cubeDispenser_s* cd)
{
	if(!cd)return;

	gsPushMatrix();
		gsSwitchRenderMode(md2GsMode);
		gsTranslate(cd->position.x,cd->position.y,cd->position.z);
		md2InstanceDraw(&cd->modelInstance);
	gsPopMatrix();
}

void drawCubeDispensers(void)
{
	int i;
	for(i=0;i<NUMCUBEDISPENSERS;i++)
	{
		if(cubeDispenser[i].used)drawCubeDispenser(&cubeDispenser[i]);
	}
}

void resetCubeDispenserCube(cubeDispenser_s* cd)
{
	if(!cd)return;

	// if(cd->currentCube)resetBox(cd->currentCube, vectMultInt(cd->position,4));
	md2InstanceChangeAnimation(&cd->modelInstance,1,true);
}

void updateCubeDispenser(cubeDispenser_s* cd)
{
	if(!cd)return;

	if(cd->currentCube && !cd->currentCube->used)cd->currentCube=NULL;
	if(cd->ao.active && !cd->ao.oldActive)
	{
		if(!cd->currentCube)
		{
			physicsCreateObb(&cd->currentCube, cd->position, vect3Df(1.0f, 1.0f, 1.0f), (cd->companion)?(&companionCubeModelInstance):(&storageCubeModelInstance), 1.0f, 0.0f);
			// if(cd->currentCube)cd->currentCube->spawner=(void*)cd;
		}else{
			// createEmancipator(&cd->currentCube->modelInstance,vectDivInt(cd->currentCube->position,4),cd->currentCube->transformationMatrix);
			// if(cd->id==gravityGunTarget)gravityGunTarget=-1;
			resetCubeDispenserCube(cd);
		}
		md2InstanceChangeAnimation(&cd->modelInstance,1,true);
	}

	md2InstanceUpdate(&cd->modelInstance);
	cd->ao.oldActive = cd->ao.active;
}

void updateCubeDispensers(void)
{
	int i;
	for(i=0;i<NUMCUBEDISPENSERS;i++)
	{
		if(cubeDispenser[i].used)updateCubeDispenser(&cubeDispenser[i]);
	}
}
