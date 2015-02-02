#include <string.h>
#include "physics/PIC.h"
#include "physics/AAR.h"
#include "physics/OBB.h"
#include "game/portal.h"

#define ALLOCATORSIZE (9*1024) //may cause problems !

u8 allocatorPool[ALLOCATORSIZE];
u16 allocatorCounter;

AAR_s aaRectangles[NUMAARS];
grid_s AARgrid;

static const u8 AARSegments[NUMAARSEGMENTS][2]={{0,1},{1,2},{3,2},{0,3}};
static const u8 AARSegmentsPD[NUMAARSEGMENTS][2]={{0,0},{1,1},{3,0},{0,1}};

u32 nodeSize=ORIGNODESIZE;

void initAllocator(void)
{
	allocatorCounter=0;
}

void* allocateData(u16 size)
{
	if(allocatorCounter+size>ALLOCATORSIZE)return NULL;
	allocatorCounter+=size;
	// fifoSendValue32(FIFO_USER_08,allocatorCounter);
	return &allocatorPool[allocatorCounter-size];
}

void initAARs(void)
{
	int i;
	for(i=0;i<NUMAARS;i++)
	{
		aaRectangles[i].used=false;
	}
	
	AARgrid.nodes=NULL;
	AARgrid.width=AARgrid.height=0;
	
	initAllocator();
}

void freeGrid(grid_s* g)
{
	if(!g || !g->nodes)return;
	g->nodes=NULL;
	initAllocator();
}

void generateGrid(grid_s* g)
{
	if(!g)g=&AARgrid;
	
	freeGrid(g);
	
	int i;
	bool b=false;
	vect3Df_s m, M;
	m=vect3Df((1<<29),(1<<29),(1<<29));
	M=vect3Df(-(1<<29),-(1<<29),-(1<<29));
	for(i=0;i<NUMAARS;i++)
	{
		if(aaRectangles[i].used)
		{
			if(aaRectangles[i].position.x<m.x)m.x=aaRectangles[i].position.x;
			if(aaRectangles[i].position.z<m.z)m.z=aaRectangles[i].position.z;
			
			if(aaRectangles[i].position.x+aaRectangles[i].size.x>M.x)M.x=aaRectangles[i].position.x+aaRectangles[i].size.x;
			if(aaRectangles[i].position.z+aaRectangles[i].size.z>M.z)M.z=aaRectangles[i].position.z+aaRectangles[i].size.z;
			b=true;
		}
	}
	if(!b)return;

	nodeSize=ORIGNODESIZE;

	g->width=(M.x-m.x)/NODESIZE+1;
	g->height=(M.z-m.z)/NODESIZE+1;
	
	while((sizeof(node_s)*g->width*g->height)>(ALLOCATORSIZE/3))
	{
		nodeSize*=2;
		g->width=(M.x-m.x)/NODESIZE+1;
		g->height=(M.z-m.z)/NODESIZE+1;
	}

	g->nodes=allocateData(sizeof(node_s)*g->width*g->height);

	g->m=m;
	g->M=M;
	
	// fifoSendValue32(FIFO_USER_08,allocatorCounter);
	
	static u16 temp[NUMAARS];
	
	int j, k;
	for(i=0;i<g->width;i++)
	{
		for(j=0;j<g->height;j++)
		{
			node_s* n=&g->nodes[i+j*g->width];
			n->length=0;
			for(k=0;k<NUMAARS;k++)
			{
				if(aaRectangles[k].used)
				{
					const float mX=g->m.x+i*NODESIZE, MX=g->m.x+(i+1)*NODESIZE;
					const float mZ=g->m.z+j*NODESIZE, MZ=g->m.z+(j+1)*NODESIZE;
					if(!((aaRectangles[k].position.x<mX && aaRectangles[k].position.x+aaRectangles[k].size.x<mX)
					||   (aaRectangles[k].position.x>MX && aaRectangles[k].position.x+aaRectangles[k].size.x>MX)
					||   (aaRectangles[k].position.z<mZ && aaRectangles[k].position.z+aaRectangles[k].size.z<mZ)
					||   (aaRectangles[k].position.z>MZ && aaRectangles[k].position.z+aaRectangles[k].size.z>MZ)))
					{
						temp[n->length]=k;
						n->length++;
					}
				}
			}
			if(n->length){n->data=allocateData(sizeof(u16)*n->length);memcpy(n->data,temp,n->length*sizeof(u16));}
			else n->data=NULL;
		}
	}
	// fifoSendValue32(FIFO_USER_08,allocatorCounter);
}

void getOBBNodes(grid_s* g, OBB_s* o, u16* x, u16* X, u16* z, u16* Z)
{
	if(!o)return;
	if(!g)g=&AARgrid;
	
	const vect3Df_s m=vsubf(o->AABBo,g->m);
	const vect3Df_s M=vaddf(m,o->AABBs);
	
	*x=m.x/NODESIZE;*z=m.z/NODESIZE;
	*X=M.x/NODESIZE;*Z=M.z/NODESIZE;
}

AAR_s* createAAR(u16 id, vect3Df_s position, vect3Df_s size, vect3Df_s normal)
{
	int i=id;
	// for(i=0;i<NUMAARS;i++)
	// {
		if(!aaRectangles[i].used)
		{
			aaRectangles[i].used=true;
			aaRectangles[i].position=position;
			aaRectangles[i].size=size;
			aaRectangles[i].normal=normal;
			// fifoSendValue32(FIFO_USER_08,i);
			return &aaRectangles[i];
		}
	// }
	return NULL;
}

void updateAAR(u16 id, vect3Df_s position)
{
	int i=id;
	// for(i=0;i<NUMAARS;i++)
	// {
		if(aaRectangles[i].used)
		{
			aaRectangles[i].position=position;
		}
	// }
}

void toggleAAR(u16 id)
{
	if(id>=NUMAARS)return;
	aaRectangles[id].used^=1;
}

bool pointInPortal(portal_s* p, vect3Df_s pos) //assuming correct normal
{
	if(!p)return false;
	const vect3Df_s v2=vsubf(pos, p->position); //then, project onto portal base
	vect3Df_s v=vect3Df(vdotf(p->plane[0],v2),vdotf(p->plane[1],v2),vdotf(p->normal,v2));
	return (fabs(v.z)<16 && v.y>-PORTALSIZEY*4 && v.y<PORTALSIZEY*4 && v.x>-PORTALSIZEX*4 && v.x<PORTALSIZEX*4);
}

void OBBAARContacts(AAR_s* a, OBB_s* o, bool port)
{
	if(!a || !o || !o->used || !a->used)return;
	
	vect3Df_s u[3];
		u[0]=vect3Df(o->transformationMatrix[0],o->transformationMatrix[3],o->transformationMatrix[6]);
		u[1]=vect3Df(o->transformationMatrix[1],o->transformationMatrix[4],o->transformationMatrix[7]);
		u[2]=vect3Df(o->transformationMatrix[2],o->transformationMatrix[5],o->transformationMatrix[8]);
		
	vect3Df_s v[4], vv[4];
	vect3Df_s uu[2], uuu[2];
		v[0]=vsubf(a->position,o->position);
		v[2]=vaddf(v[0],a->size);
		if(a->normal.x)
		{
			uu[0]=vect3Df(0,(1.0f),0);
			uu[1]=vect3Df(0,0,(1.0f));
			
			v[1]=vaddf(v[0],vect3Df(0,a->size.y,0));
			v[3]=vaddf(v[0],vect3Df(0,0,a->size.z));
		}else if(a->normal.y)
		{
			uu[0]=vect3Df((1.0f),0,0);
			uu[1]=vect3Df(0,0,(1.0f));
			
			v[1]=vaddf(v[0],vect3Df(a->size.x,0,0));
			v[3]=vaddf(v[0],vect3Df(0,0,a->size.z));
		}else{
			uu[0]=vect3Df((1.0f),0,0);
			uu[1]=vect3Df(0,(1.0f),0);
			
			v[1]=vaddf(v[0],vect3Df(a->size.x,0,0));
			v[3]=vaddf(v[0],vect3Df(0,a->size.y,0));
		}
	int i;
	for(i=0;i<4;i++) //optimizeable
	{
		vv[i]=vect3Df(vdotf(v[i],u[0]),vdotf(v[i],u[1]),vdotf(v[i],u[2]));
		v[i]=vaddf(v[i],o->position);
	}
	uuu[0]=vect3Df(vdotf(uu[0],u[0]),vdotf(uu[0],u[1]),vdotf(uu[0],u[2]));
	uuu[1]=vect3Df(vdotf(uu[1],u[0]),vdotf(uu[1],u[1]),vdotf(uu[1],u[2]));

	float ss[3];
		ss[0]=o->size.x;//+MAXPENETRATIONBOX;
		ss[1]=o->size.y;//+MAXPENETRATIONBOX;
		ss[2]=o->size.z;//+MAXPENETRATIONBOX;
	
	for(i=0;i<NUMAARSEGMENTS;i++)
	{
		vect3Df_s p1=v[AARSegments[i][0]];
		vect3Df_s p2=v[AARSegments[i][1]];
		vect3Df_s uu1=vv[AARSegments[i][0]];
		vect3Df_s uu2=vv[AARSegments[i][1]];
		vect3Df_s n1, n2;
		float k1, k2;
		bool b1, b2;
		if(clipSegmentOBB(ss, u, &p1, &p2, uu[AARSegmentsPD[i][1]], &uu1, &uu2, uuu[AARSegmentsPD[i][1]], &n1, &n2, &b1, &b2, &k1, &k2))
		{
			if(b1)
			{
				bool b=false;
				// if(port)
				// {
				// 	if((portal[0].normal.x&&a->normal.x)||(portal[0].normal.y&&a->normal.y)||(portal[0].normal.z&&a->normal.z))b=pointInPortal(&portal[0],p1);
				// 	if(!b&&((portal[1].normal.x&&a->normal.x)||(portal[1].normal.y&&a->normal.y)||(portal[1].normal.z&&a->normal.z)))b=pointInPortal(&portal[1],p1);
				// }
				if(!b)
				{
					//p1=vaddf(p1,vmulf(vv,k1));
					o->contactPoints[o->numContactPoints].point=p1;
					o->contactPoints[o->numContactPoints].type=PLANECOLLISION;
					o->contactPoints[o->numContactPoints].normal=a->normal;
					o->contactPoints[o->numContactPoints].penetration=0;
					o->contactPoints[o->numContactPoints].target=NULL;
					o->numContactPoints++;
				}
			}
			if(b2)
			{
				bool b=false;
				// if(port)
				// {
				// 	if((portal[0].normal.x&&a->normal.x)||(portal[0].normal.y&&a->normal.y)||(portal[0].normal.z&&a->normal.z))b=pointInPortal(&portal[0],p2);
				// 	if(!b&&((portal[1].normal.x&&a->normal.x)||(portal[1].normal.y&&a->normal.y)||(portal[1].normal.z&&a->normal.z)))b=pointInPortal(&portal[1],p2);
				// }
				if(!b)
				{
					//p2=vaddf(p2,vmulf(vv,k2));
					o->contactPoints[o->numContactPoints].point=p2;
					o->contactPoints[o->numContactPoints].type=PLANECOLLISION;
					o->contactPoints[o->numContactPoints].normal=a->normal;
					o->contactPoints[o->numContactPoints].penetration=0;
					o->contactPoints[o->numContactPoints].target=NULL;
					o->numContactPoints++;
				}
			}
		}
	}
}

bool AAROBBContacts(AAR_s* a, OBB_s* o, vect3Df_s* v, bool port)
{
	if(!a || !o || !o->used || !a->used)return false;
	
	u16 oldnum=o->numContactPoints;

	vect3Df_s u[3];
		u[0]=vect3Df(o->transformationMatrix[0],o->transformationMatrix[3],o->transformationMatrix[6]);
		u[1]=vect3Df(o->transformationMatrix[1],o->transformationMatrix[4],o->transformationMatrix[7]);
		u[2]=vect3Df(o->transformationMatrix[2],o->transformationMatrix[5],o->transformationMatrix[8]);

	if(a->normal.x)
	{
		if(a->position.x>o->AABBo.x+o->AABBs.x || a->position.x<o->AABBo.x)return false;
		int i;
		bool vb[8];
		for(i=0;i<8;i++)vb[i]=v[i].x>a->position.x;
		for(i=0;i<NUMOBBSEGMENTS;i++) //possible to only check half !
		{
			if(vb[OBBSegments[i][0]]!=vb[OBBSegments[i][1]])
			{
				const vect3Df_s uu=v[OBBSegmentsPD[i][0]];
				const vect3Df_s vv=u[OBBSegmentsPD[i][1]];
				const float k=(fabs(uu.x-a->position.x)/fabs(vv.x));
				const vect3Df_s p=vaddf(uu,vmulf(vv,k));
				if(p.y>a->position.y && p.y<a->position.y+a->size.y && p.z>a->position.z && p.z<a->position.z+a->size.z)
				{
					bool b=false;
					// if(port)
					// {
					// 	if(portal[0].normal.x)b=pointInPortal(&portal[0],p);
					// 	if(!b&&portal[1].normal.x)b=pointInPortal(&portal[1],p);
					// }
					if(!b)
					{
						o->contactPoints[o->numContactPoints].point=p;
						o->contactPoints[o->numContactPoints].type=AARCOLLISION;
						o->contactPoints[o->numContactPoints].normal=a->normal;
						o->contactPoints[o->numContactPoints].penetration=0;
						o->contactPoints[o->numContactPoints].target=NULL;
						o->numContactPoints++;
					}
				}
			}
		}
	}else if(a->normal.y)
	{
		if(a->position.y>o->AABBo.y+o->AABBs.y || a->position.y<o->AABBo.y)return false;

		int i;
		bool vb[8];
		for(i=0;i<8;i++)vb[i]=v[i].y>a->position.y;
		for(i=0;i<NUMOBBSEGMENTS;i++)
		{
			if(vb[OBBSegments[i][0]]!=vb[OBBSegments[i][1]])
			{
				const vect3Df_s uu=v[OBBSegmentsPD[i][0]];
				const vect3Df_s vv=u[OBBSegmentsPD[i][1]];
				const float k=(fabs(uu.y-a->position.y)/fabs(vv.y));
				const vect3Df_s p=vaddf(uu,vmulf(vv,k));
				if(p.x>a->position.x && p.x<a->position.x+a->size.x && p.z>a->position.z && p.z<a->position.z+a->size.z)
				{
					bool b=false;
					// if(port)
					// {
					// 	if(portal[0].normal.y)b=pointInPortal(&portal[0],p);
					// 	if(!b&&portal[1].normal.y)b=pointInPortal(&portal[1],p);
					// }
					if(!b)
					{
						o->contactPoints[o->numContactPoints].point=p;
						o->contactPoints[o->numContactPoints].type=AARCOLLISION;
						o->contactPoints[o->numContactPoints].normal=a->normal;
						o->contactPoints[o->numContactPoints].penetration=0;
						o->contactPoints[o->numContactPoints].target=NULL;
						o->numContactPoints++;
					}
				}
			}
		}
	}else{
		if(a->position.z>o->AABBo.z+o->AABBs.z || a->position.z<o->AABBo.z)return false;
		int i;
		bool vb[8];
		for(i=0;i<8;i++)vb[i]=v[i].z>a->position.z;
		for(i=0;i<NUMOBBSEGMENTS;i++)
		{
			if(vb[OBBSegments[i][0]]!=vb[OBBSegments[i][1]])
			{
				const vect3Df_s uu=v[OBBSegmentsPD[i][0]];
				const vect3Df_s vv=u[OBBSegmentsPD[i][1]];
				const float k=(fabs(uu.z-a->position.z)/fabs(vv.z));
				const vect3Df_s p=vaddf(uu,vmulf(vv,k));
				if(p.x>a->position.x && p.x<a->position.x+a->size.x && p.y>a->position.y && p.y<a->position.y+a->size.y)
				{
					bool b=false;
					// if(port)
					// {
					// 	if(portal[0].normal.z)b=pointInPortal(&portal[0],p);
					// 	if(!b&&portal[1].normal.z)b=pointInPortal(&portal[1],p);
					// }
					if(!b)
					{
						o->contactPoints[o->numContactPoints].point=p;
						o->contactPoints[o->numContactPoints].type=AARCOLLISION;
						o->contactPoints[o->numContactPoints].normal=a->normal;
						o->contactPoints[o->numContactPoints].penetration=0;
						o->contactPoints[o->numContactPoints].target=NULL;
						o->numContactPoints++;
					}
				}
			}
		}
	}

	OBBAARContacts(a, o, port);
	
	return o->numContactPoints>oldnum;
}

void AARsOBBContacts(OBB_s* o, bool sleep)
{
	int i, j, k;
	// bool port=portal[0].used&&portal[1].used;
	bool port=false;
	vect3Df_s v[8];
	getOBBVertices(o,v);
	if(!sleep)
	{
		u16 x, X, z, Z;
		getOBBNodes(NULL, o, &x, &X, &z, &Z);
		bool lalala[NUMAARS];
		for(i=0;i<NUMAARS;i++)lalala[i]=0;
		o->groundID=-1;
		for(i=x;i<=X;i++)
		{
			for(j=z;j<=Z;j++)
			{
				node_s* n=&AARgrid.nodes[i+j*AARgrid.width];
				for(k=0;k<n->length;k++)
				{
					u16 old=o->numContactPoints;
					
					if(!lalala[n->data[k]])
					{
						AAROBBContacts(&aaRectangles[n->data[k]], o, v, port);
					}
					if(o->groundID<0 && o->numContactPoints>old && aaRectangles[n->data[k]].normal.y>0)o->groundID=n->data[k];
					lalala[n->data[k]]=1;
				}
			}
		}
		// if(port)
		// {
		// 	AAROBBContacts(&portal[0].guideAAR[0], o, v, false);
		// 	AAROBBContacts(&portal[0].guideAAR[1], o, v, false);
		// 	AAROBBContacts(&portal[0].guideAAR[2], o, v, false);
		// 	AAROBBContacts(&portal[0].guideAAR[3], o, v, false);

		// 	AAROBBContacts(&portal[1].guideAAR[0], o, v, false);
		// 	AAROBBContacts(&portal[1].guideAAR[1], o, v, false);
		// 	AAROBBContacts(&portal[1].guideAAR[2], o, v, false);
		// 	AAROBBContacts(&portal[1].guideAAR[3], o, v, false);
		// }
	}
	// collideOBBPlatforms(o, v);
}

void fixAAR(AAR_s* a)
{
	if(!a)return;
	
	if(a->size.x<0){a->position.x+=a->size.x;a->size.x=-a->size.x;}
	if(a->size.y<0){a->position.y+=a->size.y;a->size.y=-a->size.y;}
	if(a->size.z<0){a->position.z+=a->size.z;a->size.z=-a->size.z;}
}

void generateGuidAAR(portal_s* p)
{
	if(!p)return;
	
	p->guideAAR[0].used=true;
	p->guideAAR[0].position=vsubf(p->position,vmulf(vaddf(vdivf(p->plane[0],PORTALFRACTIONX),vdivf(p->plane[1],PORTALFRACTIONY)),4));
	p->guideAAR[0].size=vmulf(vaddf(vdivf(vmulf(p->plane[0],2),PORTALFRACTIONX),vdivf(p->normal,-8)),4);
	p->guideAAR[0].normal=p->plane[1];
	fixAAR(&p->guideAAR[0]);
	
	p->guideAAR[1].used=true;
	p->guideAAR[1].position=vsubf(p->position,vmulf(vaddf(vdivf(p->plane[0],PORTALFRACTIONX),vdivf(p->plane[1],PORTALFRACTIONY)),4));
	p->guideAAR[1].size=vmulf(vaddf(vdivf(vmulf(p->plane[1],2),PORTALFRACTIONX),vdivf(p->normal,-8)),4);
	p->guideAAR[1].normal=p->plane[0];
	fixAAR(&p->guideAAR[1]);
	
	p->guideAAR[2].used=true;
	p->guideAAR[2].position=vaddf(p->position,vmulf(vaddf(vdivf(p->plane[0],PORTALFRACTIONX),vdivf(p->plane[1],PORTALFRACTIONY)),4));
	p->guideAAR[2].size=vmulf(vaddf(vdivf(vmulf(p->plane[0],2),PORTALFRACTIONX),vdivf(p->normal,-8)),4);
	p->guideAAR[2].normal=vmulf(p->plane[1],-1);
	fixAAR(&p->guideAAR[2]);
	
	p->guideAAR[3].used=true;
	p->guideAAR[3].position=vsubf(p->position,vmulf(vaddf(vdivf(p->plane[0],PORTALFRACTIONX),vdivf(p->plane[1],PORTALFRACTIONY)),4));
	p->guideAAR[3].size=vmulf(vaddf(vdivf(vmulf(p->plane[1],2),PORTALFRACTIONX),vdivf(p->normal,-8)),4);
	p->guideAAR[3].normal=vmulf(p->plane[0],-1);
	fixAAR(&p->guideAAR[3]);
}
	
