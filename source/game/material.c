#include <stdio.h>
#include <3ds.h>
#include "game/material.h"
#include "game/room.h"
#include "utils/iniparser.h"

material_s materials[NUMMATERIALS];
material_s* defaultMaterial;
materialSlice_s materialSlices[NUMMATERIALSLICES];
materialSlice_s* defaultMaterialSlice;

void initMaterials(void)
{
	int i;
	for(i=0;i<NUMMATERIALS;i++)
	{
		material_s* m=&materials[i];
		m->used=false;
		m->id=i;
		m->top=m->bottom=m->side=NULL;
	}
	for(i=0;i<NUMMATERIALSLICES;i++)
	{
		materialSlice_s* ms=&materialSlices[i];
		ms->used=false;
		ms->id=i;
		ms->img=NULL;
	}
	defaultMaterial=createMaterial();
	defaultMaterialSlice=createMaterialSlice();
	loadMaterialSlice(defaultMaterialSlice, "sdmc:/default.png");
}

materialSlice_s* createMaterialSlice()
{
	int i;
	for(i=0;i<NUMMATERIALSLICES;i++)
	{
		if(!materialSlices[i].used)
		{
			materialSlice_s* ms=&materialSlices[i];
			ms->used=true;
			ms->img=NULL;
			return ms;
		}
	}
	return NULL;
}

material_s* createMaterial()
{
	int i;
	for(i=0;i<NUMMATERIALS;i++)
	{
		if(!materials[i].used)
		{
			material_s* m=&materials[i];
			m->used=true;
			m->top=m->bottom=m->side=NULL;
			return m;
		}
	}
	return NULL;
}

void loadMaterialSlice(materialSlice_s* ms, char* filename)
{
	if(ms)
	{
		ms->img=textureCreate(filename, GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)|GPU_TEXTURE_WRAP_S(GPU_REPEAT)|GPU_TEXTURE_WRAP_T(GPU_REPEAT));
		ms->align=false;
		ms->factorX=1;
		ms->factorY=1;
	}
}

void loadMaterialSlices(char* filename)
{
	dictionary* dic=iniparser_load(filename);
	int i=0;
	char* r;
	char key[255];
	sprintf(key,"slice%d:texture",i);
	while((r=dictionary_get(dic, key, NULL)))
	{
		materialSlice_s* ms=createMaterialSlice();
		loadMaterialSlice(ms,r);
		printf("loaded %d : %s",i,r);
		
			int k=0;
			sprintf(key,"slice%d:align",i);
			sscanf(dictionary_get(dic, key, "0"),"%d",&k);
			ms->align=(k!=0);
			sprintf(key,"slice%d:factor",i);
			sscanf(dictionary_get(dic, key, "1"),"%d",&k);
			ms->factorX=k;
			ms->factorY=k;
			
			{
				char def[16];
				sprintf(def,"%d",ms->factorX);
				sprintf(key,"slice%d:factorx",i);
				sscanf(dictionary_get(dic, key, def),"%d",&k);
				ms->factorX=k;
				sprintf(key,"slice%d:factory",i);
				sscanf(dictionary_get(dic, key, def),"%d",&k);
				ms->factorY=k;
			}
		
		// if(!ms->img){break;}
		i++;
		sprintf(key,"slice%d:texture",i);		
	}
	iniparser_freedict(dic);
}

void loadMaterials(char* filename)
{
	dictionary* dic=iniparser_load(filename);
	int i=0;
	char *r1, *r2, *r3;
	char key1[255],key2[255],key3[255];
	sprintf(key1,"material%d:top",i);
	sprintf(key2,"material%d:side",i);
	sprintf(key3,"material%d:bottom",i);
	(r1=dictionary_get(dic, key1, NULL));(r2=dictionary_get(dic, key2, NULL));(r3=dictionary_get(dic, key3, NULL));
	while(r1||r2||r3)
	{
		material_s* m=createMaterial();
		if(!m){break;}
		if(r1)
		{
			int k;
			sscanf(r1,"%d",&k);
			m->top=&materialSlices[k+1];
		}
		if(r2)
		{
			int k;
			sscanf(r2,"%d",&k);
			m->side=&materialSlices[k+1];
		}
		if(r3)
		{
			int k;
			sscanf(r3,"%d",&k);
			m->bottom=&materialSlices[k+1];
		}
		i++;
		sprintf(key1,"material%d:top",i);
		sprintf(key2,"material%d:side",i);
		sprintf(key3,"material%d:bottom",i);	
		r1=dictionary_get(dic, key1, NULL);r2=dictionary_get(dic, key2, NULL);r3=dictionary_get(dic, key3, NULL);
	}
	iniparser_freedict(dic);
}

material_s* getMaterial(u16 i)
{
	if(i<0 || i>NUMMATERIALS)i=0;
	return &materials[i];
}

void getTextureCoordSlice(materialSlice_s* ms, rectangle_s* rec, vect3Di_s* v)
{
	if(!v)return;
	if(!ms)ms=defaultMaterialSlice;
	if(!ms->img)return;

	vect3Di_s p1=vect3Di(0,0,0), p2;

	if(!rec->size.x)
	{
		p1=vect3Di(ms->img->width * rec->position.z, ms->img->height * rec->position.y, 0);
		p2=vect3Di(ms->img->width * rec->size.z, ms->img->height * rec->size.y, 0);
	}else if(!rec->size.y)
	{
		p1=vect3Di(ms->img->width * rec->position.z, ms->img->height * rec->position.x, 0);
		p2=vect3Di(ms->img->width * rec->size.z, ms->img->height * rec->size.x, 0);
	}else
	{
		p1=vect3Di(ms->img->width * rec->position.y, ms->img->height * rec->position.x, 0);
		p2=vect3Di(ms->img->width * rec->size.y, ms->img->height * rec->size.x, 0);
	}

	p1.x%=(ms->img->width);
	p1.y%=(ms->img->height);

	p2=vaddi(p1,p2);

	p1=vect3Di(p1.x/ms->factorX, p1.y/ms->factorY, 0);
	p2=vect3Di(p2.x/ms->factorX, p2.y/ms->factorY, 0);

	v[0]=vect3Di(p1.x, p1.y, 0);
	v[1]=vect3Di(p1.x, p2.y, 0);
	v[2]=vect3Di(p2.x, p2.y, 0);
	v[3]=vect3Di(p2.x, p1.y, 0);
}

void getMaterialTextureCoord(rectangle_s* rec, vect3Di_s* v)
{
	if(!rec || !rec->material || !v)return;

	materialSlice_s* ms=rec->material->side;

	if(!rec->size.y)
	{
		if(rec->normal.y>0)ms=rec->material->top;
		else ms=rec->material->bottom;
	}

	getTextureCoordSlice(ms, rec, v);
}

texture_s* getRectangleTexture(rectangle_s* rec)
{
	if(!rec)return NULL;
	material_s* m=rec->material;
	if(!m)m=defaultMaterial;

	materialSlice_s* ms=rec->material->side;
	if(!rec->size.y)
	{
		if(rec->normal.y>0)ms=rec->material->top;
		else ms=rec->material->bottom;
	}
	if(!ms)ms=defaultMaterialSlice;

	return ms->img;
}
