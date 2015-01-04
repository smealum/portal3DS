#ifndef ROOM_H
#define ROOM_H

#include <3ds.h>
#include "utils/math.h"

typedef struct
{
	vect3Di_s position, size, normal;
	// material_s* material;
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
}rectangleVertex_s;

typedef struct
{
	// material_s** materials;
	vect3Df_s position;
	u16 width, height;
	rectangleList_s rectangles;
	int numVertices, numIndices[6];
	rectangleVertex_s* vertexBuffer;
	u16* indexBuffers[6];
}room_s;

void initRectangleList(rectangleList_s* p);
rectangle_s* addRectangle(rectangle_s r, rectangleList_s* p);
void popRectangle(rectangleList_s* p);

void readVect3Df(vect3Df_s* v, FILE* f, bool fp);
void readRectangle(rectangle_s* rec, FILE* f);
void readRectangles(room_s* r, FILE* f);

void initRoom(room_s* r, u16 w, u16 h, vect3Df_s p);
void generateRoomGeometry(room_s* r);
rectangle_s* addRoomRectangle(room_s* r, rectangle_s rec);
void removeRoomRectangles(room_s* r);

#endif
