#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/room.h"

void initRectangleList(rectangleList_s* p)
{
	p->first=NULL;
	p->num=0;
}

rectangle_s* addRectangle(rectangle_s r, rectangleList_s* p)
{
	listCell_s* pc=(listCell_s*)malloc(sizeof(listCell_s));
	pc->next=p->first;
	pc->data=r;
	p->first=pc;
	p->num++;
	return &pc->data;
}

void popRectangle(rectangleList_s* p)
{
	p->num--;
	if(p->first)
	{
		listCell_s* pc=p->first;
		p->first=pc->next;
		free(pc);
	}
}

rectangle_s* addRoomRectangle(room_s* r, rectangle_s rec)
{
	if((!rec.size.x && (!rec.size.z || !rec.size.y)) || (!rec.size.y && !rec.size.z))return NULL;
	return addRectangle(rec, &r->rectangles);
}

void removeRoomRectangles(room_s* r)
{
	while(r->rectangles.num)popRectangle(&r->rectangles);
}

void initRoom(room_s* r, u16 w, u16 h, vect3Df_s p)
{
	if(!r)return;
	
	r->width=w;
	r->height=h;
	r->position=p;
	
	initRectangleList(&r->rectangles);

	r->vertexBuffer=NULL;
	int i;for(i=0; i<6; i++)r->indexBuffers[i]=NULL;
	
	// if(r->height && r->width)
	// {
	// 	r->materials=malloc(r->height*r->width*sizeof(material_s*));
	// 	int i;for(i=0;i<r->height*r->width;i++){r->materials[i]=NULL;}
	// }else r->materials=NULL;
}

u8 getNormalOrientation(vect3Di_s v)
{
	if(v.x>0)return 0;
	if(v.x<0)return 1;
	if(v.y>0)return 2;
	if(v.y<0)return 3;
	if(v.z>0)return 4;
	if(v.z<0)return 5;
	return 0;
}

void getRectangleUnitVectors(rectangle_s* rec, vect3Di_s* v1, vect3Di_s* v2)
{
	if(!rec)return;

	if(rec->size.x)
	{
		if(v1)*v1=vect3Di(rec->size.x, 0, 0);
		if(v2)
		{
			if(rec->size.y)*v2=vect3Di(0, rec->size.y, 0);
			else *v2=vect3Di(0, 0, rec->size.z);
		}
	}else{
		if(v1)*v1=vect3Di(0, rec->size.y, 0);
		if(v2)*v2=vect3Di(0, 0, rec->size.z);
	}
}

int findRoomVertex(room_s* r, vect3Di_s v)
{
	int i;
	for(i=0; i<r->numVertices; i++)if(r->vertexBuffer[i].x==v.x && r->vertexBuffer[i].y==v.y && r->vertexBuffer[i].z==v.z)return i;
	return -1;
}

void generateRoomGeometry(room_s* r)
{
	if(!r)return;

	int i;

	if(r->vertexBuffer)linearFree(r->vertexBuffer);
	for(i=0; i<6; i++)if(r->vertexBuffer)linearFree(r->indexBuffers[i]);
	r->vertexBuffer=NULL;
	for(i=0; i<6; i++)r->indexBuffers[i]=NULL;

	rectangleVertex_s* tmpVertex=malloc(sizeof(rectangleVertex_s)*r->rectangles.num*4);
	if(!tmpVertex)return;

	{
		u16 indexCnt[6]={0,0,0,0,0,0};
		listCell_s* l=r->rectangles.first;
		while(l)
		{
			indexCnt[getNormalOrientation(l->data.normal)]++;
			l=l->next;
		}
		for(i=0; i<6; i++)r->indexBuffers[i]=linearAlloc(sizeof(u16)*6*indexCnt[i]);
	}

	{
		r->vertexBuffer=tmpVertex;
		r->numVertices=0;
		for(i=0; i<6; i++)r->numIndices[i]=0;
		listCell_s* l=r->rectangles.first;
		while(l)
		{
			u8 o=getNormalOrientation(l->data.normal);
			vect3Di_s v1, v2;
			getRectangleUnitVectors(&l->data, &v1, &v2);

			vect3Di_s vertices[4] = {l->data.position, vaddi(l->data.position, v1), vaddi(l->data.position, v2), vaddi(l->data.position, l->data.size)};
			int vertexIndices[4] = {findRoomVertex(r, vertices[0]), findRoomVertex(r, vertices[1]), findRoomVertex(r, vertices[2]), findRoomVertex(r, vertices[3])};

			for(i=0; i<4; i++)
			{
				if(vertexIndices[i]<0)
				{
					vertexIndices[i] = r->numVertices++;
					r->vertexBuffer[vertexIndices[i]] = (rectangleVertex_s){vertices[i].x, vertices[i].y, vertices[i].z};
				}
			}

			r->indexBuffers[o][r->numIndices[o]++] = vertexIndices[0];
			r->indexBuffers[o][r->numIndices[o]++] = vertexIndices[1];
			r->indexBuffers[o][r->numIndices[o]++] = vertexIndices[2];

			r->indexBuffers[o][r->numIndices[o]++] = vertexIndices[1];
			r->indexBuffers[o][r->numIndices[o]++] = vertexIndices[3];
			r->indexBuffers[o][r->numIndices[o]++] = vertexIndices[2];

			l=l->next;
		}
	}

	r->vertexBuffer=linearAlloc(sizeof(rectangleVertex_s)*r->numVertices);
	memcpy(r->vertexBuffer, tmpVertex, sizeof(rectangleVertex_s)*r->numVertices);

	free(tmpVertex);
}
