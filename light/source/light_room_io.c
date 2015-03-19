#include <stdlib.h>
#include <stdio.h>
#include "light_room.h"
#include "light_room_io.h"

light_s lights[NUMLIGHTS];

void initLights()
{
	int i;
	for(i=0;i<NUMLIGHTS;i++)lights[i].used=false;
}

void createLight(vect3Di_s pos, float intensity)
{
	int i;
	for(i=0;i<NUMLIGHTS;i++)
	{
		if(!lights[i].used)
		{
			lights[i].position=convertRectangleVector(pos);
			lights[i].intensity=intensity;
			lights[i].used=true;
			return;
		}
	}
}

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

void readHeader(mapHeader_s* h, FILE* f)
{
	if(!h || !f)return;
	fseek(f, 0, SEEK_SET);
	fread(h, MAPHEADER_SIZE, 1, f);
}

void readEntity(room_s* r, u8 i, FILE* f)
{
	if(!f)return;
	u8 type=0, dir=0; vect3Di_s v;
	fread(&type, sizeof(u8), 1, f);
	readVect3Di(&v, f);
	fread(&dir, sizeof(u8), 1, f);
	switch(type)
	{
		case 0:
			//energy ball catcher
			{
				vect3Di_s p; readVect3Di(&p,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
			}
			break;
		case 1:
			//energy ball launcher
			{
				vect3Di_s p; readVect3Di(&p,f);
			}
			break;
		case 2:
			//timed button
			{
				vect3Di_s p; readVect3Di(&p,f);
				u8 d; fread(&d, sizeof(u8), 1, f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
			}
			break;
		case 3:
			//pressure button
			{
				vect3Di_s p; readVect3Di(&p,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
			}
			break;
		case 4:
			//turret
			{
				vect3Di_s p; readVect3Di(&p,f);
				u8 d; fread(&d, sizeof(u8), 1, f);
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
			}
			break;
		case 8:
			//emancipation grid
			{
				s32 l; fread(&l,sizeof(s32),1,f);
				vect3Di_s p; readVect3Di(&p,f);
			}
			break;
		case 9:
			//platform
			{
				vect3Di_s p1, p2;
				readVect3Di(&p1,f);
				readVect3Di(&p2,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
			}
			return;
		case 10:
			//door
			{
				vect3Di_s p; readVect3Di(&p,f);
				u8 orientation; fread(&orientation, sizeof(u8), 1, f);
			}
			break;
		case 11:
			//light
			{
				vect3Di_s p; readVect3Di(&p,f);
				printf("LIGHT ! %d %d %d\n",p.x,p.y,p.z);
				createLight(p, TILESIZE_FLOAT*2*16);
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
			}
			return;
		case 14:
			//wall door (exit)
			{
				vect3Di_s p; readVect3Di(&p,f);
				u8 o; fread(&o,sizeof(u8),1,f);
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
}

void readRoom(char* filename, room_s* r, u8 flags)
{
	if(!filename || !r)return;

	initLights();

	FILE* f=fopen(filename,"rb");
	if(!f){printf("failure : couldn't open file %s\n", filename);return;}

	mapHeader_s h;
	readHeader(&h, f);

	//room data
	initRoom(r, 64*2, 64*2, vect3Df(-32*2,-32*2,0));

	fseek(f, h.rectanglesPosition, SEEK_SET);
	fread(&r->rectangles.num,sizeof(int),1,f);

	printf("RECTANGLE NUMBER %d\n", r->rectangles.num);
	
	readRectangles(r, f);

	//entities
	fseek(f, h.entityPosition, SEEK_SET);
	readEntities(r, f);
	
	fclose(f);
}
