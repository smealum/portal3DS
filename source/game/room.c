#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>

#include "game/room.h"
#include "gfx/gs.h"

#include "physics/physics.h"

#include "room_vsh_shbin.h"

DVLB_s* roomDvlb;
shaderProgram_s roomProgram;
const u32 roomBaseAddr=0x14000000;

int roomUniformTextureDimensions;

void roomInit()
{
	roomDvlb = DVLB_ParseFile((u32*)room_vsh_shbin, room_vsh_shbin_size);

	if(!roomDvlb)return;

	shaderProgramInit(&roomProgram);
	shaderProgramSetVsh(&roomProgram, &roomDvlb->DVLE[0]);

	roomUniformTextureDimensions = shaderInstanceGetUniformLocation(roomProgram.vertexShader, "textureDimensions");
}

void roomExit()
{
	shaderProgramFree(&roomProgram);
	DVLB_Free(roomDvlb);
}

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
	r->indexBuffers=NULL;
	
	// if(r->height && r->width)
	// {
	// 	r->materials=malloc(r->height*r->width*sizeof(material_s*));
	// 	int i;for(i=0;i<r->height*r->width;i++){r->materials[i]=NULL;}
	// }else r->materials=NULL;
}

void transferRoomRectangles(room_s* r)
{
	if(!r)return;
	listCell_s *lc=r->rectangles.first;
	int i=0;
	while(lc)
	{
		// lc->data.AARid=
		physicsCreateAar(NULL, convertRectangleVector(lc->data.position), convertRectangleVector(lc->data.size), vmulf(lc->data.normal, -1.0f));
		lc=lc->next;
		i++;
	}
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

vect3Df_s convertRectangleVector(vect3Di_s v)
{
	return vect3Df(v.x*TILESIZE_FLOAT*2, v.y*HEIGHTUNIT_FLOAT, v.z*TILESIZE_FLOAT*2);
}

bool collideLineRectangle(rectangle_s* rec, vect3Df_s o, vect3Df_s v, float d, float* kk, vect3Df_s* ip)
{
	if(!rec)return false;
	vect3Df_s n=vect3Df(fabs(rec->normal.x),fabs(rec->normal.y),fabs(rec->normal.z));
	float p1=vdotf(v,n);
	if(fabs(p1)>0.001f)
	{
		vect3Df_s p = convertRectangleVector(rec->position);
		vect3Df_s s = convertRectangleVector(rec->size);
		
		float p2=vdotf(vsubf(p,o),n);

		float k=p2/p1;
		s8 sign=((s.x>0)^(s.y<0)^(s.z>0)^(p1<0))?(-1):(1);
		if(kk)
		{
			*kk=k+sign;
		}
		if(k<0 || k>d){return false;}
		vect3Df_s i=vaddf(o,vmulf(v,k));
		if(ip)*ip=i;
		i=vsubf(i,p);
		
		bool r=true;
		if(s.x)
		{
			if(s.x>0)r=r&&i.x<s.x&&i.x>=0;
			else r=r&&i.x>s.x&&i.x<=0;
		}
		if(s.y)
		{
			if(s.y>0)r=r&&i.y<s.y&&i.y>=0;
			else r=r&&i.y>s.y&&i.y<=0;
		}
		if(s.z)
		{
			if(s.z>0)r=r&&i.z<s.z&&i.z>=0;
			else r=r&&i.z>s.z&&i.z<=0;
		}
		return r;
	}
	return false;
}

rectangle_s* collideLineMapClosest(room_s* r, rectangle_s* rec, vect3Df_s l, vect3Df_s u, float d, vect3Df_s* i, float* lk)
{
	if(!r)return NULL;
	listCell_s *lc=r->rectangles.first;
	vect3Df_s v;
	float lowestK=d;
	rectangle_s* hit=NULL;
	while(lc)
	{
		if(&lc->data!=rec && lc->data.collides)
		{
			float k;
			if(collideLineRectangle(&lc->data,l,u,lowestK,&k,&v))
			{
				if(k<lowestK)
				{
					if(i)*i=v;
					if(lk)*lk=k;
					lowestK=k;
					hit=&lc->data;
				}
			}
		}
		lc=lc->next;
	}
	return hit;
}

int findTexture(texture_s* t, texture_s** tb, int n)
{
	if(!t || !tb)return -1;
	int i;
	for(i=0;i<n;i++)if(t==tb[i])return i;
	return -1;
}

void generateRoomGeometry(room_s* r)
{
	if(!r)return;

	if(r->vertexBuffer)linearFree(r->vertexBuffer);
	if(r->numIndices)linearFree(r->numIndices);
	if(r->indexBufferTextures)linearFree(r->indexBufferTextures);
	if(r->indexBuffers)
	{
		int i; for(i=0; i<r->numIndexBuffers; i++)linearFree(r->indexBuffers[i]);
		free(r->indexBuffers);
	}
	r->vertexBuffer=NULL;
	r->numIndices=NULL;
	r->indexBufferTextures=NULL;
	r->indexBuffers=NULL;
	r->numIndexBuffers=0;

	rectangleVertex_s* tmpVertex=malloc(sizeof(rectangleVertex_s)*r->rectangles.num*4);
	if(!tmpVertex)return;

	{
		texture_s** tmpTextures=calloc(TEXTURES_NUM, sizeof(texture_s*));
		int* tmpNumIndices=calloc(TEXTURES_NUM, sizeof(int));

		r->numIndexBuffers=0;
		listCell_s* l=r->rectangles.first;
		while(l)
		{
			texture_s* t=getRectangleTexture(&l->data);
			int i=findTexture(t, tmpTextures, r->numIndexBuffers);
			if(i<0)tmpTextures[i=(r->numIndexBuffers++)]=t;
			tmpNumIndices[i]++;
			l=l->next;
		}
		r->indexBuffers=calloc(r->numIndexBuffers, sizeof(u16*));
		r->numIndices=calloc(r->numIndexBuffers, sizeof(int));
		r->indexBufferTextures=calloc(r->numIndexBuffers, sizeof(texture_s*));
		int i; for(i=0; i<r->numIndexBuffers; i++)r->indexBuffers[i]=linearAlloc(sizeof(u16)*6*tmpNumIndices[i]);

		free(tmpTextures);
		free(tmpNumIndices);
	}

	{
		r->numIndexBuffers=0;

		r->vertexBuffer=tmpVertex;
		r->numVertices=0;
		listCell_s* l=r->rectangles.first;
		while(l)
		{
			texture_s* t=getRectangleTexture(&l->data);
			int b=findTexture(t, r->indexBufferTextures, r->numIndexBuffers);
			if(b<0)
			{
				r->indexBufferTextures[b=r->numIndexBuffers++]=t;
			}

			vect3Di_s v1, v2;
			getRectangleUnitVectors(&l->data, &v1, &v2);

			int vertexIndices[4];
			vect3Di_s texCoords[4];
			vect3Di_s vertices[4] = {l->data.position, vaddi(l->data.position, v1), vaddi(l->data.position, l->data.size), vaddi(l->data.position, v2)};
			
			getMaterialTextureCoord(&l->data, texCoords);

			// printf("%f %f, %f %f\n", (texCoords[0].x)*0.0078125, (texCoords[0].y)*0.0078125, (texCoords[2].x)*0.0078125, (texCoords[2].y)*0.0078125);

			int i;
			for(i=0; i<4; i++)
			{
				vertexIndices[i] = r->numVertices++;
				r->vertexBuffer[vertexIndices[i]] = (rectangleVertex_s){vertices[i].x, vertices[i].y, vertices[i].z, (texCoords[i].x), (texCoords[i].y)};
			}

			r->indexBuffers[b][r->numIndices[b]++] = vertexIndices[0];
			r->indexBuffers[b][r->numIndices[b]++] = vertexIndices[1];
			r->indexBuffers[b][r->numIndices[b]++] = vertexIndices[3];

			r->indexBuffers[b][r->numIndices[b]++] = vertexIndices[1];
			r->indexBuffers[b][r->numIndices[b]++] = vertexIndices[2];
			r->indexBuffers[b][r->numIndices[b]++] = vertexIndices[3];

			l=l->next;
		}
	}

	r->vertexBuffer=linearAlloc(sizeof(rectangleVertex_s)*r->numVertices);
	memcpy(r->vertexBuffer, tmpVertex, sizeof(rectangleVertex_s)*r->numVertices);

	free(tmpVertex);
}

void drawRoom(room_s* r)
{
	if(!r)return;

	gsSetShader(&roomProgram);

	GPU_SetAttributeBuffers(
		2, // number of attributes
		(u32*)osConvertVirtToPhys(roomBaseAddr), // we use the start of linear heap as base since that's where all our buffers are located
		GPU_ATTRIBFMT(0, 3, GPU_SHORT)|GPU_ATTRIBFMT(1, 2, GPU_SHORT), // we want v0 and v1
		0xFF8, // mask : we want v0 and v1
		0x10, // permutation : we use identity
		1, // number of buffers : we have one attribute per buffer
		(u32[]){(u32)r->vertexBuffer-roomBaseAddr}, // buffer offsets (placeholders)
		(u64[]){0x10}, // attribute permutations for each buffer
		(u8[]){2} // number of attributes for each buffer
		);

	gsPushMatrix();

		gsScale(TILESIZE_FLOAT*2, HEIGHTUNIT_FLOAT, TILESIZE_FLOAT*2);

		gsUpdateTransformation();

		int i;
		for(i=0; i<r->numIndexBuffers; i++)
		{
			textureBind(r->indexBufferTextures[i], GPU_TEXUNIT0);
			GPU_SetFloatUniform(GPU_VERTEX_SHADER, roomUniformTextureDimensions, (u32*)(float[]){0.0f, 0.0f, 1.0f / r->indexBufferTextures[i]->height, 1.0f / r->indexBufferTextures[i]->width}, 1);
			GPU_DrawElements(GPU_UNKPRIM, (u32*)((u32)r->indexBuffers[i]-roomBaseAddr), r->numIndices[i]);
		}

	gsPopMatrix();
}
