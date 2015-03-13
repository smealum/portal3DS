#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <3ds.h>
#include "game/emancipation.h"
#include "gfx/gs.h"

emancipator_s emancipators[NUMEMANCIPATORS];
emancipationGrid_s emancipationGrids[NUMEMANCIPATIONGRIDS];
md2_model_t gridModel;
texture_s gridTextures[6];

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
	textureLoad(&gridTextures[0], "gridcolor1.pcx", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&gridTextures[1], "gridcolor2.pcx", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&gridTextures[2], "gridcolor3.pcx", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&gridTextures[3], "gridcolor4.pcx", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&gridTextures[4], "gridcolor5.pcx", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&gridTextures[5], "gridcolor6.pcx", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
}

void exitEmancipation(void)
{
	md2FreeModel(&gridModel);

	textureFree(&gridTextures[0]);
	textureFree(&gridTextures[1]);
	textureFree(&gridTextures[2]);
	textureFree(&gridTextures[3]);
	textureFree(&gridTextures[4]);
	textureFree(&gridTextures[5]);
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
	e->velocity=vdivf(vnormf(e->velocity),256);
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
	
	// TODO : blackout + alpha fade

	// u8 shade=max(31-(e->counter*31)/BLACKENINGTIME,0);
	// u8 alpha=(e->counter<BLACKENINGTIME)?(31):(max(31-((e->counter-BLACKENINGTIME)*31)/FADINGTIME,1));
	
	gsPushMatrix();
		gsSwitchRenderMode(md2GsMode);

		gsTranslate(e->position.x, e->position.y, e->position.z);

		// TODO : transpose ?
		gsMultMatrix(e->transformationMatrix);
		
		// gsRotateY(e->angle, e->axis.x, e->axis.y, e->axis.z);

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

// void initEmancipationGrid(room_s* r, emancipationGrid_s* eg, vect3Df_s pos, int32 l, bool dir)
// {
// 	if(!eg || !r)return;
	
// 	pos=vect(pos.x+r->position.x, pos.y, pos.z+r->position.y);
// 	eg->position=convertVect(pos);
// 	eg->length=l;
// 	eg->direction=dir;
	
// 	eg->used=true;
// }

// void createEmancipationGrid(room_s* r, vect3Df_s pos, int32 l, bool dir)
// {
// 	if(!r)r=getPlayer()->currentRoom;
// 	int i;
// 	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
// 	{
// 		if(!emancipationGrids[i].used)
// 		{
// 			initEmancipationGrid(r, &emancipationGrids[i],pos,l,dir);
// 			return;
// 		}
// 	}
// }

// void updateEmancipationGrid(emancipationGrid_s* eg)
// {
// 	if(!eg)return;
	
// 	player_s* pl=getPlayer();
// 	vect3Df_s pos, sp;
// 	getEmancipationGridAAR(eg,&pos,&sp);
// 	if(intersectAABBAAR(pl->object->position, vect(PLAYERRADIUS,PLAYERRADIUS*5,PLAYERRADIUS), pos, sp))
// 	{
// 		resetPortals();
// 	}
// }

// void updateEmancipationGrids(void)
// {
// 	int i;
// 	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
// 	{
// 		if(emancipationGrids[i].used)
// 		{
// 			updateEmancipationGrid(&emancipationGrids[i]);
// 		}
// 	}
// }

// u16 counter=0;

// void drawEmancipationGrid(emancipationGrid_s* eg)
// {
// 	if(!eg)return;
	
// 	int32 l=abs(eg->length);
	
// 	glPushMatrix();
// 		glTranslatef32(eg->position.x,eg->position.y,eg->position.z);
// 		if(eg->direction)glRotateYi(-8192);
// 		if(eg->length<0)glRotateYi(8192*2);
// 		renderModelFrameInterp(0, 0, 0, &gridModel, POLY_ALPHA(31) | POLY_ID(20) | POLY_CULL_FRONT | POLY_TOON_HIGHLIGHT | POLY_FORMAT_LIGHT0 | POLY_FOG, false, NULL, RGB15(31,31,31));
// 		glPushMatrix();
// 			glTranslatef32(l,0,0);
// 			glRotateYi(8192*2);
// 			renderModelFrameInterp(0, 0, 0, &gridModel, POLY_ALPHA(31) | POLY_ID(20) | POLY_CULL_FRONT | POLY_TOON_HIGHLIGHT | POLY_FORMAT_LIGHT0 | POLY_FOG, false, NULL, RGB15(31,31,31));
// 		glPopMatrix(1);
		
// 		applyMTL(gridMtl);
// 		bindPaletteAddr(gridPalettes[(((counter++)/4)%6)]);
// 		GFX_COLOR=RGB15(31,31,31);
// 		glPolyFmt(POLY_ALPHA(12) | POLY_ID(21) | POLY_CULL_NONE | POLY_FOG);
// 		glScalef32(l,EMANCIPATIONGRIDHEIGHT/2,inttof32(1));
// 		glBegin(GL_QUADS);
// 			GFX_TEX_COORD = TEXTURE_PACK(0*16, 0*16);
// 			glVertex3v16(0, inttof32(1), 0);
// 			GFX_TEX_COORD = TEXTURE_PACK(l*32*16/TILESIZE, 0*16);
// 			glVertex3v16(inttof32(1), inttof32(1), 0);
// 			GFX_TEX_COORD = TEXTURE_PACK(l*32*16/TILESIZE, 256*16);
// 			glVertex3v16(inttof32(1), -inttof32(1), 0);
// 			GFX_TEX_COORD = TEXTURE_PACK(0*16, 256*16);
// 			glVertex3v16(0, -inttof32(1), 0);
// 	glPopMatrix(1);
// }

// void drawEmancipationGrids(void)
// {
// 	int i;
// 	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
// 	{
// 		if(emancipationGrids[i].used)
// 		{
// 			drawEmancipationGrid(&emancipationGrids[i]);
// 		}
// 	}
// }

// void getEmancipationGridAAR(emancipationGrid_s* eg, vect3Df_s* pos, vect3Df_s* sp)
// {
// 	if(!eg || !pos || !sp)return;
	
// 	*sp=vect(eg->direction?(0):(eg->length/2),0,eg->direction?(eg->length/2):(0));
// 	*pos=addVect(eg->position,*sp);
// 	sp->y=EMANCIPATIONGRIDHEIGHT/2;
// 	sp->x=abs(sp->x);sp->z=abs(sp->z);
// }

// bool emancipationGridBoxCollision(emancipationGrid_s* eg, OBB_s* o)
// {
// 	if(!eg || !o)return false;
	
// 	vect3Df_s s, pos, sp;
// 	getBoxAABB(o,&s);
// 	getEmancipationGridAAR(eg,&pos,&sp);
	
// 	return intersectAABBAAR(vectDivInt(o->position,4), s, pos, sp);
// }

// bool collideBoxEmancipationGrids(OBB_s* o)
// {
// 	if(!o)return false;
// 	int i;
// 	for(i=0;i<NUMEMANCIPATIONGRIDS;i++)
// 	{
// 		if(emancipationGrids[i].used)
// 		{
// 			if(emancipationGridBoxCollision(&emancipationGrids[i], o))return true;
// 		}
// 	}
// 	return false;
// }

// bool emancipationGridLineCollision(emancipationGrid_s* eg, vect3Df_s l, vect3Df_s v, int32 d)
// {
// 	if(!eg)return false;
	
// 	vect3Df_s pos, sp;
// 	getEmancipationGridAAR(eg,&pos,&sp);
// 	pos=vectDifference(pos,convertVect(vect(gameRoom.position.x,0,gameRoom.position.y)));

// 	pos=vectDifference(pos,sp);
// 	sp=vect(sp.x*2,sp.y*2,sp.z*2);

// 	vect3Df_s n;
// 	if(!sp.x)n=vect(inttof32(1),0,0);
// 	else if(!sp.y)n=vect(0,inttof32(1),0);
// 	else n=vect(0,0,inttof32(1));

// 	NOGBA("%d %d %d vs %d %d %d",pos.x,pos.y,pos.z,l.x,l.y,l.z);
	
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
