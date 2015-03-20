#include <string.h>
#include "physics/PIC.h"
#include "physics/OBB.h"
#include "game/portal.h"
#include "gfx/gs.h"

#define TIMEPREC (6)

OBB_s objects[NUMOBJECTS];

u32 coll, integ, impul;

/*s16 divLUT[4097];

void initDivision(void)
{
	int i;
	for(i=1;i<4097;i++)
	{
		divLUT[i]=divv16(4096,i);
	}
	divLUT[0]=0;
}*/

static inline float divv(float a, float b) // b in 1-4096
{
	return a/b;
}

void initOBB(OBB_s* o, vect3Df_s pos, vect3Df_s size, md2_instance_t* model, float mass, float angle)
{
	if(!o)return;
	
	o->used=true;
	o->position=pos;
	o->angularMomentum=vect3Df(0,0,0);
	o->numContactPoints=0;
	o->size=size;
	o->mass=mass;
	o->maxPenetration=0;
	
	o->energy=0;
	o->sleep=false;
	
	o->velocity=vect3Df(0,0,0);
	o->angularVelocity=vect3Df(0,0,0);
	o->moment=vect3Df(0,0,0);
	o->forces=vect3Df(0,0,0);

	initObbTransformationMatrix(o->transformationMatrix, angle);

	float x2=(o->size.x * o->size.x);
	float y2=(o->size.y * o->size.y);
	float z2=(o->size.z * o->size.z);
	
	int i;for(i=0;i<9;i++){o->invInertiaMatrix[i]=0;}
    o->invInertiaMatrix[0]=((3.0f)/((o->mass * (y2+z2))));
    o->invInertiaMatrix[4]=((3.0f)/((o->mass * (x2+z2))));
    o->invInertiaMatrix[8]=((3.0f)/((o->mass * (x2+y2))));
	
	//temporary
	o->contactPoints=contactPoints;
	
	//rotateMatrixX(o->transformationMatrix,4096,false);
	//rotateMatrixZ(o->transformationMatrix,4096,false);
	
	for(i=0; i < NUM_PORTALS; i++)
	{
		if(portals[i].target)updateOBBPortals(o,i,true);
	}

	o->modelInstance=model;
}

void initOBBs(void)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		objects[i].used=false;
	}
}

void copyOBB(OBB_s* o1, OBB_s* o2)
{
	if(!o1 || !o2)return;
	
	o2->angularMomentum=o1->angularMomentum;
	o2->numContactPoints=o1->numContactPoints;
	o2->mass=o1->mass;
	o2->position=o1->position;
	o2->size=o1->size;
	o2->maxPenetration=o1->maxPenetration;
	
	o2->velocity=o1->velocity;
	o2->angularVelocity=o1->angularVelocity;
	o2->forces=o1->forces;
	o2->moment=o1->moment;
	
	o2->energy=o1->energy;
	o2->sleep=o1->sleep;
	
	//temporary
	o2->contactPoints=contactPoints;
	
	memcpy(o2->transformationMatrix,o1->transformationMatrix,sizeof(float)*9);
	memcpy(o2->invInertiaMatrix,o1->invInertiaMatrix,sizeof(float)*9);
	memcpy(o2->invWInertiaMatrix,o1->invWInertiaMatrix,sizeof(float)*9);
	
	memcpy(o2->contactPoints,o1->contactPoints,sizeof(contactPoint_struct)*o2->numContactPoints);
}

bool collideAABB(vect3Df_s o1, vect3Df_s s1, vect3Df_s o2, vect3Df_s s2)
{
	return !(o2.x>o1.x+s1.x || o2.y>o1.y+s1.y || o2.z>o1.z+s1.z
		  || o2.x+s2.x<o1.x || o2.y+s2.y<o1.y || o2.z+s2.z<=o1.z);
}

vect3Df_s projectPointAABB(vect3Df_s size, vect3Df_s p, vect3Df_s* n)
{
	if(!n)return vect3Df(0,0,0);
	vect3Df_s v=p;
	*n=vect3Df(0,0,0);

	/*if(p.x<-size.x){v.x=-size.x;n->x=-1;}
	else if(p.x>size.x){v.x=size.x;n->x=1;}
	
	if(p.y<-size.y){v.y=-size.y;n->y=-1;}
	else if(p.y>size.y){v.y=size.y;n->y=1;}

	if(p.z<-size.z){v.z=-size.z;n->z=-1;}
	else if(p.z>size.z){v.z=size.z;n->z=1;}*/

	if(p.x<-size.x){v.x=-size.x;n->x=-1;}
	else if(p.x>size.x){v.x=size.x;n->x=1;}
	else if(p.y<-size.y){v.y=-size.y;n->y=-1;}
	else if(p.y>size.y){v.y=size.y;n->y=1;}
	else if(p.z<-size.z){v.z=-size.z;n->z=-1;}
	else if(p.z>size.z){v.z=size.z;n->z=1;}

	if(!n->x && !n->y && !n->z)
	{	
		float d1=fabs(p.x+size.x);float d2=fabs(p.x-size.x);
		float d3=fabs(p.y+size.y);float d4=fabs(p.y-size.y);
		float d5=fabs(p.z+size.z);float d6=fabs(p.z-size.z);
		float d=minf(d1,d2);
		d=minf(d,minf(d3,d4));
		d=minf(d,minf(d5,d6));
		if(d==d1){v.x=-size.x;n->x=-1;}
		else if(d==d2){v.x=size.x;n->x=1;}
		else if(d==d3){v.y=-size.y;n->y=-1;}
		else if(d==d4){v.y=size.y;n->y=1;}
		else if(d==d5){v.z=-size.z;n->z=-1;}
		else {v.z=size.z;n->z=1;}
	}

	return v;
}

bool clipSegmentOBB(float* ss, vect3Df_s *uu, vect3Df_s* p1, vect3Df_s* p2, vect3Df_s vv, vect3Df_s* uu1, vect3Df_s* uu2, vect3Df_s vv1, vect3Df_s* n1, vect3Df_s* n2, bool* b1, bool* b2, float* k1, float* k2)
{
	if(!p1 || !p2 || !uu1 || !uu2 || !n1 || !n2 || !b1 || !b2 || !k1 || !k2)return false;

	if(uu1->x<-ss[0])
	{
		if(uu2->x>-ss[0])
		{
			float k=divv(fabs(uu1->x+ss[0]),fabs(vv1.x));
				*uu1=vaddf(*uu1,vmulf(vv1,k));
				*p1=vaddf(*p1,vmulf(vv,k));
			*k1=maxf(*k1,k);
			*n1=vect3Df(-uu[0].x,-uu[0].y,-uu[0].z);
			*b1=true;
		}else return false;
	}else{
		if(uu2->x<-ss[0])
		{
			float k=divv(fabs(uu1->x+ss[0]),fabs(vv1.x));
				*uu2=vaddf(*uu1,vmulf(vv1,k));
				*p2=vaddf(*p1,vmulf(vv,k));
			*k2=minf(*k2,k);
			*n2=vect3Df(-uu[0].x,-uu[0].y,-uu[0].z);
			*b2=true;
		}
	}
				
	if(uu1->x<ss[0])
	{
		if(uu2->x>ss[0])
		{
			float k=divv(fabs(uu1->x-ss[0]),fabs(vv1.x));
				*uu2=vaddf(*uu1,vmulf(vv1,k));
				*p2=vaddf(*p1,vmulf(vv,k));
			*k2=minf(*k2,k);
			*n2=(uu[0]);
			*b2=true;
		}
	}else{
		if(uu2->x<ss[0])
		{
			float k=divv(fabs(uu1->x-ss[0]),fabs(vv1.x));
				*uu1=vaddf(*uu1,vmulf(vv1,k));
				*p1=vaddf(*p1,vmulf(vv,k));
			*k1=maxf(*k1,k);
			*n1=(uu[0]);
			*b1=true;
		}else return false;
	}

				
	if(uu1->y<-ss[1])
	{
		if(uu2->y>-ss[1])
		{
			float k=divv(fabs(uu1->y+ss[1]),fabs(vv1.y));
				*uu1=vaddf(*uu1,vmulf(vv1,k));
				*p1=vaddf(*p1,vmulf(vv,k));
			*k1=maxf(*k1,k);
			*n1=vect3Df(-uu[1].x,-uu[1].y,-uu[1].z);
			*b1=true;
		}else return false;
	}else{
		if(uu2->y<-ss[1])
		{
			float k=divv(fabs(uu1->y+ss[1]),fabs(vv1.y));
				*uu2=vaddf(*uu1,vmulf(vv1,k));
				*p2=vaddf(*p1,vmulf(vv,k));
			*k2=minf(*k2,k);
			*n2=vect3Df(-uu[1].x,-uu[1].y,-uu[1].z);
			*b2=true;
		}
	}
				
	if(uu1->y<ss[1])
	{
		if(uu2->y>ss[1])
		{
			float k=divv(fabs(uu1->y-ss[1]),fabs(vv1.y));
				*uu2=vaddf(*uu1,vmulf(vv1,k));
				*p2=vaddf(*p1,vmulf(vv,k));
			*k2=minf(*k2,k);
			*n2=(uu[1]);
			*b2=true;
		}
	}else{
		if(uu2->y<ss[1])
		{
			float k=divv(fabs(uu1->y-ss[1]),fabs(vv1.y));
				*uu1=vaddf(*uu1,vmulf(vv1,k));
				*p1=vaddf(*p1,vmulf(vv,k));
			*k1=maxf(*k1,k);
			*n1=(uu[1]);
			*b1=true;
		}else return false;
	}

				
	if(uu1->z<-ss[2])
	{
		if(uu2->z>-ss[2])
		{
			float k=divv(fabs(uu1->z+ss[2]),fabs(vv1.z));
				*uu1=vaddf(*uu1,vmulf(vv1,k));
				*p1=vaddf(*p1,vmulf(vv,k));
			*k1=maxf(*k1,k);
			*n1=vect3Df(-uu[2].x,-uu[2].y,-uu[2].z);
			*b1=true;
		}else return false;
	}else{
		if(uu2->z<-ss[2])
		{
			float k=divv(fabs(uu1->z+ss[2]),fabs(vv1.z));
				*uu2=vaddf(*uu1,vmulf(vv1,k));
				*p2=vaddf(*p1,vmulf(vv,k));
			*k2=minf(*k2,k);
			*n2=vect3Df(-uu[2].x,-uu[2].y,-uu[2].z);
			*b2=true;
		}
	}
				
	if(uu1->z<ss[2])
	{
		if(uu2->z>ss[2])
		{
			float k=divv(fabs(uu1->z-ss[2]),fabs(vv1.z));
				*uu2=vaddf(*uu1,vmulf(vv1,k));
				*p2=vaddf(*p1,vmulf(vv,k));
			*k2=minf(*k2,k);
			*n2=(uu[2]);
			*b2=true;
		}
	}else{
		if(uu2->z<ss[2])
		{
			float k=divv(fabs(uu1->z-ss[2]),fabs(vv1.z));
				*uu1=vaddf(*uu1,vmulf(vv1,k));
				*p1=vaddf(*p1,vmulf(vv,k));
			*k1=maxf(*k1,k);
			*n1=(uu[2]);
			*b1=true;
		}else return false;
	}
	return true;
}

void collideOBBs(OBB_s* o1, OBB_s* o2)
{
	if(!o1 || !o2)return;
	if(o1==o2)return;

	if(!collideAABB(vect3Df(o1->AABBo.x-MAXPENETRATIONBOX,o1->AABBo.y-MAXPENETRATIONBOX,o1->AABBo.z-MAXPENETRATIONBOX),
					vect3Df(o1->AABBs.x+2*MAXPENETRATIONBOX,o1->AABBs.y+2*MAXPENETRATIONBOX,o1->AABBs.z+2*MAXPENETRATIONBOX),
					vect3Df(o2->AABBo.x-MAXPENETRATIONBOX,o2->AABBo.y-MAXPENETRATIONBOX,o2->AABBo.z-MAXPENETRATIONBOX),
					vect3Df(o2->AABBs.x+2*MAXPENETRATIONBOX,o2->AABBs.y+2*MAXPENETRATIONBOX,o2->AABBs.z+2*MAXPENETRATIONBOX)))return;

	vect3Df_s v[8],v2[8];
	getOBBVertices(o1,v);
	
	vect3Df_s z1=vect3Df(o1->transformationMatrix[0],o1->transformationMatrix[3],o1->transformationMatrix[6]);
	vect3Df_s z2=vect3Df(o1->transformationMatrix[1],o1->transformationMatrix[4],o1->transformationMatrix[7]);
	vect3Df_s z3=vect3Df(o1->transformationMatrix[2],o1->transformationMatrix[5],o1->transformationMatrix[8]);

	vect3Df_s u1=vect3Df(o2->transformationMatrix[0],o2->transformationMatrix[3],o2->transformationMatrix[6]);
	vect3Df_s u2=vect3Df(o2->transformationMatrix[1],o2->transformationMatrix[4],o2->transformationMatrix[7]);
	vect3Df_s u3=vect3Df(o2->transformationMatrix[2],o2->transformationMatrix[5],o2->transformationMatrix[8]);

	vect3Df_s pp=vsubf(o1->position,o2->position);

	getVertices(o1->size, vect3Df(vdotf(pp,u1),vdotf(pp,u2),vdotf(pp,u3)), vect3Df(vdotf(z1,u1),vdotf(z1,u2),vdotf(z1,u3)), 
		vect3Df(vdotf(z2,u1),vdotf(z2,u2),vdotf(z2,u3)), vect3Df(vdotf(z3,u1),vdotf(z3,u2),vdotf(z3,u3)), v2);

	int i;
	/*for(i=0;i<8;i++)
	{
		//v2[i]=vsubf(v[i],o2->position);
		//v2[i]=vect3Df(vdotf(v2[i],u1),vdotf(v2[i],u2),vdotf(v2[i],u3));

		if(v2[i].x>-o2->size.x-MAXPENETRATIONBOX && v2[i].x<o2->size.x+MAXPENETRATIONBOX
		&& v2[i].y>-o2->size.y-MAXPENETRATIONBOX && v2[i].y<o2->size.y+MAXPENETRATIONBOX
		&& v2[i].z>-o2->size.z-MAXPENETRATIONBOX && v2[i].z<o2->size.z+MAXPENETRATIONBOX)
		{
			vect3Df_s n;
			vect3Df_s p=projectPointAABB(o2->size,v2[i],&n);
			o1->contactPoints[o1->numContactPoints].point=v[i];
			o1->contactPoints[o1->numContactPoints].normal=vnormf(vect3Df(n.x*u1.x+n.y*u2.x+n.z*u3.x,n.x*u1.y+n.y*u2.y+n.z*u3.y,n.x*u1.z+n.y*u2.z+n.z*u3.z));
			if(fabs(n.x)+fabs(n.y)+fabs(n.z)!=1)
			{
				printf("erf\n");
			}
			o1->contactPoints[o1->numContactPoints].penetration=vdistf(v2[i],p);
			o1->contactPoints[o1->numContactPoints].penetration=0;
			o1->contactPoints[o1->numContactPoints].target=o2;
			o1->contactPoints[o1->numContactPoints].type=BOXCOLLISION;
			o1->maxPenetration=maxf(o1->maxPenetration,o1->contactPoints[o1->numContactPoints].penetration);
			o1->numContactPoints++;
		}
	}*/

	//optimize by working in o2 space ?
	vect3Df_s vv2[8];
	getOBBVertices(o2,vv2);
	vect3Df_s u[3];
		u[0]=vect3Df(o1->transformationMatrix[0],o1->transformationMatrix[3],o1->transformationMatrix[6]);
		u[1]=vect3Df(o1->transformationMatrix[1],o1->transformationMatrix[4],o1->transformationMatrix[7]);
		u[2]=vect3Df(o1->transformationMatrix[2],o1->transformationMatrix[5],o1->transformationMatrix[8]);
	vect3Df_s uu[3];
		uu[0]=vect3Df(o2->transformationMatrix[0],o2->transformationMatrix[3],o2->transformationMatrix[6]);
		uu[1]=vect3Df(o2->transformationMatrix[1],o2->transformationMatrix[4],o2->transformationMatrix[7]);
		uu[2]=vect3Df(o2->transformationMatrix[2],o2->transformationMatrix[5],o2->transformationMatrix[8]);
	float s[3];
		s[0]=o1->size.x;
		s[1]=o1->size.y;
		s[2]=o1->size.z;
	float ss[3];
		ss[0]=o2->size.x;//+MAXPENETRATIONBOX;
		ss[1]=o2->size.y;//+MAXPENETRATIONBOX;
		ss[2]=o2->size.z;//+MAXPENETRATIONBOX;
	for(i=0;i<NUMOBBSEGMENTS;i++)
	{
		vect3Df_s uu1=v2[OBBSegments[i][0]];
		vect3Df_s uu2=v2[OBBSegments[i][1]];
		const bool t=!((uu1.x<-ss[0] && uu2.x<-ss[0]) || (uu1.x>ss[0] && uu2.x>ss[0])
				  || (uu1.y<-ss[1] && uu2.y<-ss[1]) || (uu1.y>ss[1] && uu2.y>ss[1])
				  || (uu1.z<-ss[2] && uu2.z<-ss[2]) || (uu1.z>ss[2] && uu2.z>ss[2]));
		if(t)
		{
			do{
				const vect3Df_s vv=u[OBBSegmentsPD[i][1]];
				const vect3Df_s vv1=vect3Df(vdotf(vv,uu[0]),vdotf(vv,uu[1]),vdotf(vv,uu[2]));
				vect3Df_s p1=v[OBBSegments[i][0]];
				vect3Df_s p2=v[OBBSegments[i][1]];
				vect3Df_s n1=vect3Df(0,0,0);
				vect3Df_s n2=vect3Df(0,0,0);
				float k1=0;
				float k2=s[OBBSegmentsPD[i][1]]*2;
				bool b1=false;
				bool b2=false;
				if(!clipSegmentOBB(ss,uu,&p1,&p2,vv,&uu1,&uu2,vv1,&n1,&n2,&b1,&b2,&k1,&k2))break;
				
				if(b1&&b2)
				{
					//p1=vaddf(p1,vmulf(vv,k1));
					//p2=vaddf(p2,vmulf(vv,k2));
					o1->contactPoints[o1->numContactPoints].point=vdivf(vaddf(p1,p2),2);
					o1->contactPoints[o1->numContactPoints].type=TESTPOINT;
					vect3Df_s n;
					vect3Df_s oo=vdivf(vaddf(uu1,uu2),2);
					vect3Df_s p=projectPointAABB(o2->size,oo,&n);
					o1->contactPoints[o1->numContactPoints].normal=(vect3Df(n.x*u1.x+n.y*u2.x+n.z*u3.x,n.x*u1.y+n.y*u2.y+n.z*u3.y,n.x*u1.z+n.y*u2.z+n.z*u3.z));
					o1->contactPoints[o1->numContactPoints].penetration=vdistf(p,oo);
					//o1->contactPoints[o1->numContactPoints].penetration=0;
					o1->contactPoints[o1->numContactPoints].target=o2;
					o1->numContactPoints++;	
				}else{
					if(b1)
					{
						//p1=vaddf(p1,vmulf(vv,k1));
						o1->contactPoints[o1->numContactPoints].point=p1;
						o1->contactPoints[o1->numContactPoints].type=TESTPOINT;
						o1->contactPoints[o1->numContactPoints].normal=n1;
						o1->contactPoints[o1->numContactPoints].penetration=0;
						o1->contactPoints[o1->numContactPoints].target=o2;
						o1->numContactPoints++;
					}
					if(b2)
					{
						//p2=vaddf(p2,vmulf(vv,k2));
						o1->contactPoints[o1->numContactPoints].point=p2;
						o1->contactPoints[o1->numContactPoints].type=TESTPOINT;
						o1->contactPoints[o1->numContactPoints].normal=n2;
						o1->contactPoints[o1->numContactPoints].penetration=0;
						o1->contactPoints[o1->numContactPoints].target=o2;
						o1->numContactPoints++;
					}
				}
			}while(0);
		}
	}
}

void initObbTransformationMatrix(float* m, float angle)
{
	if(!m)return;
	int i;
	for(i=0;i<9;i++)m[i]=0;

	m[4]=(1.0f);

	m[0]=cos(angle);
	m[6]=sin(angle);

	m[2]=-sin(angle);
	m[8]=cos(angle);
}

void getVertices(vect3Df_s s, vect3Df_s p, vect3Df_s u1, vect3Df_s u2, vect3Df_s u3, vect3Df_s* v)
{
	float m2[9];
	m2[0]=(u1.x*s.x);m2[3]=(u1.y*s.x);m2[6]=(u1.z*s.x);
	m2[1]=(u2.x*s.y);m2[4]=(u2.y*s.y);m2[7]=(u2.z*s.y);
	m2[2]=(u3.x*s.z);m2[5]=(u3.y*s.z);m2[8]=(u3.z*s.z);

	v[0]=vect3Df(-m2[0]-m2[1]-m2[2],-m2[3]-m2[4]-m2[5],-m2[6]-m2[7]-m2[8]);
	v[1]=vect3Df(m2[0]-m2[1]-m2[2],m2[3]-m2[4]-m2[5],m2[6]-m2[7]-m2[8]);
	v[2]=vect3Df(m2[0]-m2[1]+m2[2],m2[3]-m2[4]+m2[5],m2[6]-m2[7]+m2[8]);
	v[3]=vect3Df(-m2[0]-m2[1]+m2[2],-m2[3]-m2[4]+m2[5],-m2[6]-m2[7]+m2[8]);
	
	v[4]=vect3Df(-v[1].x,-v[1].y,-v[1].z);
	v[5]=vect3Df(-v[2].x,-v[2].y,-v[2].z);
	v[6]=vect3Df(-v[3].x,-v[3].y,-v[3].z);
	v[7]=vect3Df(-v[0].x,-v[0].y,-v[0].z);
	
	v[0]=vaddf(v[0],p);v[1]=vaddf(v[1],p);v[2]=vaddf(v[2],p);v[3]=vaddf(v[3],p);
	v[4]=vaddf(v[4],p);v[5]=vaddf(v[5],p);v[6]=vaddf(v[6],p);v[7]=vaddf(v[7],p);
}

void getOBBVertices(OBB_s* o, vect3Df_s* v)
{
	if(!o || !v)return;
	float* m=o->transformationMatrix;
	getVertices(o->size,o->position,vect3Df(m[0],m[3],m[6]),vect3Df(m[1],m[4],m[7]),vect3Df(m[2],m[5],m[8]),v);

	int i;
	vect3Df_s mm=o->position;
	vect3Df_s M=o->position;
	for(i=0;i<8;i++)
	{
		//v[i]=vaddf(v[i],o->position);
		if(v[i].x<mm.x)mm.x=v[i].x;
		if(v[i].y<mm.y)mm.y=v[i].y;
		if(v[i].z<mm.z)mm.z=v[i].z;
		if(v[i].x>M.x)M.x=v[i].x;
		if(v[i].y>M.y)M.y=v[i].y;
		if(v[i].z>M.z)M.z=v[i].z;
	}
	o->AABBo=mm;
	o->AABBs=vsubf(M,mm);
}

void applyOBBForce(OBB_s* o, vect3Df_s p, vect3Df_s f)
{
	if(!o)return;
	o->forces=vaddf(o->forces,f);
	o->moment=vaddf(o->moment,vprodf(vsubf(p,o->position),f));
}

void applyOBBImpulsePlane(OBB_s* o, u8 pID)
{
	if(!o || pID>=o->numContactPoints)return;
	contactPoint_struct* cp=&o->contactPoints[pID];    
	vect3Df_s r=vsubf(cp->point,o->position);
	vect3Df_s v=vaddf(o->velocity,vprodf(o->angularVelocity,r));

	const float CoefficientOfRestitution=(0.2f);
    
	float iN=-(((1.0f)+CoefficientOfRestitution) * vdotf(v,cp->normal));
	float invMass=((1.0f)/o->mass);
	float iD=invMass+vdotf(vprodf(vevalf(o->invWInertiaMatrix,vprodf(r,cp->normal)),r),cp->normal);
    //iN=divf32(iN,iD);
    iN=(iN/iD);
	if(iN<0)iN=0;
	//vect3Df_s imp=vmulf(cp->normal,iN);
	vect3Df_s imp=vmulf(cp->normal,iN+cp->penetration/2); //added bias adds jitter, but prevents sinking.

	//printf("imp : %d",iN);
    
    // apply impulse to primary quantities
	o->velocity=vaddf(o->velocity,vmulf(imp,invMass));
	o->angularMomentum=vaddf(o->angularMomentum,vprodf(r,imp));
    
    // compute affected auxiliary quantities
	o->angularVelocity=vevalf(o->invWInertiaMatrix,o->angularMomentum);
	
	{
		vect3Df_s tangent=vect3Df(0,0,0);
		tangent=vsubf(v,(vmulf(cp->normal,vdotf(v, cp->normal))));
		if(vmagf(tangent)<0.01f)return;
		tangent=vnormf(tangent);

		float kTangent=invMass+vdotf(tangent,vprodf(vevalf(o->invWInertiaMatrix,(vprodf(r, tangent))), r));

		float vt = vdotf(v, tangent);
		//float dPt = divf32((-vt),kTangent);
		float dPt = ((-vt)/kTangent);

		const float frictionCONST=(1.0f);

		float maxPt=fabs((frictionCONST*iN));
		if(dPt<-maxPt)dPt=-maxPt;
		else if(dPt>maxPt)dPt=maxPt;

		// Apply contact impulse
		vect3Df_s P = vmulf(tangent,dPt);

		o->velocity=vaddf(o->velocity,vmulf(P,invMass));
		o->angularMomentum=vaddf(o->angularMomentum,vprodf(r,P));
    
		// compute affected auxiliary quantities
		o->angularVelocity=vevalf(o->invWInertiaMatrix,o->angularMomentum);
	}
}

void applyOBBImpulseOBB(OBB_s* o, u8 pID)
{
	if(!o || pID>=o->numContactPoints)return;
	contactPoint_struct* cp=&o->contactPoints[pID];
	OBB_s* o2=(OBB_s*)cp->target;
	if(!o2)return;
	vect3Df_s r1=vsubf(cp->point,o->position);
	vect3Df_s r2=vsubf(cp->point,o2->position);
	vect3Df_s v1=vaddf(o->velocity,vprodf(o->angularVelocity,r1));
	vect3Df_s v2=vaddf(o2->velocity,vprodf(o2->angularVelocity,r2));
	vect3Df_s dv=vsubf(v1,v2);

	const float CoefficientOfRestitution=(0.5f);
    
	float iN=-(((1.0f)+CoefficientOfRestitution)*vdotf(dv,cp->normal));
	float invMass1=((1.0f)/o->mass);
	float invMass2=((1.0f)/o2->mass);
	float iD=invMass1+invMass2+vdotf(vaddf(vprodf(vevalf(o->invWInertiaMatrix,vprodf(r1,cp->normal)),r1),vprodf(vevalf(o2->invWInertiaMatrix,vprodf(r2,cp->normal)),r2)),cp->normal);
	//iN=divf32(iN,iD);
	iN=(iN/iD);
	if(iN<0)iN=0;
	//vect3Df_s imp=vmulf(cp->normal,iN);
	vect3Df_s imp=vmulf(cp->normal,iN+cp->penetration); //added bias adds jitter, but prevents sinking.

	//printf("norm : %d %d %d\n",cp->normal.x,cp->normal.y,cp->normal.z);
    
    // apply impulse to primary quantities
	o->velocity=vaddf(o->velocity,vmulf(imp,invMass1));
	o->angularMomentum=vaddf(o->angularMomentum,vprodf(r1,imp));

	o2->velocity=vsubf(o2->velocity,vmulf(imp,invMass2));
	o2->angularMomentum=vsubf(o2->angularMomentum,vprodf(r2,imp));
    
    // compute affected auxiliary quantities
	o->angularVelocity=vevalf(o->invWInertiaMatrix,o->angularMomentum);

	o2->angularVelocity=vevalf(o2->invWInertiaMatrix,o2->angularMomentum);
	
	{
		vect3Df_s tangent=vect3Df(0,0,0);
		tangent=vsubf(dv,(vmulf(cp->normal,vdotf(dv, cp->normal))));
		if(vmagf(tangent)<0.01f)return;
		tangent=vnormf(tangent);

		float kTangent=invMass1+invMass2+vdotf(tangent,vaddf(vprodf(vevalf(o->invWInertiaMatrix,(vprodf(r1, tangent))), r1),
																	vprodf(vevalf(o->invWInertiaMatrix,(vprodf(r2, tangent))), r2)));

		float vt = vdotf(dv, tangent);
		//float dPt = divf32((-vt),kTangent);
		float dPt = ((-vt)/kTangent);

		const float frictionCONST=(0.5f);

		float maxPt=fabs((frictionCONST*iN));
		if(dPt<-maxPt)dPt=-maxPt;
		else if(dPt>maxPt)dPt=maxPt;

		// Apply contact impulse
		vect3Df_s P = vmulf(tangent,dPt);
		
		o->velocity=vaddf(o->velocity,vmulf(P,invMass1));
		o->angularMomentum=vaddf(o->angularMomentum,vprodf(r1,P));

		o2->velocity=vsubf(o2->velocity,vmulf(P,invMass2));
		o2->angularMomentum=vsubf(o2->angularMomentum,vprodf(r2,P));
    
		// compute affected auxiliary quantities
		o->angularVelocity=vevalf(o->invWInertiaMatrix,o->angularMomentum);
		o2->angularVelocity=vevalf(o2->invWInertiaMatrix,o2->angularMomentum);
	}
}

void applyOBBImpulses(OBB_s* o)
{
	if(!o)return;
	int i;
	for(i=0;i<o->numContactPoints;i++)
	{
		switch(o->contactPoints[i].type)
		{
			case PLANECOLLISION:
				applyOBBImpulsePlane(o,i);
				break;
			case AARCOLLISION:
				applyOBBImpulsePlane(o,i);
				break;
			case BOXCOLLISION:
				// printf("collision ! %d",i);
				//applyOBBImpulseOBB(o,i);
				break;
			case TESTPOINT:
				// printf("test collision ! %d",i);
				applyOBBImpulseOBB(o,i);
				break;
		}
	}
}

void integrate(OBB_s* o, float dt)
{
	if(!o)return;
	//o->position=vaddf(o->position,vmulf(o->velocity,dt));
	//o->position=vaddf(o->position,vect3Df(mulf32(o->velocity.x,dt)>>TIMEPREC,mulf32(o->velocity.y,dt)>>TIMEPREC,mulf32(o->velocity.z,dt)>>TIMEPREC));
	//o->position=vaddf(o->position,vect3Df((mulf32((o->velocity.x+o->oldVelocity.x)/2,dt)>>TIMEPREC),
	//									(mulf32((o->velocity.y+o->oldVelocity.y)/2,dt)>>TIMEPREC),
	//									(mulf32((o->velocity.z+o->oldVelocity.z)/2,dt)>>TIMEPREC)));
	o->position=vaddf(o->position,vect3Df(o->velocity.x*dt,o->velocity.y*dt,o->velocity.z*dt));
	
	float m[9], m2[9];
	//m[0]=0;m[1]=-(mulf32(dt,o->angularVelocity.z));m[2]=(mulf32(dt,o->angularVelocity.y));
    //m[3]=-m[1];m[4]=0;m[5]=-(mulf32(dt,o->angularVelocity.x));
	//m[0]=0;m[1]=-(mulf32(dt,o->angularVelocity.z)>>TIMEPREC);m[2]=(mulf32(dt,o->angularVelocity.y)>>TIMEPREC);
    //m[3]=-m[1];m[4]=0;m[5]=-(mulf32(dt,o->angularVelocity.x)>>TIMEPREC);
	//m[0]=0;m[1]=-((mulf32(dt,(o->angularVelocity.z+o->oldAngularVelocity.z)/2)>>TIMEPREC));m[2]=((mulf32(dt,(o->angularVelocity.y+o->oldAngularVelocity.y)/2)>>TIMEPREC));
    //m[3]=-m[1];m[4]=0;m[5]=-((mulf32(dt,(o->angularVelocity.x+o->oldAngularVelocity.x)/2)>>TIMEPREC));
	m[0]=0;m[1]=-((dt*o->angularVelocity.z));m[2]=((dt*o->angularVelocity.y));
    m[3]=-m[1];m[4]=0;m[5]=-((dt*o->angularVelocity.x));
    m[6]=-m[2];m[7]=-m[5];m[8]=0;
	multMatrix33(m,o->transformationMatrix,m2);
	addMatrix33(o->transformationMatrix,m2,o->transformationMatrix);
	
	//o->velocity=vaddf(o->velocity,vdivf(vmulf(o->forces,dt),o->mass));
	//o->velocity=vaddf(o->velocity,vdivf(vect3Df(mulf32(o->forces.x,dt)>>TIMEPREC,mulf32(o->forces.y,dt)>>TIMEPREC,mulf32(o->forces.z,dt)>>TIMEPREC),o->mass));
	//o->velocity=vaddf(o->velocity,vdivf(vect3Df((mulf32((o->forces.x+o->oldForces.x)/2,dt)>>TIMEPREC),
	//											(mulf32((o->forces.y+o->oldForces.y)/2,dt)>>TIMEPREC),
	//											(mulf32((o->forces.z+o->oldForces.z)/2,dt)>>TIMEPREC)),o->mass));
	o->velocity=vaddf(o->velocity,vdivf(vect3Df(o->forces.x*dt,o->forces.y*dt,o->forces.z*dt),o->mass));
	
	//o->angularMomentum=vaddf(o->angularMomentum,vmulf(o->moment,dt));
	//o->angularMomentum=vaddf(o->angularMomentum,vect3Df(mulf32(o->moment.x,dt)>>TIMEPREC,mulf32(o->moment.y,dt)>>TIMEPREC,mulf32(o->moment.z,dt)>>TIMEPREC));
	//o->angularMomentum=vaddf(o->angularMomentum,vect3Df((mulf32((o->moment.x+o->oldMoment.x)/2,dt)>>TIMEPREC),
	//												(mulf32((o->moment.y+o->oldMoment.y)/2,dt)>>TIMEPREC),
	//												(mulf32((o->moment.z+o->oldMoment.z)/2,dt)>>TIMEPREC)));
	o->angularMomentum=vaddf(o->angularMomentum,vect3Df(o->moment.x*dt,o->moment.y*dt,o->moment.z*dt));
	
	fixMatrix33(o->transformationMatrix);

    // compute auxiliary quantities
	transposeMatrix33(o->transformationMatrix,m2);
	multMatrix33(m2,o->invInertiaMatrix,m);
	multMatrix33(m,o->transformationMatrix,o->invWInertiaMatrix);
	
	o->angularVelocity=vevalf(o->invWInertiaMatrix,o->angularMomentum);
}

void checkOBBCollisions(OBB_s* o, bool sleep)
{
	if(!o)return;
	int i;
	o->numContactPoints=0;
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(objects[i].used && o!=&objects[i] && (!sleep || !objects[i].sleep))
		{
			collideOBBs(o,&objects[i]);
		}
	}
	AARsOBBContacts(o, sleep);
}

void wakeOBBs(void)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(objects[i].used)
		{
			objects[i].counter=0;
			objects[i].sleep=false;
		}
	}
}

void getBoxAABB(OBB_s* o, vect3Df_s* s)
{
	if(!o || !s)return;
	
	s->x=fabs((o->transformationMatrix[0]*o->size.x))+fabs((o->transformationMatrix[1]*o->size.y))+fabs((o->transformationMatrix[2]*o->size.z));
	s->y=fabs((o->transformationMatrix[3]*o->size.x))+fabs((o->transformationMatrix[4]*o->size.y))+fabs((o->transformationMatrix[5]*o->size.z));
	s->z=fabs((o->transformationMatrix[6]*o->size.x))+fabs((o->transformationMatrix[7]*o->size.y))+fabs((o->transformationMatrix[8]*o->size.z));
}

bool intersectAABBAAR(vect3Df_s o1, vect3Df_s s, vect3Df_s o2, vect3Df_s sp)
{
	const vect3Df_s v=vsubf(o2,o1);
	
	if(!sp.x)return !((v.x>s.x || v.x<-s.x) || (v.y-sp.y>s.y || v.y+sp.y<-s.y) || (v.z-sp.z>s.z || v.z+sp.z<-s.z));
	else if(!sp.y)return !((v.y>s.y || v.y<-s.y) || (v.x-sp.x>s.x || v.x+sp.x<-s.x) || (v.z-sp.z>s.z || v.z+sp.z<-s.z));
	else return !((v.z>s.z || v.z<-s.z) || (v.y-sp.y>s.y || v.y+sp.y<-s.y) || (v.x-sp.x>s.x || v.x+sp.x<-s.x));
}

bool intersectOBBPortal(portal_s* p, OBB_s* o)
{
	if(!p || !o)return false;
	
	vect3Df_s s;
	getBoxAABB(o, &s);
	vect3Df_s sp=vect3Df(fabs(p->plane[0].x*PORTAL_WIDTH)+fabs(p->plane[1].x*PORTAL_HEIGHT),fabs(p->plane[0].y*PORTAL_WIDTH)+fabs(p->plane[1].y*PORTAL_HEIGHT),fabs(p->plane[0].z*PORTAL_WIDTH)+fabs(p->plane[1].z*PORTAL_HEIGHT));
		
	return intersectAABBAAR(o->position, s, p->position, sp);
}

float distanceLinePoint(vect3Df_s o, vect3Df_s u, vect3Df_s p);

OBB_s* collideRayBoxes(vect3Df_s o, vect3Df_s u, float l)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		OBB_s* b=&objects[i];
		if(b->used && vdistf(o,b->position)<=l)
		{
			float d=distanceLinePoint(o, u, b->position);
			printf("distance %f\n",d);
			if(d<1.5f)return b;
		}
	}
	return NULL;
}

void setObbVelocity(OBB_s* o, vect3Df_s v)
{
	o->velocity=v;
	o->sleep=false;
	o->counter=0;
}

void ejectPortalOBBs(portal_s* p)
{
	if(!p)return;
	
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(objects[i].used && intersectOBBPortal(p, &objects[i]))
		{
			applyOBBForce(&objects[i], objects[i].position, vmulf(p->normal, -100.0f));
			objects[i].sleep=false;
			objects[i].counter=0;
		}
	}
}

void calculateOBBEnergy(OBB_s* o)
{
	if(!o)return;
	
	float tmp=vdotf(o->velocity,o->velocity)+vdotf(o->angularVelocity,o->angularVelocity);
	// o->energy=(o->energy*9+tmp)/10;
	o->energy=tmp;
}

u8 sleeping;

void simulate(OBB_s* o, float dt)
{
	if(!o)return;
    float currentTime=0;
    float targetTime=dt;
	
	applyOBBForce(o,o->position,vect3Df(0,-(2.0f),0)); //gravity
	o->forces=vaddf(o->forces,vdivf(o->velocity,-25));
	o->moment=vaddf(o->moment,vdivf(o->angularVelocity,-20));
	
	if(!o->sleep)
	{
		while(currentTime<dt)
		{
			OBB_s bkp;
			copyOBB(o,&bkp);

			// cpuStartTiming(0);
			integrate(o,targetTime-currentTime);
			// integ+=cpuEndTiming();

			// cpuStartTiming(0);
			checkOBBCollisions(o, false);
			// coll+=cpuEndTiming();
			
			// cpuStartTiming(0);
			if(o->numContactPoints && o->maxPenetration>PENETRATIONTHRESHOLD)
			{
				targetTime=(currentTime+targetTime)/2;
				copyOBB(&bkp,o);
				if(targetTime-currentTime<=0.000001f)
				{
					// printf("desp impulse\n");
					checkOBBCollisions(o, false);
					applyOBBImpulses(o);
					currentTime=targetTime;
					targetTime=dt;
				}
			}else if(o->numContactPoints)
			{
				// printf("impulse\n");
				applyOBBImpulses(o);
				currentTime=targetTime;
				targetTime=dt;
			}else{
				currentTime=targetTime;
				targetTime=dt;
			}
			// impul+=cpuEndTiming();
		}
		// collideSpherePlatforms(&o->position,o->size.x-8);
	}else sleeping++;

	calculateOBBEnergy(o);
	bool oldSleep=o->sleep;
	// printf("energy %f\n", o->energy);
	if(o->energy>=SLEEPTHRESHOLD*10)o->sleep=false;
	else if(o->energy<=SLEEPTHRESHOLD)
	{
		o->counter++;
		if(o->counter>=SLEEPTIMETHRESHOLD)o->sleep=true;
	}else o->counter=0;

	if(o->sleep)
	{
		if(oldSleep)checkOBBCollisions(o, true); //make it so sleeping collisions don't have to check vs AARs and other sleeping OBBs
		{
			int i;
			bool canSleep=oldSleep;
			for(i=0;i<o->numContactPoints;i++)
			{
				switch(o->contactPoints[i].type)
				{
					case AARCOLLISION:
						canSleep=true;
						break;
					case PLANECOLLISION:
						canSleep=true;
						break;
					case TESTPOINT:
					case BOXCOLLISION:
						{
							OBB_s* o2=(OBB_s*)o->contactPoints[i].target;
							if(o2->energy<=SLEEPTHRESHOLD && o2->counter>=SLEEPTIMETHRESHOLD && o->energy<=SLEEPTHRESHOLD)
							{
								canSleep=true;
								o->sleep=true;
								o2->sleep=true;
								// i=o->numContactPoints; //get a similar system to work with more than 2 boxes in contact ? (only make it for !o2->sleep ?)
							}else{
								canSleep=false;
								o2->sleep=false;
								i=o->numContactPoints;
							}
						}
						break;
				}
			}
			if(!canSleep)o->sleep=false;
		}
	}
		
	o->forces=vect3Df(0,0,0);
	o->moment=vect3Df(0,0,0);
}

bool pointInFrontOfPortal(portal_s* p, vect3Df_s pos, float* z) //assuming correct normal
{
	if(!p)return false;
	const vect3Df_s v2=vsubf(pos, p->position); //then, project onto portal base
	vect3Df_s v=vect3Df(vdotf(p->plane[0],v2),vdotf(p->plane[1],v2),0);
	*z=vdotf(p->normal,v2);
	return (v.y>-PORTAL_HEIGHT && v.y<PORTAL_HEIGHT && v.x>-PORTAL_WIDTH && v.x<PORTAL_WIDTH);
}

extern OBB_s* gravityGunObject;

void updateOBBPortals(OBB_s* o, u8 id, bool init)
{
	if(!o&&id<2)return;

	float z;
	o->oldPortal[id]=o->portal[id];
	o->portal[id]=((vdotf(vsubf(o->position,portals[id].position),portals[id].normal)>0)&1)|(((pointInFrontOfPortal(&portals[id],o->position,&z))&1)<<1);

	// printf("portal %d %f\n",o->portal[id]&2,z);
	
	if(init)
	{
		o->oldPortal[id]=o->portal[id];
	}else if(portals[id].target && (z>=0.0f && o->oldPortalZ[id]<=0.0f))
	{
		// printf("WARP object (%d %d)\n", (int)(o->portal[id]&2 && z<=0.0f && z>-0.1f), (int)(((o->oldPortal[id]&1) && !(o->portal[id]&1) && (o->oldPortal[id]&2 || o->portal[id]&2))));
		if(o == gravityGunObject)gravityGunObject = NULL; // TEMP : TODO better solution
		o->position=vaddf(portals[id].target->position,warpPortalVector(&portals[id],vsubf(o->position,portals[id].position)));
		o->velocity=warpPortalVector(&portals[id],o->velocity);
		o->forces=warpPortalVector(&portals[id],o->forces);
		o->angularVelocity=warpPortalVector(&portals[id],o->angularVelocity);
		o->moment=warpPortalVector(&portals[id],o->moment);
		
		warpPortalMatrix(&portals[id], o->transformationMatrix);
		warpPortalMatrix(&portals[id], o->invWInertiaMatrix);
		
		o->portaled=true;
	}
	o->oldPortalZ[id] = z;
}

void updateOBB(OBB_s* o)
{
	if(!o)return;
	
	simulate(o,0.015f);

	int i;
	for(i=0; i < NUM_PORTALS; i++)
	{
		if(portals[i].target)updateOBBPortals(o,i,false);
	}
}

void updateOBBs(void)
{
	int i;
	sleeping=0;
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(objects[i].used)
		{
			updateOBB(&objects[i]);
		}
	}
}

OBB_s* createOBB(vect3Df_s position, vect3Df_s size, md2_instance_t* model, float mass, float angle)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(!objects[i].used)
		{
			initOBB(&objects[i], position, size, model, mass, angle);
			printf("created object %d\n",i);
			return &objects[i];
		}
	}
	return NULL;
}

void drawOBBs(void)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(objects[i].used)
		{
			if(keysHeld()&KEY_SELECT)printf("drawing object %d %p\n",i,objects[i].modelInstance);
			drawOBB(&objects[i]);
		}
	}
}

void drawOBB(OBB_s* o)
{
	if(!o || !o->modelInstance)return;

	gsPushMatrix();
		gsSwitchRenderMode(md2GsMode);
		gsTranslate(o->position.x,o->position.y,o->position.z);
		gsMultMatrix3(o->transformationMatrix);
		if(keysHeld()&KEY_SELECT)printf("D%d : %f %f %f\n", (int)o->sleep, o->position.x, o->position.y, o->position.z);
		md2InstanceDraw(o->modelInstance);
	gsPopMatrix();
}
