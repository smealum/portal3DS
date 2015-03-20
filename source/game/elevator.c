#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/elevator.h"
#include "gfx/gs.h"

#define ELEVATOR_SPEED (0.05f)

md2_model_t elevatorModel;
md2_model_t elevatorFrameModel;
texture_s elevatorTexture;
texture_s elevatorFrameTexture;

void initElevators(void)
{
	md2ReadModel(&elevatorModel, "elevator.md2");
	textureLoad(&elevatorTexture, "balllauncher.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);

	md2ReadModel(&elevatorFrameModel, "elevatorframe.md2");
	textureLoad(&elevatorFrameTexture, "cubedispenser.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
}

void exitElevators(void)
{
	md2FreeModel(&elevatorModel);
	md2FreeModel(&elevatorFrameModel);

	textureFree(&elevatorTexture);
	textureFree(&elevatorFrameTexture);
}

void initElevator(elevator_s* ev, room_s* r, vect3Di_s position, u8 direction, bool up)
{
	if(!ev)return;

	ev->position=convertRectangleVector(vect3Di(position.x, position.y-4, position.z));;
	ev->direction=direction|(up<<ELEVATOR_UPDOWNBIT);
	ev->progress=0;
	ev->state=ELEVATOR_OPEN;
	ev->doorSurface=NULL;
	md2InstanceInit(&ev->modelInstance, &elevatorModel, &elevatorTexture);
	ev->modelInstance.speed=0.1f;

	printf("EV %d\n",up);
	printf("EV %d\n",ev->direction);
}

void setElevatorArriving(elevator_s* ev, float distance)
{
	if(!ev)return;

	ev->state=ELEVATOR_ARRIVING;
	ev->progress=distance;

	updateElevator(ev);
}

void closeElevator(elevator_s* ev)
{
	if(!ev || ev->state!=ELEVATOR_OPEN)return;

	md2InstanceChangeAnimation(&ev->modelInstance, 0, false);
	md2InstanceChangeAnimation(&ev->modelInstance, 3, true);
	ev->state=ELEVATOR_CLOSING;
}

void updateElevator(elevator_s* ev)
{
	if(!ev)return;

	switch(ev->state)
	{
		case ELEVATOR_ARRIVING:
			md2InstanceChangeAnimation(&ev->modelInstance, 0, false);
			if(ev->progress<=0)
			{
				ev->progress=0;
				md2InstanceChangeAnimation(&ev->modelInstance, 2, false);
				md2InstanceChangeAnimation(&ev->modelInstance, 1, true);
				ev->state=ELEVATOR_OPENING;
			}else ev->progress-=ELEVATOR_SPEED;
			break;
		case ELEVATOR_OPENING:
			ev->progress=0;
			if(ev->modelInstance.currentAnim==2)ev->state=ELEVATOR_OPEN;
			break;
		case ELEVATOR_OPEN:
			ev->progress=0;
			break;
		case ELEVATOR_CLOSING:
			ev->progress=0;
			if(ev->modelInstance.currentAnim==0)ev->state=ELEVATOR_LEAVING;
			break;
		case ELEVATOR_LEAVING:
			md2InstanceChangeAnimation(&ev->modelInstance, 0, false);
			ev->progress+=ELEVATOR_SPEED;
			break;
	}

	bool up=(ev->direction&(1<<ELEVATOR_UPDOWNBIT))!=0;
	ev->realPosition=ev->position;
	ev->realPosition.y+=up?(ev->progress):(-ev->progress);

	if(ev->doorSurface)ev->doorSurface->collides=ev->state!=ELEVATOR_OPEN;

	md2InstanceUpdate(&ev->modelInstance);
}

void drawElevator(elevator_s* ev)
{
	if(!ev)return;

	gsPushMatrix();
		gsSwitchRenderMode(md2GsMode);

		gsTranslate(ev->position.x, ev->position.y, ev->position.z);
		
		switch(ev->direction&(~(1<<ELEVATOR_UPDOWNBIT)))
		{
			case 0:
				gsRotateY(M_PI/2);
				break;
			case 1:
				gsRotateY(-M_PI/2);
				break;
			case 5:
				gsRotateY(M_PI);
				break;
			default:
				break;
		}
		

		md2InstanceDraw(&ev->modelInstance);
	gsPopMatrix();
}
