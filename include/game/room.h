#ifndef ROOM_H
#define ROOM_H

#include <3ds.h>
#include "utils/math.h"
#include "game/material.h"

#define TILESIZE (2)
#define TILESIZE_FLOAT ((float)TILESIZE)
#define HEIGHTUNIT (1)
#define HEIGHTUNIT_FLOAT ((float)HEIGHTUNIT)

typedef struct
{
	vect3Di_s position, size;
	vect3Df_s normal;
	material_s* material;
	bool portalable, hide, touched, collides;
}rectangle_s;

typedef struct listCell_s
{
	rectangle_s data;
	struct listCell_s* next;
}listCell_s;

typedef struct
{
	listCell_s* first;
	int num;
}rectangleList_s;

typedef struct
{
	s16 x, y, z;
	s16 u, v;
}rectangleVertex_s;

typedef struct
{
	// material_s** materials;
	vect3Df_s position;
	u16 width, height;
	rectangleList_s rectangles;
	int numVertices;
	rectangleVertex_s* vertexBuffer;
	int numIndexBuffers;
	int* numIndices;
	texture_s** indexBufferTextures;
	u16** indexBuffers;
}room_s;

void roomInit();
void roomExit();

void initRectangleList(rectangleList_s* p);
rectangle_s* addRectangle(rectangle_s r, rectangleList_s* p);
void popRectangle(rectangleList_s* p);
void getTextureCoordSlice(materialSlice_s* ms, rectangle_s* rec, vect3Di_s* v);
void getMaterialTextureCoord(rectangle_s* rec, vect3Di_s* v);
texture_s* getRectangleTexture(rectangle_s* rec);

void readVect3Df(vect3Df_s* v, FILE* f, bool fp);
void readRectangle(rectangle_s* rec, FILE* f);
void readRectangles(room_s* r, FILE* f);

void initRoom(room_s* r, u16 w, u16 h, vect3Df_s p);
void generateRoomGeometry(room_s* r);
rectangle_s* addRoomRectangle(room_s* r, rectangle_s rec);
void removeRoomRectangles(room_s* r);
void drawRoom(room_s* r);

rectangle_s* collideLineMapClosest(room_s* r, rectangle_s* rec, vect3Df_s l, vect3Df_s u, float d, vect3Df_s* i, float* lk);

#endif
