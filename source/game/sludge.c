#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/sludge.h"
#include "gfx/gs.h"

rectangleList_s sludgeRectangleList;
texture_s sludgeTexture;

rectangleVertex_s* sludgeVertexBuffer = NULL;
u16* sludgeIndexBuffer = NULL;
int sludgeNumVertices = 0;
int sludgeNumIndices = 0;

void initSludge(void)
{
	initRectangleList(&sludgeRectangleList);

	textureLoad(&sludgeTexture, "sludge.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)|GPU_TEXTURE_WRAP_S(GPU_REPEAT)|GPU_TEXTURE_WRAP_T(GPU_REPEAT), 0);

	sludgeVertexBuffer = NULL;
	sludgeIndexBuffer = NULL;
	sludgeNumVertices = 0;
	sludgeNumIndices = 0;
}

void exitSludge(void)
{
	while(sludgeRectangleList.num)popRectangle(&sludgeRectangleList);
	if(sludgeVertexBuffer)linearFree(sludgeVertexBuffer);
	if(sludgeIndexBuffer)linearFree(sludgeIndexBuffer);
}

void addSludgeRectangle(rectangle_s* rec)
{
	if(!rec)return;

	addRectangle(*rec, &sludgeRectangleList);
}

void generateSludgeRectangleGeometry(rectangle_s* rec)
{
	if(!rec)return;

	vect3Di_s texCoords[4];

	texCoords[0] = vect3Di(rec->position.x, rec->position.z, 0);
	texCoords[1] = vect3Di(rec->position.x+rec->size.x, rec->position.z, 0);
	texCoords[2] = vect3Di(rec->position.x+rec->size.x, rec->position.z+rec->size.z, 0);
	texCoords[3] = vect3Di(rec->position.x, rec->position.z+rec->size.z, 0);

	int i; for(i=0; i<4; i++)texCoords[i] = vmuli(texCoords[i], 16);

	generateRectangleGeometry(rec, texCoords, sludgeVertexBuffer, &sludgeNumVertices, sludgeIndexBuffer, &sludgeNumIndices);
}

void generateSludgeGeometry(void)
{
	listCell_s* lc=sludgeRectangleList.first;

	sludgeVertexBuffer = linearAlloc(sizeof(rectangleVertex_s) * sludgeRectangleList.num * 4);
	sludgeIndexBuffer = linearAlloc(sizeof(u16) * sludgeRectangleList.num * 6);

	while(lc)
	{
		generateSludgeRectangleGeometry(&lc->data);
		lc=lc->next;
	}
}

int sludgeAnimationCounter=0;

extern shaderProgram_s roomProgram;
extern int roomUniformTextureDimensions;
extern const u32 roomBaseAddr;

void drawSludge(room_s* r)
{
	if(!r)return;

	//TODO : sludge texture animation

	gsSwitchRenderMode(-1);

	gsSetShader(&roomProgram);

	GPU_SetAttributeBuffers(
		3, // number of attributes
		(u32*)osConvertVirtToPhys(roomBaseAddr), // we use the start of linear heap as base since that's where all our buffers are located
		GPU_ATTRIBFMT(0, 3, GPU_SHORT)|GPU_ATTRIBFMT(1, 2, GPU_SHORT)|GPU_ATTRIBFMT(2, 2, GPU_SHORT), // we want v0, v1 and v2
		0xFF8, // mask : we want v0, v1 and v2
		0x210, // permutation : we use identity
		1, // number of buffers : we have one attribute per buffer
		(u32[]){(u32)sludgeVertexBuffer-roomBaseAddr}, // buffer offsets (placeholders)
		(u64[]){0x210}, // attribute permutations for each buffer
		(u8[]){3} // number of attributes for each buffer
		);

	gsPushMatrix();

		gsScale(TILESIZE_FLOAT*2, HEIGHTUNIT_FLOAT, TILESIZE_FLOAT*2);
		gsTranslate(0.0f, -HEIGHTUNIT_FLOAT*0.6f, 0.0f);

		gsUpdateTransformation();

		// textureBind(&sludgeTexture, GPU_TEXUNIT0);
		// GPU_SetFloatUniform(GPU_VERTEX_SHADER, roomUniformTextureDimensions, (u32*)(float[]){0.0f, 0.0f, 1.0f / sludgeTexture.height, 1.0f / sludgeTexture.width}, 1);
		// GPU_DrawElements(GPU_UNKPRIM, (u32*)((u32)sludgeIndexBuffer-roomBaseAddr), sludgeNumIndices);

	gsPopMatrix();
}

bool sludgeAABBCollision(rectangle_s* rec, vect3Df_s p, vect3Df_s s)
{
	if(!rec)return false;

	vect3Df_s pos=convertRectangleVector(rec->position);
	vect3Df_s sp=convertRectangleVector(rec->size);

	if(sp.x<0){pos.x+=sp.x;sp.x=-sp.x;}
	if(sp.z<0){pos.z+=sp.z;sp.z=-sp.z;}

	sp.x/=2; sp.z/=2;
	pos.x+=sp.x; pos.z+=sp.z;
	pos.y-=SLUDGEMARGIN;

	return intersectAABBAAR(p, s, pos, sp);
}

bool collideAABBSludge(vect3Df_s p, vect3Df_s s)
{
	listCell_s* lc=sludgeRectangleList.first;

	while(lc)
	{
		if(sludgeAABBCollision(&lc->data, p, s))return true;
		lc=lc->next;
	}
	return false;
}

bool collideBoxSludge(OBB_s* o)
{
	if(!o)return false;

	vect3Df_s p=o->position;
	vect3Df_s s;
	getBoxAABB(o,&s);

	return collideAABBSludge(p,s);
}
