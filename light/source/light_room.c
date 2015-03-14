#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "light_room.h"

void roomInit()
{
}

void roomExit()
{
}

void initRectangleList(rectangleList_s* p)
{
	p->first=NULL;
	p->num=0;
}

vect3Di_s orientVector(vect3Di_s v, u8 k)
{
	vect3Di_s u;

	switch(k)
	{
		case 0:
			u.x=-v.z;
			u.y=v.y;
			u.z=v.x;
			break;
		case 1:
			u.x=v.z;
			u.y=v.y;
			u.z=-v.x;
			break;
		case 4:
			u.x=v.x;
			u.y=v.y;
			u.z=-v.z;
			break;
		default:
			u=v;
			break;
	}

	return u;
}

void invertRectangle(rectangle_s* rec)
{
	if(!rec)return;

	if(rec->size.x)
	{
		rec->position.x+=rec->size.x;
		rec->size.x=-rec->size.x;
	}else{
		rec->position.z+=rec->size.z;
		rec->size.z=-rec->size.z;
	}
}

void roomOriginSize(room_s* r, vect3Di_s* o, vect3Di_s* s)
{
	if(!r || (!o && !s))return;

	vect3Di_s m=vect3Di(8192,8192,8192); vect3Di_s M=vect3Di(0,0,0);

	listCell_s *lc=r->rectangles.first;

	while(lc)
	{
		m=vmini(lc->data.position,m);
		m=vmini(vaddi(lc->data.position,lc->data.size),m);
		M=vmaxi(lc->data.position,M);
		M=vmaxi(vaddi(lc->data.position,lc->data.size),M);
		lc=lc->next;
	}

	if(o)*o=m;
	if(s)*s=vsubi(M,m);
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
	rec.hide=false;
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
