#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/walldoor.h"
#include "game/room_io.h"
#include "game/elevator.h"
#include "gfx/gs.h"

#define DOORFRAMELENGTH (8)
#define WALLDOORINTERVAL (32)

room_s elevatorRoom;

wallDoor_s entryWallDoor;
wallDoor_s exitWallDoor;

md2_model_t wallDoorModel;
texture_s wallDoorTexture;

vect3Df_s doorFrameData[]={{-TILESIZE*2,HEIGHTUNIT*8,0},{-TILESIZE*2,HEIGHTUNIT*5,0},{-TILESIZE,HEIGHTUNIT*8,0},{-TILESIZE,HEIGHTUNIT*7,0},{TILESIZE,HEIGHTUNIT*8,0},{TILESIZE,HEIGHTUNIT*7,0},{TILESIZE*2,HEIGHTUNIT*8,0},{TILESIZE*2,HEIGHTUNIT*5,0}};

vect3Di_s wallDoorV1[]={{0,0,1},
					{0,0,-1},
					{0,0,0},
					{0,0,0},
					{1,0,0},
					{-1,0,0}};

vect3Di_s wallDoorV2[]={{-1,0,0},
					{1,0,0},
					{0,0,0},
					{0,0,0},
					{0,0,-1},
					{0,0,1}};

vect3Df_s wallDoorV1f[]={{0,0,1},
					{0,0,-1},
					{0,0,0},
					{0,0,0},
					{1,0,0},
					{-1,0,0}};

vect3Df_s wallDoorV2f[]={{-1,0,0},
					{1,0,0},
					{0,0,0},
					{0,0,0},
					{0,0,-1},
					{0,0,1}};

void initWallDoor(wallDoor_s* wd)
{
	if(!wd)return;

	wd->used=false;
	wd->rectangle=NULL;

	initActivatableObject(&wd->ao);
	
	md2InstanceInit(&wd->modelInstance, &wallDoorModel, &wallDoorTexture);
}

void initWallDoors(void)
{
	initWallDoor(&entryWallDoor);
	initWallDoor(&exitWallDoor);

	//TEMP ?
	md2ReadModel(&wallDoorModel, "door.md2");
	textureLoad(&wallDoorTexture, "door.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);

	readRoom("elevatorroom.map", &elevatorRoom, MAP_READ_LIGHT);
}

void exitWallDoors(void)
{
	md2FreeModel(&wallDoorModel);
	textureFree(&wallDoorTexture);
	freeRoom(&elevatorRoom);
}

void setupWallDoor(room_s* r, wallDoor_s* wd, vect3Di_s position, u8 orientation)
{
	if(!wd || wd->used)return;
	if(!r)return;

	wd->used=true;
	wd->gridPosition=vect3Di(position.x, position.y-4, position.z);
	wd->position=convertRectangleVector(wd->gridPosition);
	wd->orientation=orientation;
	// wd->frameMaterial=getMaterial(1);

	printf("ORIENTATION %d\n",orientation);

	rectangle_s rec;
	rectangle_s* recp;

	//door wall
	rec.material=NULL;
	rec.position=vaddi(position,vsubi(vect3Di(0,-4,0),wallDoorV1[orientation]));
	rec.size=vaddi(vect3Di(0,8,0),vmuli(wallDoorV1[orientation],2));
	rec.normal=wallDoorV2f[orientation];
	wd->rectangle=addRoomRectangle(r, rec);
	if(wd->rectangle)wd->rectangle->hide=true;

	//elevator
	initElevator(&wd->elevator, r, vaddi(position,vmuli(wallDoorV2[wd->orientation],7)), orientation, true);
	setElevatorArriving(&wd->elevator, 2.5f);

	//elevator room
	insertRoom(r, &elevatorRoom, position, orientation);
}

bool pointInWallDoorRoom(wallDoor_s* wd, vect3Df_s p)
{
	if(!wd)return false;
	float v1=vdotf(vsubf(p,wd->position), wallDoorV1f[wd->orientation]);
	float v2=vdotf(vsubf(p,wd->position), wallDoorV2f[wd->orientation]);
	return (v1<=18)&&(wd->gridPosition.y-2<=p.y)&&(v2<=30) && (v1>=-18)&&(wd->gridPosition.y+22>=p.y)&&(v2>0);
}

void updateWallDoor(player_s* pl, wallDoor_s* wd)
{
	if(!wd || !pl)return;

	bool pin=pointInWallDoorRoom(wd, pl->object.position);

	if(pin || wd->ao.active)
	{
		if(wd->modelInstance.currentAnim==0)
		{
			md2InstanceChangeAnimation(&wd->modelInstance, 2, false);
			md2InstanceChangeAnimation(&wd->modelInstance, 1, true);
		}
		updateElevator(&wd->elevator);
	}else{
		if(wd->modelInstance.currentAnim==2)
		{
			md2InstanceChangeAnimation(&wd->modelInstance, 0, false);
			md2InstanceChangeAnimation(&wd->modelInstance, 3, true);
		}
	}

	if(wd->rectangle)
	{
		if(wd->modelInstance.currentAnim==2)wd->rectangle->collides=false;
		else wd->rectangle->collides=true;
	}

	md2InstanceUpdate(&wd->modelInstance);
}

void updateWallDoors(player_s* pl)
{
	if(!pl)return;

	updateWallDoor(pl, &entryWallDoor);
	updateWallDoor(pl, &exitWallDoor);

	// if(exitWallDoor.elevator.state==ELEVATOR_LEAVING)endGame();
}

void drawWallDoor(wallDoor_s* wd)
{
	if(!wd || !wd->used)return;

	gsPushMatrix();
		gsSwitchRenderMode(md2GsMode);

		gsTranslate(wd->position.x, wd->position.y, wd->position.z);
		
		switch(wd->orientation)
		{
			case 4:
				gsRotateY(M_PI);
				break;
			case 1:
				gsRotateY(M_PI/2);
				break;
			case 0:
				gsRotateY(-M_PI/2);
				break;
		}

		md2InstanceDraw(&wd->modelInstance);
	gsPopMatrix();

	drawElevator(&wd->elevator);
}

void drawWallDoors(void)
{
	drawWallDoor(&entryWallDoor);
	drawWallDoor(&exitWallDoor);
}
