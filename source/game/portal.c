#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/portal.h"
#include "utils/math.h"
#include "gfx/gs.h"

#include "portal_vsh_shbin.h"

#define PORTAL_OUTLINE (0.2f)
#define PORTAL_DETAIL (32)

#define PORTALMARGIN (0.2f)

DVLB_s* portalDvlb;
shaderProgram_s portalProgram;
extern u32 __linear_heap;
#define portalBaseAddr __linear_heap

portal_s portals[NUM_PORTALS];

typedef struct
{
	vect3Df_s position;
}portalVertex_s;

portalVertex_s* portalVertexData;
int portalNumVertices;
portalVertex_s* portalOutlineVertexData;
int portalOutlineNumVertices;

void resetPortals()
{
	int i;
	for(i=0; i<NUM_PORTALS; i++)
	{
		portals[i].draw = false;
		portals[i].open = false;
	}
}

void generatePortalGeometry(portalVertex_s** v1, int* n1, portalVertex_s** v2, int* n2)
{
	if(!v1 || !v2 || !n1 || !n2)return;

	*v1 = linearAlloc(sizeof(portalVertex_s)*(PORTAL_DETAIL+1)*2);
	*v2 = linearAlloc(sizeof(portalVertex_s)*(PORTAL_DETAIL+1)*2);
	if(!*v1 || !*v2)return;

	*n1 = 0;
	*n2 = 0;

	int i;
	for(i=0; i<PORTAL_DETAIL+1; i++)
	{
		(*v1)[(*n1)++] = (portalVertex_s){(vect3Df_s){0.0f, 0.0f, 0.0f}};
		(*v1)[(*n1)++] = (portalVertex_s){(vect3Df_s){cos((i*2*M_PI)/PORTAL_DETAIL)*PORTAL_WIDTH, sin((i*2*M_PI)/PORTAL_DETAIL)*PORTAL_HEIGHT, 0.0f}};

		(*v2)[(*n2)++] = (portalVertex_s){(vect3Df_s){cos((i*2*M_PI)/PORTAL_DETAIL)*PORTAL_WIDTH, sin((i*2*M_PI)/PORTAL_DETAIL)*PORTAL_HEIGHT, 0.0f}};
		(*v2)[(*n2)++] = (portalVertex_s){(vect3Df_s){cos((i*2*M_PI)/PORTAL_DETAIL)*(PORTAL_WIDTH+PORTAL_OUTLINE), sin((i*2*M_PI)/PORTAL_DETAIL)*(PORTAL_HEIGHT+PORTAL_OUTLINE), 0.0f}};
	}
}

void portalInit()
{
	portalDvlb = DVLB_ParseFile((u32*)portal_vsh_shbin, portal_vsh_shbin_size);

	if(!portalDvlb)return;

	shaderProgramInit(&portalProgram);
	shaderProgramSetVsh(&portalProgram, &portalDvlb->DVLE[0]);

	generatePortalGeometry(&portalVertexData, &portalNumVertices, &portalOutlineVertexData, &portalOutlineNumVertices);

	int i;
	for(i=0; i<NUM_PORTALS; i++)initPortal(&portals[i]);
}

void portalExit()
{
	if(portalVertexData)linearFree(portalVertexData);
	if(portalOutlineVertexData)linearFree(portalOutlineVertexData);
}

void initPortal(portal_s* p)
{
	if(!p)return;

	// p->position = vect3Df();
	p->target = NULL;
	updatePortalOrientation(p, vect3Df(0.0f, 0.0f, 1.0f), vect3Df(-1.0f, 0.0f, 0.0f));
	p->color = vect3Df(1.0f, 1.0f, 1.0f);
	p->open = false;
	p->draw = false;
}

void updatePortalOrientation(portal_s* p, vect3Df_s plane0, vect3Df_s normal)
{
	if(!p)return;

	p->normal = normal;
	p->plane[0] = plane0;
	p->plane[1] = vprodf(p->normal, p->plane[0]);

	p->matrix[0+0*4] = p->plane[0].x;
	p->matrix[0+1*4] = p->plane[0].y;
	p->matrix[0+2*4] = p->plane[0].z;
	p->matrix[0+3*4] = 0.0f;
	
	p->matrix[1+0*4] = p->plane[1].x;
	p->matrix[1+1*4] = p->plane[1].y;
	p->matrix[1+2*4] = p->plane[1].z;
	p->matrix[1+3*4] = 0.0f;

	p->matrix[2+0*4] = p->normal.x;
	p->matrix[2+1*4] = p->normal.y;
	p->matrix[2+2*4] = p->normal.z;
	p->matrix[2+3*4] = 0.0f;

	p->matrix[3+0*4] = 0.0f;
	p->matrix[3+1*4] = 0.0f;
	p->matrix[3+2*4] = 0.0f;
	p->matrix[3+3*4] = 1.0f;

	generateGuideAAR(p);
}

vect3Df_s warpPortalVector(portal_s* p, vect3Df_s v)
{
	if(!p)return vect3Df(0.0f, 0.0f, 0.0f);
	portal_s* p2=p->target;
	if(!p2)return vect3Df(0.0f, 0.0f, 0.0f);

	v = multMatrix44Vect3(p->matrix, v, true);

	// rotation
	v.x = -v.x;
	v.z = -v.z;
	
	return multMatrix44Vect3(p2->matrix, v, false);
}

void warpPortalMatrix(portal_s* p, float* m) //3x3
{
	if(!m)return;
	
	vect3Df_s x=warpPortalVector(p,vect3Df(m[0],m[3],m[6]));
	vect3Df_s y=warpPortalVector(p,vect3Df(m[1],m[4],m[7]));
	vect3Df_s z=warpPortalVector(p,vect3Df(m[2],m[5],m[8]));
	
	m[0]=x.x;m[3]=x.y;m[6]=x.z;
	m[1]=y.x;m[4]=y.y;m[7]=y.z;
	m[2]=z.x;m[5]=z.y;m[8]=z.z;
}

bool isPointInPortal(portal_s* p, vect3Df_s o, vect3Df_s *v, float* x, float* y, float* z)
{
	if(!x || !y || !z || !v || !p)return false;
	*v=vsubf(o,p->position);
	const vect3Df_s u1=p->plane[0], u2=p->plane[1];
	*x=vdotf(*v,u1);
	*y=vdotf(*v,u2);
	*z=vdotf(*v,p->normal);
	return !(*x<-PORTAL_WIDTH || *y<-PORTAL_HEIGHT || *x>=PORTAL_WIDTH || *y>=PORTAL_HEIGHT || *z>3.0f);
}

void GPU_SetScissorTest_(GPU_SCISSORMODE mode, u32 x, u32 y, u32 w, u32 h)
{
	u32 param[4];
	
	param[0x0] = mode;
	param[0x1] = (y<<16)|(x&0xFFFF);
	param[0x2] = ((h-1)<<16)|((w-1)&0xFFFF);
	GPUCMD_AddIncrementalWrites(GPUREG_SCISSORTEST_MODE, param, 0x00000003);

	//enable depth buffer
	param[0x0]=0x0000000F;
	param[0x1]=0x0000000F;
	param[0x2]=0x00000002;
	param[0x3]=0x00000002;
	GPUCMD_AddIncrementalWrites(GPUREG_0112, param, 0x00000004);
}

void getPortalBoundingBox(portal_s* p, camera_s* c, vect3Di_s* topleft, vect3Di_s* bottomright, float* depth)
{
	if(!p || !topleft || !bottomright || !depth)return;

	vect4Df_s v;
	float mX=1.0f, MX=-1.0f;
	float mY=1.0f, MY=-1.0f;

	*depth = 1000.0f;

	v = projectPointCamera(c, vaddf(p->position, vect3Df(p->plane[0].x*PORTAL_WIDTH+p->plane[1].x*PORTAL_HEIGHT, p->plane[0].y*PORTAL_WIDTH+p->plane[1].y*PORTAL_HEIGHT, p->plane[0].z*PORTAL_WIDTH+p->plane[1].z*PORTAL_HEIGHT)));
	if(v.x < mX) mX = v.x;
	if(v.y < mY) mY = v.y;
	if(v.x > MX) MX = v.x;
	if(v.y > MY) MY = v.y;
	if(*depth > -v.w) *depth = -v.w;

	v = projectPointCamera(c, vaddf(p->position, vect3Df(p->plane[0].x*PORTAL_WIDTH-p->plane[1].x*PORTAL_HEIGHT, p->plane[0].y*PORTAL_WIDTH-p->plane[1].y*PORTAL_HEIGHT, p->plane[0].z*PORTAL_WIDTH-p->plane[1].z*PORTAL_HEIGHT)));
	if(v.x < mX) mX = v.x;
	if(v.y < mY) mY = v.y;
	if(v.x > MX) MX = v.x;
	if(v.y > MY) MY = v.y;
	if(*depth > -v.w) *depth = -v.w;

	v = projectPointCamera(c, vaddf(p->position, vect3Df(-p->plane[0].x*PORTAL_WIDTH-p->plane[1].x*PORTAL_HEIGHT, -p->plane[0].y*PORTAL_WIDTH-p->plane[1].y*PORTAL_HEIGHT, -p->plane[0].z*PORTAL_WIDTH-p->plane[1].z*PORTAL_HEIGHT)));
	if(v.x < mX) mX = v.x;
	if(v.y < mY) mY = v.y;
	if(v.x > MX) MX = v.x;
	if(v.y > MY) MY = v.y;
	if(*depth > -v.w) *depth = -v.w;

	v = projectPointCamera(c, vaddf(p->position, vect3Df(-p->plane[0].x*PORTAL_WIDTH+p->plane[1].x*PORTAL_HEIGHT, -p->plane[0].y*PORTAL_WIDTH+p->plane[1].y*PORTAL_HEIGHT, -p->plane[0].z*PORTAL_WIDTH+p->plane[1].z*PORTAL_HEIGHT)));
	if(v.x < mX) mX = v.x;
	if(v.y < mY) mY = v.y;
	if(v.x > MX) MX = v.x;
	if(v.y > MY) MY = v.y;
	if(*depth > -v.w) *depth = -v.w;

	topleft->x = (mX+1.0f)*240/2;
	topleft->y = (mY+1.0f)*400/2;

	bottomright->x = (MX+1.0f)*240/2;
	bottomright->y = (MY+1.0f)*400/2;

	if(topleft->x < 0) topleft->x = 0;
	if(topleft->y < 0) topleft->y = 0;

	if(bottomright->x > 240) bottomright->x = 240;
	if(bottomright->y > 400) bottomright->y = 400;
}

#define stencilValue(i, stencil) ((depth*n*2+(i)+1)^(stencil))

void drawPortals(portal_s* portals[], int n, renderSceneCallback_t callback, camera_s* c, int depth, u8 stencil)
{
	if(!portals || !portalVertexData || !callback || !c || !depth)return;
	int i;

	gsSwitchRenderMode(-1);

	GPU_SetAttributeBuffers(
		1, // number of attributes
		(u32*)osConvertVirtToPhys(portalBaseAddr), // we use the start of linear heap as base since that's where all our buffers are located
		GPU_ATTRIBFMT(0, 3, GPU_FLOAT), // we want v0 (vertex position)
		0xFFE, // mask : we want v0
		0x0, // permutation : we use identity
		1, // number of buffers : we have one attribute per buffer
		(u32[]){(u32)portalOutlineVertexData-portalBaseAddr}, // buffer offsets (placeholders)
		(u64[]){0x0}, // attribute permutations for each buffer
		(u8[]){1} // number of attributes for each buffer
		);

	gsPushMatrix();
		shaderInstanceSetBool(portalProgram.vertexShader, 0, true);
		gsSetShader(&portalProgram);

		//TEMP
		int colorUniformLoc = shaderInstanceGetUniformLocation(portalProgram.vertexShader, "color");

		GPU_SetTexEnv(0, 
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
			GPU_TEVOPERANDS(0,0,0), 
			GPU_TEVOPERANDS(0,0,0), 
			GPU_MODULATE, GPU_MODULATE, 
			0xFFFFFFFF);

		for(i=0; i<n; i++)
		{
			portal_s* p = portals[i];

			if(!p->draw)continue;

			GPU_SetFloatUniform(GPU_VERTEX_SHADER, colorUniformLoc, (u32*)(float[]){1.0f, p->color.z, p->color.y, p->color.x}, 1);

			gsPushMatrix();
				gsTranslate(p->position.x, p->position.y, p->position.z);
				gsMultMatrix(p->matrix);

				gsUpdateTransformation();

				GPU_DrawArray(GPU_TRIANGLE_STRIP, 0, portalNumVertices);
			gsPopMatrix();
		}
			
		GPU_SetTexEnv(0, 
			GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
			GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
			GPU_TEVOPERANDS(0,0,0), 
			GPU_TEVOPERANDS(0,0,0), 
			GPU_MODULATE, GPU_MODULATE, 
			0xFFFFFFFF);

		GPU_SetFloatUniform(GPU_VERTEX_SHADER, colorUniformLoc, (u32*)(float[]){1.0f, 1.0f, 1.0f, 1.0f}, 1);
		GPUCMD_AddWrite(GPUREG_ATTRIBBUFFER0_CONFIG0, (u32)portalVertexData-portalBaseAddr);

		GPU_SetDepthTestAndWriteMask(true, GPU_GEQUAL, GPU_WRITE_COLOR);
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_INVERT);

		for(i=0; i<n; i++)
		{
			portal_s* p = portals[i];
			
			if(!p->draw)continue;
			if(!p->target || !p->target->open)continue;

			GPU_SetStencilTest(true, GPU_EQUAL, stencil, 0xFF, stencilValue(i, 0));

			gsPushMatrix();
				gsTranslate(p->position.x, p->position.y, p->position.z);
				gsMultMatrix(p->matrix);

				gsUpdateTransformation();

				GPU_DrawArray(GPU_TRIANGLE_STRIP, 0, portalNumVertices);
			gsPopMatrix();
		}

		shaderInstanceSetBool(portalProgram.vertexShader, 0, false);
		GPUCMD_AddWrite(GPUREG_VSH_BOOLUNIFORM, 0x7FFF0000|portalProgram.vertexShader->boolUniforms);

		GPU_SetDepthTestAndWriteMask(true, GPU_ALWAYS, GPU_WRITE_DEPTH);
		GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP);

		for(i=0; i<n; i++)
		{
			portal_s* p = portals[i];
			
			if(!p->draw)continue;
			if(!p->target || !p->target->open)continue;

			GPU_SetStencilTest(true, GPU_EQUAL, stencilValue(i, stencil), 0xFF, 0x00);

			gsPushMatrix();
				gsTranslate(p->position.x, p->position.y, p->position.z);
				gsMultMatrix(p->matrix);

				gsUpdateTransformation();

				GPU_DrawArray(GPU_TRIANGLE_STRIP, 0, portalNumVertices);
			gsPopMatrix();
		}

	gsPopMatrix();

	GPU_SetDepthTestAndWriteMask(true, GPU_GREATER, GPU_WRITE_ALL);
	GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP);

	for(i=0; i<n; i++)
	{
		portal_s* p = portals[i];

		if(!p->draw)continue;
		if(!p->target || !p->target->open)continue;
		
		GPU_SetStencilTest(true, GPU_EQUAL, stencilValue(i, stencil), 0xFF, 0x00);

		float near;
		vect3Di_s bottomright, topleft;
		getPortalBoundingBox(p, c, &topleft, &bottomright, &near);
		
		//TODO : improve
		if(bottomright.x-topleft.x <= 0 || bottomright.y-topleft.y <= 0)continue;

		GPU_SetScissorTest_(GPU_SCISSOR_NORMAL, topleft.x, topleft.y, bottomright.x, bottomright.y);

		// if(!i && depth==2)printf("%f\n",near);

		gsPushMatrix();
			camera_s camera=*c;
			initProjectionMatrix((float*)camera.projection, 1.3962634f, 240.0f/400.0f, near, 1000.0f);
			float tmp1[4*4], tmp2[4*4];
			transposeMatrix44(p->target->matrix, tmp1);
			camera.position = vaddf(p->target->position, warpPortalVector(p, vsubf(c->position, p->position)));
			multMatrix44((float*)camera.orientation, p->matrix, tmp2);
			rotateMatrixY(tmp1, M_PI, true);
			multMatrix44(tmp2, tmp1, (float*)camera.orientation);

			memcpy(camera.modelview, camera.orientation, sizeof(mtx44));
			translateMatrix((float*)camera.modelview, -camera.position.x, -camera.position.y, -camera.position.z);

			callback(&camera, p, depth-1, stencilValue(i, stencil));
		gsPopMatrix();
	}
}

const u8 segmentList[4][2]={{0,1},{1,2},{2,3},{3,0}};
const u8 segmentNormal[4]={0,1,0,1};

bool doSegmentsIntersect(vect3Df_s s1, vect3Df_s s2, vect3Df_s sn, vect3Df_s p1, vect3Df_s p2, vect3Df_s pn, bool *vv1, bool *vv2)
{
	bool v1=vdotf(vsubf(p1,s1),pn)>0;
	bool v2=vdotf(vsubf(p1,s2),pn)>0;
	bool v3=vdotf(vsubf(s1,p1),sn)>0;
	bool v4=vdotf(vsubf(s1,p2),sn)>0;
	*vv1=v1;*vv2=v2;
	return (v1!=v2)&&(v3!=v4);
}

bool portalRectangleIntersection(room_s* r, portal_s* p, rectangle_s* rec, vect3Df_s origpos, bool fix)
{
	if(!p || !rec)return false;
	if((rec->normal.x&&p->normal.x)||(rec->normal.z&&p->normal.z)||(rec->normal.y&&p->normal.y))return false;
	vect3Df_s pr=convertRectangleVector(rec->position);
	vect3Df_s s=convertRectangleVector(rec->size);

	//only need to do this once for all rectangles (so optimize it out)
	const vect3Df_s v[]={(vmulf(p->plane[0],PORTAL_WIDTH)), (vmulf(p->plane[1],PORTAL_HEIGHT))};
	const vect3Df_s points[]={vaddf(vaddf(p->position,v[0]),v[1]),vsubf(vaddf(p->position,v[0]),v[1]),vsubf(vsubf(p->position,v[0]),v[1]),vaddf(vsubf(p->position,v[0]),v[1])};
	
	//projection = more elegant, less efficient ? (rectangles are axis aligned biatch)
	if(p->normal.x)
	{
		if(!((pr.x-PORTALMARGIN<=p->position.x&&pr.x+s.x+PORTALMARGIN>=p->position.x)||(pr.x+PORTALMARGIN>=p->position.x&&pr.x+s.x-PORTALMARGIN<=p->position.x)))return false;
	}else if(p->normal.y)
	{
		if(!((pr.y-PORTALMARGIN<=p->position.y&&pr.y+s.y+PORTALMARGIN>=p->position.y)||(pr.y+PORTALMARGIN>=p->position.y&&pr.y+s.y-PORTALMARGIN<=p->position.y)))return false;
	}else{
		if(!((pr.z-PORTALMARGIN<=p->position.z&&pr.z+s.z+PORTALMARGIN>=p->position.z)||(pr.z+PORTALMARGIN>=p->position.z&&pr.z+s.z-PORTALMARGIN<=p->position.z)))return false;
	}
	
	vect3Df_s p1=pr, p2=vaddf(pr,s);
	vect3Df_s pn=rec->normal;
	
	int i;
	for(i=0;i<4;i++)
	{
		const vect3Df_s s1=points[segmentList[i][0]], s2=points[segmentList[i][1]]; //segment
		bool v1, v2;
		if(doSegmentsIntersect(s1,s2,v[segmentNormal[i]],p1,p2,pn,&v1,&v2))
		{
			if(fix)
			{
				bool v3=vdotf(vsubf(p1,origpos),pn)>0;
				if(v3==v1)
				{
					float ln=vdotf(pn,vsubf(p1,s2));
					ln+=(ln>0)?(PORTALMARGIN):(-PORTALMARGIN);
					p->position=vaddf(p->position,vmulf(pn,ln));
				}else{
					float ln=vdotf(pn,vsubf(p1,s1));
					ln+=(ln>0)?(PORTALMARGIN):(-PORTALMARGIN);
					p->position=vaddf(p->position,vmulf(pn,ln));
				}
			//if put back, add updating of points
			// }else{
				// return true;
			}
			return true;
		}
	}
	p1=vsubf(p1,p->position);
	p1=vect3Df(vdotf(p1,p->plane[0]),vdotf(p1,p->plane[1]),0);
	if(p1.x<PORTAL_WIDTH||p1.x>PORTAL_WIDTH||p1.y<PORTAL_HEIGHT||p1.y>PORTAL_HEIGHT)return false;
	p2=vsubf(p2,p->position);
	p2=vect3Df(vdotf(p2,p->plane[0]),vdotf(p2,p->plane[1]),0);
	if(p2.x<PORTAL_WIDTH||p2.x>PORTAL_WIDTH||p2.y<PORTAL_HEIGHT||p2.y>PORTAL_HEIGHT)return false;
	
	return true;
}

bool isPortalOnWall(room_s* r, portal_s* p, bool fix)
{
	listCell_s *lc=r->rectangles.first;
	vect3Df_s origpos=p->position;
	while(lc)
	{
		if(portalRectangleIntersection(r,p,&lc->data,origpos,fix)&&!fix)return false;
		lc=lc->next;
	}
	return true;
}
