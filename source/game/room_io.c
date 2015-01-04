#include <stdlib.h>
#include <stdio.h>
#include <3ds.h>
#include "game/room_io.h"

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
		fread(v, sizeof(s32), 3, f);
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
	readVect3Di(&rec->normal, f);

	fread(&rec->portalable, sizeof(bool), 1, f);
	
	u16 mid=0; fread(&mid,sizeof(u16),1,f);

	// //TEMP ?
	// if(rec->portalable)rec->material=getMaterial(1);
	// else rec->material=getMaterial(2);
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

void readRoom(char* filename, room_s* r, u8 flags)
{
	if(!filename || !r)return;

	FILE* f=fopen(filename,"rb");
	if(!f)return;

	mapHeader_s h;
	readHeader(&h, f);

	//room data
	initRoom(r, 64*2, 64*2, vect3Df(-32*2,-32*2,0));

	fseek(f, h.rectanglesPosition, SEEK_SET);
	fread(&r->rectangles.num,sizeof(int),1,f);

	printf("RECTANGLE NUMBER %d", r->rectangles.num);
	
	readRectangles(r, f);

	// //lighting stuff
	// if(flags&(1))
	// {
	// 	fseek(f, h.lightPosition, SEEK_SET);
	// 		readLightingData(r, &r->lightingData, f);
	// }

	// //entities
	// if(flags&(1<<1))
	// {
	// 	fseek(f, h.entityPosition, SEEK_SET);
	// 		readEntities(f);
	// }

	// //sludge stuff
	// fseek(f, h.sludgePosition, SEEK_SET);
	// 	readSludgeRectangles(f);

	// //info
	// if(flags&(1<<7))
	// {
	// 	int l=strlen(filename);
	// 	filename[l-1]='i';
	// 	filename[l-2]='n';
	// 	filename[l-3]='i';
	// 	readMapInfo(filename);
	// }

	generateRoomGeometry(r);
	
	fclose(f);
}
