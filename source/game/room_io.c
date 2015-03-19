#include <stdlib.h>
#include <stdio.h>
#include <3ds.h>

#include "game/room_io.h"
#include "game/player.h"
#include "game/portal.h"
#include "game/cubes.h"
#include "game/door.h"
#include "game/walldoor.h"
#include "game/energyball.h"
#include "game/platform.h"
#include "game/bigbutton.h"
#include "game/timedbutton.h"
#include "game/activator.h"
#include "game/sludge.h"
#include "game/light.h"

#include "utils/filesystem.h"

#define NUMENTITIES (256)

s16 entityTargetArray[NUMENTITIES];
activator_s* entityActivatorArray[NUMENTITIES];
void* entityEntityArray[NUMENTITIES];
activatorTarget_t entityTargetTypeArray[NUMENTITIES];

void readVect3Di(vect3Di_s* v, FILE* f)
{
	if(!v || !f)return;

	fread(v, sizeof(vect3Di_s), 1, f);
}

void readVect3Df(vect3Df_s* v, FILE* f, bool fp)
{
	if(!v || !f)return;

	if(fp)
	{
		s32 fpv[3];
		fread(fpv, sizeof(s32), 3, f);
		v->x=f32tofloat(fpv[0]);
		v->y=f32tofloat(fpv[1]);
		v->z=f32tofloat(fpv[2]);
	}else fread(v, sizeof(vect3Df_s), 1, f);
}

void readRectangle(rectangle_s* rec, FILE* f)
{
	if(!rec || !f)return;
	
	readVect3Di(&rec->position, f);
	readVect3Di(&rec->size, f);
	readVect3Df(&rec->normal, f, true);

	// printf("rec : %d %d %d\n", rec->position.x, rec->position.y, rec->position.z);

	fread(&rec->portalable, sizeof(bool), 1, f);
	
	u16 mid=0; fread(&mid,sizeof(u16),1,f);

	rec->lightData.lightMap = NULL;

	//TEMP ?
	if(rec->portalable)rec->material=getMaterial(1);
	else rec->material=getMaterial(2);
}

void readRectangles(room_s* r, FILE* f)
{
	if(!r || !f)return;
	int i;
	int k=r->rectangles.num;
	r->rectangles.num=0;
	for(i=0;i<k;i++)
	{
		rectangle_s rec;
		readRectangle(&rec, f);
		addRoomRectangle(r, rec);
	}
}

void readSludgeRectangles(FILE* f)
{
	if(!f)return;
	int i, k;

	fread(&k,sizeof(int),1,f);
	for(i=0;i<k;i++)
	{
		rectangle_s rec;
		readRectangle(&rec, f);
		addSludgeRectangle(&rec);
	}
}

extern player_s testPlayer;

void addEntityTarget(u8 k, u8 n, void* target, activatorTarget_t type)
{
	if(!target)return;
	int i;
	for(i=0;i<n;i++)
	{
		if(entityTargetArray[i]==k && entityActivatorArray[i])
		{
			addActivatorTarget(entityActivatorArray[i], target, type);
		}
	}
}

void readEntity(room_s* r, u8 i, FILE* f)
{
	if(!f)return;
	u8 type=0, dir=0; vect3Di_s v;
	fread(&type, sizeof(u8), 1, f);
	readVect3Di(&v, f);
	fread(&dir, sizeof(u8), 1, f);
	entityTargetArray[i]=-1;
	entityActivatorArray[i]=NULL;
	entityEntityArray[i]=NULL;
	switch(type)
	{
		case 0:
			//energy ball catcher
			{
				vect3Di_s p; readVect3Di(&p,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
				energyDevice_s* e=createEnergyDevice(r, p, dir, type);
				if(e)entityActivatorArray[i]=&e->activator;
				entityTargetArray[i]=target;
			}
			break;
		case 1:
			//energy ball launcher
			{
				vect3Di_s p; readVect3Di(&p,f);
				createEnergyDevice(r, p, dir, type);
			}
			break;
		case 2:
			//timed button
			{
				vect3Di_s p; readVect3Di(&p,f);
				u8 d; fread(&d, sizeof(u8), 1, f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
				timedButton_s* e=createTimedButton(r, p, (d+2)*M_PI);
				if(e)entityActivatorArray[i]=&e->activator;
				entityTargetArray[i]=target;
			}
			break;
		case 3:
			//pressure button
			{
				vect3Di_s p; readVect3Di(&p,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
				bigButton_s* e=createBigButton(r, p);
				if(e)entityActivatorArray[i]=&e->activator;
				entityTargetArray[i]=target;
			}
			break;
		case 4:
			//turret
			{
				vect3Di_s p; readVect3Di(&p,f);
				u8 d; fread(&d, sizeof(u8), 1, f);
				// createTurret(r, p, d);
			}
			break;
		case 5: case 6:
			//cubes
			{
				vect3Di_s p; readVect3Di(&p,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
			}
			break;
		case 7:
			//dispenser
			{
				vect3Di_s p; readVect3Di(&p,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
				cubeDispenser_s* e=createCubeDispenser(r, p, true);
				entityEntityArray[i]=(void*)e;
				entityTargetTypeArray[i]=DISPENSER_TARGET;
			}
			break;
		case 8:
			//emancipation grid
			{
				s32 l; fread(&l,sizeof(s32),1,f);
				vect3Di_s p; readVect3Di(&p,f);
				createEmancipationGrid(r, p, (dir%2)?(-l):(l), !(dir<=1)); //TEMP ?
			}
			break;
		case 9:
			//platform
			{
				vect3Di_s p1, p2;
				readVect3Di(&p1,f);
				readVect3Di(&p2,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
				platform_s* e=createPlatform(r, p1, p2, true);
				entityEntityArray[i]=(void*)e;
				entityTargetTypeArray[i]=PLATFORM_TARGET;
			}
			return;
		case 10:
			//door
			{
				vect3Di_s p; readVect3Di(&p,f);
				u8 orientation; fread(&orientation, sizeof(u8), 1, f);
				door_s* e=createDoor(r, p, orientation%2);
				entityEntityArray[i]=(void*)e;
				entityTargetTypeArray[i]=DOOR_TARGET;
			}
			break;
		case 11:
			//light
			{
				vect3Di_s p; readVect3Di(&p,f);
				// createLight(p, TILESIZE*2*16);
			}
			break;
		case 12:
			//platform target
			{
				s16 target=-1;
				fread(&target, sizeof(s16), 1, f);
			}
			return;
		case 13:
			//wall door (start)
			{
				vect3Di_s p; readVect3Di(&p,f);
				u8 o; fread(&o,sizeof(u8),1,f);
				printf("start : %d %d %d\n", p.x, p.y, p.z);
				setupWallDoor(r, &entryWallDoor, p, o);
				if(entryWallDoor.used)
				{
					testPlayer.object.position=vaddf(entryWallDoor.elevator.realPosition,vect3Df(0,PLAYER_RADIUS*5,0));
					switch(o)
					{
						case 0:
							rotateCamera(NULL, vect3Df(0,-M_PI/2,0));
							break;
						case 1:
							rotateCamera(NULL, vect3Df(0,M_PI/2,0));
							break;
						case 4:
							rotateCamera(NULL, vect3Df(0,M_PI*2,0));
							break;
					}
				}
				entityEntityArray[i]=(void*)&entryWallDoor;
				entityTargetTypeArray[i]=WALLDOOR_TARGET;
			}
			return;
		case 14:
			//wall door (exit)
			{
				vect3Di_s p; readVect3Di(&p,f);
				u8 o; fread(&o,sizeof(u8),1,f);
				setupWallDoor(r, &exitWallDoor, p, o);
				entityEntityArray[i]=(void*)&exitWallDoor;
				entityTargetTypeArray[i]=WALLDOOR_TARGET;
				// exitWallDoor.override=true;
			}
			return;
		default:
			break;
	}
}

void readEntities(room_s* r, FILE* f)
{
	if(!f)return;

	u16 cnt; fread(&cnt,sizeof(u16),1,f);
	int i; for(i=0;i<cnt;i++)readEntity(r,i,f);
	for(i=0;i<cnt;i++)addEntityTarget(i,cnt,entityEntityArray[i],entityTargetTypeArray[i]);
}

void readHeader(mapHeader_s* h, FILE* f)
{
	if(!h || !f)return;
	fseek(f, 0, SEEK_SET);
	fread(h, MAPHEADER_SIZE, 1, f);
}

void readVertexLightingData(vertexLightingData_s* vld, FILE* f)
{
	if(!vld || !f)return;

	fread(&vld->width, sizeof(u8), 1, f);
	fread(&vld->height, sizeof(u8), 1, f);

	vld->values=malloc(sizeof(u8)*vld->width*vld->height);
	fread(vld->values, sizeof(u8), vld->width*vld->height, f);
}

void readLightingData(room_s* r, lightingData_s* ld, FILE* f)
{
	if(!r || !ld || !f)return;

	ld->type=LIGHTMAP_DATA; //TEMP

	switch(ld->type)
	{
		case LIGHTMAP_DATA:
			initLightDataLM(ld, r->rectangles.num);

			readVect3Di(&ld->data.lightMap.lmSize,f);

			ld->data.lightMap.buffer=malloc(sizeof(u8)*ld->data.lightMap.lmSize.x*ld->data.lightMap.lmSize.y);
			if(!ld->data.lightMap.buffer)return;

			fread(ld->data.lightMap.buffer, sizeof(u8), ld->data.lightMap.lmSize.x*ld->data.lightMap.lmSize.y,f);
			fread(ld->data.lightMap.coords, sizeof(lightMapCoordinates_s), ld->size, f);

			int i=0;
			listCell_s* lc=r->rectangles.first;
			while(lc)
			{
				lc->data.lightData.lightMap=&ld->data.lightMap.coords[i++];
				lc=lc->next;
			}

			ld->data.lightMap.texture = textureCreateBuffer(ld->data.lightMap.buffer, ld->data.lightMap.lmSize.x, ld->data.lightMap.lmSize.y, GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 10);
			printf("texture : %p\n",ld->data.lightMap.texture);

			break;
		default:
			initLightDataVL(ld, r->rectangles.num);
			{
				int i;
				for(i=0;i<ld->size;i++)readVertexLightingData(&ld->data.vertexLighting[i],f);
			}
			{
				int i=r->rectangles.num-1;
				listCell_s* lc=r->rectangles.first;
				while(lc)
				{
					lc->data.lightData.vertex=&ld->data.vertexLighting[i--]; //stacking reverses the order...
					lc=lc->next;
				}
			}
			break;
	}
}

void readRoom(char* filename, room_s* r, u8 flags)
{
	if(!filename || !r)return;

	FILE* f=openFile(filename,"rb");
	if(!f)return;

	mapHeader_s h;
	readHeader(&h, f);

	//room data
	initRoom(r, 64*2, 64*2, vect3Df(-32*2,-32*2,0));

	fseek(f, h.rectanglesPosition, SEEK_SET);
	fread(&r->rectangles.num,sizeof(int),1,f);

	printf("RECTANGLE NUMBER %d\n", r->rectangles.num);
	
	readRectangles(r, f);

	//lighting stuff
	if(flags&MAP_READ_LIGHT)
	{
		fseek(f, h.lightPosition, SEEK_SET);
		readLightingData(r, &r->lightingData, f);
	}

	//entities
	if(flags&MAP_READ_ENTITIES)
	{
		fseek(f, h.entityPosition, SEEK_SET);
		readEntities(r, f);
	}

	//sludge stuff
	fseek(f, h.sludgePosition, SEEK_SET);
	readSludgeRectangles(f);

	// //info
	// if(flags&MAP_READ_INFO)
	// {
	// 	int l=strlen(filename);
	// 	filename[l-1]='i';
	// 	filename[l-2]='n';
	// 	filename[l-3]='i';
	// 	readMapInfo(filename);
	// }

	generateRoomGrid(r);
	generateRoomGeometry(r);
	
	fclose(f);
}
