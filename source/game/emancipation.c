#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <3ds.h>
#include "game/emancipation.h"
#include "game/cubes.h"
#include "physics/AAR.h"
#include "physics/OBB.h"
#include "gfx/gs.h"

#include "emancipation_vsh_shbin.h"

emancipator_s emancipators[NUMEMANCIPATORS];
emancipationGrid_s emancipationGrids[NUMEMANCIPATIONGRIDS];
md2_model_t gridModel;
texture_s gridTexture;
texture_s gridSurfaceTexture;
md2_instance_t gridInstance;

DVLB_s* emancipationDvlb;
shaderProgram_s emancipationProgram;

float emancipationRectangleData[] = {1.0f, 1.0f, 0.0f,
									1.0f, -1.0f, 0.0f,
									0.0f, -1.0f, 0.0f,
									1.0f, 1.0f, 0.0f,
									0.0f, -1.0f, 0.0f,
									0.0f, 1.0f, 0.0f,
									1.0f, 1.0f, 0.0f,
									0.0f, -1.0f, 0.0f,
									1.0f, -1.0f, 0.0f,
									0.0f, -1.0f, 0.0f,
									1.0f, 1.0f, 0.0f,
									0.0f, 1.0f, 0.0f};

u32* emancipationRectangleVertexData = NULL;

const u32 emancipationBaseAddr=0x14000000;

int emancipationUniformTextureDimensions;

void initEmancipation(void)
{
	int i;
	for(i=0;i<NUMEMANCIPATORS;i++)
	{
		emancipators[i].used=false;
	}
	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
	{
		emancipationGrids[i].used=false;
	}
	
	md2ReadModel(&gridModel, "grid.md2");
	textureLoad(&gridTexture, "balllauncher.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&gridSurfaceTexture, "grid.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)|GPU_TEXTURE_WRAP_S(GPU_REPEAT)|GPU_TEXTURE_WRAP_T(GPU_REPEAT), 0);
	md2InstanceInit(&gridInstance, &gridModel, &gridTexture);

	emancipationRectangleVertexData = linearAlloc(sizeof(emancipationRectangleData));
	memcpy(emancipationRectangleVertexData, emancipationRectangleData, sizeof(emancipationRectangleData));

	emancipationDvlb = DVLB_ParseFile((u32*)emancipation_vsh_shbin, emancipation_vsh_shbin_size);
	if(!emancipationDvlb)return;
	shaderProgramInit(&emancipationProgram);
	shaderProgramSetVsh(&emancipationProgram, &emancipationDvlb->DVLE[0]);

	emancipationUniformTextureDimensions = shaderInstanceGetUniformLocation(emancipationProgram.vertexShader, "textureDimensions");
}

void exitEmancipation(void)
{
	md2FreeModel(&gridModel);

	textureFree(&gridTexture);
	textureFree(&gridSurfaceTexture);

	// if(emancipationRectangleVertexData)linearFree(emancipationRectangleVertexData);

	// shaderProgramFree(&emancipationProgram);
	// DVLB_Free(emancipationDvlb);
}

float randFloat(float a, float b)
{
	return (((float)rand())/((float)RAND_MAX))*(b-a)+a;
}

void initEmancipator(emancipator_s* e, md2_instance_t* mi, vect3Df_s pos, float* m)
{
	if(!e)return;
	
	e->position=pos;
	e->modelInstance=*mi;
	if(m)memcpy(e->transformationMatrix,m,sizeof(float)*9);
	else{
		m=e->transformationMatrix;
		m[0]=m[4]=m[8]=1.0f;
		m[1]=m[2]=m[3]=0;
		m[5]=m[6]=m[7]=0;
	}
	e->counter=0;
	e->angle=0;
	e->velocity=vect3Df(randFloat(-1.0f, 1.0f),1.0f,randFloat(-1.0f, 1.0f));
	e->velocity=vdivf(vnormf(e->velocity),16);
	e->axis=vect3Df(randFloat(-1.0f, 1.0f),randFloat(-1.0f, 1.0f),randFloat(-1.0f, 1.0f));
	e->axis=vnormf(e->axis);
	
	e->used=true;
}

void createEmancipator(md2_instance_t* mi, vect3Df_s pos, float* m)
{
	if(!mi)return;
	int i;
	for(i=0;i<NUMEMANCIPATORS;i++)
	{
		if(!emancipators[i].used)
		{
			initEmancipator(&emancipators[i],mi,pos,m);
			return;
		}
	}
}

void updateEmancipator(emancipator_s* e)
{
	if(!e || !e->used)return;
	
	e->counter++;
	e->angle+=0.05f;
	e->position=vaddf(e->position,e->velocity);
	if(e->counter>BLACKENINGTIME+FADINGTIME)e->used=false;
}

void updateEmancipators(void)
{
	int i;
	for(i=0;i<NUMEMANCIPATORS;i++)
	{
		if(emancipators[i].used)
		{
			updateEmancipator(&emancipators[i]);
		}
	}
}

void drawEmancipator(emancipator_s* e)
{
	if(!e || !e->used)return;
	
	e->modelInstance.brightness=maxf(1.0f-(e->counter*1.0f)/BLACKENINGTIME,0.0f);
	e->modelInstance.alpha=(e->counter<BLACKENINGTIME)?(1.0f):(maxf(1.0f-((e->counter-BLACKENINGTIME)*1.0f)/FADINGTIME,0.0f));
	
	gsPushMatrix();
		gsSwitchRenderMode(md2GsMode);

		gsTranslate(e->position.x, e->position.y, e->position.z);

		// TODO : transpose ?
		gsMultMatrix3(e->transformationMatrix);
		
		gsRotateX(e->axis.x*e->angle);
		gsRotateY(e->axis.y*e->angle);
		gsRotateZ(e->axis.z*e->angle);

		md2InstanceDraw(&e->modelInstance);
	gsPopMatrix();
}

void drawEmancipators(void)
{
	int i;
	for(i=0;i<NUMEMANCIPATORS;i++)
	{
		if(emancipators[i].used)
		{
			drawEmancipator(&emancipators[i]);
		}
	}
}

void initEmancipationGrid(room_s* r, emancipationGrid_s* eg, vect3Di_s pos, float l, bool dir)
{
	if(!eg || !r)return;
	
	pos=vect3Di(pos.x, pos.y, pos.z);
	eg->position=convertRectangleVector(pos);
	eg->length=l;
	eg->direction=dir;
	eg->used=true;
}

void createEmancipationGrid(room_s* r, vect3Di_s pos, float l, bool dir)
{
	if(!r)return;
	int i;
	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
	{
		if(!emancipationGrids[i].used)
		{
			initEmancipationGrid(r, &emancipationGrids[i],pos,l,dir);
			return;
		}
	}
}

bool collideBoxGrid(emancipationGrid_s* eg, OBB_s* o)
{
	if(!o)return false;

	vect3Df_s p=o->position;
	vect3Df_s s;
	getBoxAABB(o,&s);

	vect3Df_s pos, sp;
	getEmancipationGridAAR(eg,&pos,&sp);

	return intersectAABBAAR(p, s, pos, sp);
}

void updateEmancipationGrid(player_s* pl, emancipationGrid_s* eg)
{
	if(!eg)return;
	
	vect3Df_s pos, sp;
	getEmancipationGridAAR(eg,&pos,&sp);
	if(intersectAABBAAR(pl->object.position, vect3Df(PLAYER_RADIUS,PLAYER_RADIUS*5,PLAYER_RADIUS), pos, sp))
	{
		resetPortals();
	}

	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		OBB_s* o = &objects[i];
		if(o->used && collideBoxGrid(eg, o))
		{
			createEmancipator(o->modelInstance, o->position, o->transformationMatrix);
			resetDispenserCube(o);
			printf("imma emancipate your shit\n");
		}
	}
}

void updateEmancipationGrids(player_s* pl)
{
	int i;
	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
	{
		if(emancipationGrids[i].used)
		{
			updateEmancipationGrid(pl, &emancipationGrids[i]);
		}
	}
}

// u16 counter=0;

void drawEmancipationGrid(emancipationGrid_s* eg)
{
	if(!eg)return;
	
	float l=fabs(eg->length);

	// TODO : animate surface texture
	
	gsPushMatrix();
		gsSwitchRenderMode(md2GsMode);

		gsTranslate(eg->position.x, eg->position.y, eg->position.z);

		if(eg->direction)gsRotateY(-(M_PI/2));
		if(eg->length<0)gsRotateY((M_PI/2)*2);
		md2InstanceDraw(&gridInstance);

		gsPushMatrix();
			gsTranslate(l,0,0);
			gsRotateY((M_PI/2)*2);
			md2InstanceDraw(&gridInstance);
		gsPopMatrix();

		gsSwitchRenderMode(-1);

		GPU_SetAttributeBuffers(
			1, // number of attributes
			(u32*)osConvertVirtToPhys(emancipationBaseAddr), // we use the start of linear heap as base since that's where all our buffers are located
			GPU_ATTRIBFMT(0, 3, GPU_FLOAT), // we want v0 (vertex position)
			0xFFE, // mask : we want v0
			0x0, // permutation : we use identity
			1, // number of buffers : we have one attribute per buffer
			(u32[]){(u32)emancipationRectangleVertexData-emancipationBaseAddr}, // buffer offsets (placeholders)
			(u64[]){0x0}, // attribute permutations for each buffer
			(u8[]){1} // number of attributes for each buffer
			);

		gsSetShader(&emancipationProgram);

		gsScale(l, 4.0f, 1.0f);
		gsUpdateTransformation();

		textureBind(&gridSurfaceTexture, GPU_TEXUNIT0);
		GPU_SetFloatUniform(GPU_VERTEX_SHADER, emancipationUniformTextureDimensions, (u32*)(float[]){0.0f, 0.0f, 0.7f, l/8}, 1);

		GPU_DrawArray(GPU_TRIANGLES, 12);
	gsPopMatrix();
}

void drawEmancipationGrids(void)
{
	int i;
	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
	{
		if(emancipationGrids[i].used)
		{
			drawEmancipationGrid(&emancipationGrids[i]);
		}
	}
}

void getEmancipationGridAAR(emancipationGrid_s* eg, vect3Df_s* pos, vect3Df_s* sp)
{
	if(!eg || !pos || !sp)return;
	
	*sp=vect3Df(eg->direction?(0):(eg->length/2),0,eg->direction?(eg->length/2):(0));
	*pos=vaddf(eg->position,*sp);
	sp->y=EMANCIPATIONGRIDHEIGHT/2;
	sp->x=abs(sp->x);sp->z=abs(sp->z);
}

bool emancipationGridBoxCollision(emancipationGrid_s* eg, OBB_s* o)
{
	if(!eg || !o)return false;
	
	vect3Df_s s, pos, sp;
	getBoxAABB(o,&s);
	getEmancipationGridAAR(eg,&pos,&sp);
	
	return intersectAABBAAR(o->position, s, pos, sp);
}

bool collideBoxEmancipationGrids(OBB_s* o)
{
	if(!o)return false;
	int i;
	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
	{
		if(emancipationGrids[i].used)
		{
			if(emancipationGridBoxCollision(&emancipationGrids[i], o))return true;
		}
	}
	return false;
}

// bool emancipationGridLineCollision(emancipationGrid_s* eg, vect3Df_s l, vect3Df_s v, float d)
// {
// 	if(!eg)return false;
	
// 	vect3Df_s pos, sp;
// 	getEmancipationGridAAR(eg,&pos,&sp);

// 	pos=vsubf(pos,sp);
// 	sp=vect3Df(sp.x*2,sp.y*2,sp.z*2);

// 	vect3Df_s n;
// 	if(!sp.x)n=vect3Df(1.0f,0,0);
// 	else if(!sp.y)n=vect3Df(0,1.0f,0);
// 	else n=vect3Df(0,0,1.0f);

// 	// NOGBA("%d %d %d vs %d %d %d",pos.x,pos.y,pos.z,l.x,l.y,l.z);
	
// 	return collideLineConvertedRectangle(n, pos, sp, l, v, d, NULL, NULL);
// }

// bool collideLineEmancipationGrids(vect3Df_s l, vect3Df_s v, int32 d)
// {
// 	int i;
// 	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
// 	{
// 		if(emancipationGrids[i].used)
// 		{
// 			if(emancipationGridLineCollision(&emancipationGrids[i], l, v, d))return true;
// 		}
// 	}
// 	return false;
// }
