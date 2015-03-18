#ifndef __RECTANGLE9__
#define __RECTANGLE9__

#include "light_lightmap.h"

typedef enum
{
	EMPTY,
	RECTANGLE,
	HORIZONTAL,
	VERTICAL
}treeNode_type;

typedef struct treeNode_s
{
	struct treeNode_s* son[2];
	treeNode_type type;
	short data;
}treeNode_s;

typedef struct
{
	treeNode_s* root;
	short width, height;
}tree_s;

typedef struct
{
	vect3Di_s position, size;
	lightMapCoordinates_s* real;
	bool rot;
}rectangle2D_s;

typedef struct listCell2D_s
{
	rectangle2D_s data;
	struct listCell2D_s* next;
}listCell2D_s;

typedef struct
{
	listCell2D_s* first;
	int surface;
	int num;
}rectangle2DList_s;

static inline vect3Di_s vect2(s32 x, s32 y)
{
	return (vect3Di_s){x,y,0};
}

void initRectangle2DList(rectangle2DList_s* l);
void insertRectangle2DList(rectangle2DList_s* l, rectangle2D_s rec);
void packRectangles(rectangle2DList_s* l, short w, short h);
bool packRectanglesSize(rectangle2DList_s* l, short* w, short* h);
void freeRectangle2DList(rectangle2DList_s* l);

void fillRectangle(u8* data, int w, int h, vect3Di_s* pos, vect3Di_s* size, u8 mask);
void getMaxRectangle(u8* data, u8 val, int w, int h, vect3Di_s* pos, vect3Di_s* size);
// bool collideLineRectangle(rectangle_s* rec, vect3D o, vect3D v, int32 d, int32* kk, vect3D* ip);
// bool collideLineConvertedRectangle(vect3D n, vect3D p, vect3D s, vect3D o, vect3D v, int32 d, int32* kk, vect3D* ip);
// vect3D getClosestPointRectangleStruct(rectangle_s* rec, vect3D o);
// vect3D getClosestPointRectangle(vect3D p, vect3D s, vect3D o);

// materialSlice_s* bindMaterial(material_s* m, rectangle_s* rec, int32* t, vect3D* v,bool DL);

#endif
