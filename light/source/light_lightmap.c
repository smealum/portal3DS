#include <stdio.h>
#include <stdlib.h>
#include "light_lightmap.h"
#include "light_rectangle.h"

#define AMBIENTLIGHT (10)
#define LIGHTMAPRESOLUTION (currentResolution)

u8 currentResolution = 32;

//LIGHTMAP

void initLightDataLM(lightMapData_s* ld, u16 n)
{
	if(!ld)return;

	ld->num=n;
	ld->lmSize=vect3Di(0,0,0);
	ld->buffer=NULL;
	ld->coords=(lightMapCoordinates_s*)malloc(sizeof(lightMapCoordinates_s)*n);
}

u8 computeLighting(vect3Df_s l, float intensity, vect3Df_s p, rectangle_s* rec, room_s* r)
{
	float dist=vdistf(l,p);
	if(dist<intensity)
	{
		vect3Df_s u=vsubf(p,l);
		u=vdivf(u,dist);
		if(collideLineMapClosest(r, rec, l, u, dist, NULL, NULL)){return 0;}
		float v=vdotf(u,rec->normal);
		v=maxf(0,v);
		v*=3;
		v/=4;
		v+=0.25f;
		// printf("%f\n",v);
		return (u8)(v*(255-((dist*255)/intensity)));
	}
	return 0;
}

u8 computeLightings(vect3Df_s p, rectangle_s* rec, room_s* r)
{
	int v=AMBIENTLIGHT;
	int i;
	for(i=0;i<NUMLIGHTS;i++)
	{
		if(lights[i].used)
		{
			light_s* l=&lights[i];
			v+=computeLighting(l->position, l->intensity, p, rec, r);
		}
	}
	return (u8)(mini(maxi(v,0),255));
}

void fillBuffer(u8* buffer, vect3Di_s p, vect3Di_s s, u8* v, bool rot, int w, int h)
{
	if(!buffer || !v)return;
	int i;
	// u8 vt=(rand()%31)<<3;
	if(!rot)
	{
		printf("bounds %d %d\n",p.x+s.x,p.y+s.y);
		for(i=0;i<s.x;i++)
		{
			int j;
			for(j=0;j<s.y;j++)
			{
				buffer[p.x+i+((p.y+j))*w]=v[i+j*s.x];
				// buffer[p.x+i+((p.y+j))*w]=vt;
			}
		}
	}else{
		printf("bounds %d %d\n",p.x+s.y,p.y+s.x);
		for(i=0;i<s.x;i++)
		{
			int j;
			for(j=0;j<s.y;j++)
			{
				buffer[p.x+j+((p.y+i))*w]=v[i+j*s.x];
				// buffer[p.x+j+((p.y+i))*w]=vt;
			}
		}
	}
}

vect3Df_s getUnitVect(rectangle_s* rec)
{
	vect3Df_s u=vect3Df(0,0,0);
	vect3Di_s size=rec->size;	
	if(size.x>0)u.x=(TILESIZE_FLOAT*2)/LIGHTMAPRESOLUTION;
	else if(size.x)u.x=-(TILESIZE_FLOAT*2)/LIGHTMAPRESOLUTION;
	if(size.y>0)u.y=(TILESIZE_FLOAT*2)/LIGHTMAPRESOLUTION;
	else if(size.y)u.y=-(TILESIZE_FLOAT*2)/LIGHTMAPRESOLUTION;
	if(size.z>0)u.z=(TILESIZE_FLOAT*2)/LIGHTMAPRESOLUTION;
	else if(size.z)u.z=-(TILESIZE_FLOAT*2)/LIGHTMAPRESOLUTION;
	return u;
}

void generateLightmap(rectangle_s* rec, room_s* r, lightMapData_s* lmd, u8* b, lightMapCoordinates_s* lmc)
{
	if(rec && b)
	{
		u16 x=lmc->lmSize.x, y=lmc->lmSize.y;
		u8* data=malloc(x*y);
		if(!data)return;
		vect3Df_s p=convertRectangleVector(rec->position);
		// printf("p : %f, %f, %f\n",p.x,p.y,p.z);
		printf("p : %d, %d, %d\n",rec->position.x,rec->position.y,rec->position.z);
		printf("t : %d, %d\n",lmc->lmPos.x,lmc->lmPos.y);
		int i;
		vect3Df_s u=getUnitVect(rec);
		for(i=0;i<x;i++)
		{
			int j;
			for(j=0;j<y;j++)
			{
				if(!rec->size.x)data[i+j*x]=computeLightings(vaddf(p,vect3Df(0,i*u.y+u.y/2,j*u.z+u.z/2)),rec,r);
				else if(rec->size.y)data[i+j*x]=computeLightings(vaddf(p,vect3Df(i*u.x+u.x/2,j*u.y+u.y/2,0)),rec,r);
				else data[i+j*x]=computeLightings(vaddf(p,vect3Df(i*u.x+u.x/2,0,j*u.z+u.z/2)),rec,r);
			}
		}
		fillBuffer(b, vect3Di(lmc->lmPos.x,lmc->lmPos.y,0), vect3Di(lmc->lmSize.x,lmc->lmSize.y,0), data, lmc->rot, lmd->lmSize.x, lmd->lmSize.y);
		free(data);
	}else printf("NOTHING?\n");
}

int generateLightmaps(room_s* r, lightMapData_s* ld)
{
	if(!r)return -1;
	listCell_s *lc=r->rectangles.first;
	rectangle2DList_s rl;
	initRectangle2DList(&rl);
	int i=0;
	initLightDataLM(ld, r->rectangles.num);

	while(lc)
	{
		insertRectangle2DList(&rl,(rectangle2D_s){vect2(0,0),vect2(abs(lc->data.size.x?(lc->data.size.x*LIGHTMAPRESOLUTION):(lc->data.size.y*LIGHTMAPRESOLUTION*HEIGHTUNIT/(TILESIZE_FLOAT*2))),
																		abs((lc->data.size.y&&lc->data.size.x)?(lc->data.size.y*LIGHTMAPRESOLUTION*HEIGHTUNIT/(TILESIZE_FLOAT*2)):(lc->data.size.z*LIGHTMAPRESOLUTION))),
																		&ld->coords[i++], false});
		lc=lc->next;
	}
	short w=32, h=32;

	bool rr=packRectanglesSize(&rl, &w, &h);
	ld->lmSize=vect3Di(w,h,0);
	printf("done : %d %dx%d\n",(int)rr,w,h);

	if(!rr)
	{
		//TEMP
		// freeLightData(ld);
		return -1;
	}

	ld->buffer=malloc(w*h);

	if(!ld->buffer)
	{
		// freeLightData(ld);
		return -1;
	}

	lc=r->rectangles.first;
	i=0;
	while(lc)
	{
		generateLightmap(&lc->data, r, ld, ld->buffer, &ld->coords[i++]);
		lc=lc->next;
	}
		
	freeRectangle2DList(&rl);
	printf("freed.\n");

	return 0;
}
