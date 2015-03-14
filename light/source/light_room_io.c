#include <stdlib.h>
#include <stdio.h>
#include "light_room.h"
#include "light_room_io.h"

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

void readRoom(char* filename, room_s* r, u8 flags)
{
	if(!filename || !r)return;

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
	
	fclose(f);
}
