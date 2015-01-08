#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/portal.h"
#include "utils/math.h"
#include "gfx/gs.h"

#include "portal_vsh_shbin.h"

#define PORTAL_HEIGHT (4.0f)
#define PORTAL_WIDTH (2.0f)
#define PORTAL_OUTLINE (0.1f)
#define PORTAL_DETAIL (32)

DVLB_s* portalDvlb;
shaderProgram_s portalProgram;
const u32 portalBaseAddr=0x14000000;

typedef struct
{
	vect3Df_s position;
}portalVertex_s;

portalVertex_s* portalVertexData;
int portalNumVertices;
portalVertex_s* portalOutlineVertexData;
int portalOutlineNumVertices;

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
}

void portalExit()
{

}

void initPortal(portal_s* p)
{
	if(!p)return;

	// p->position = vect3Df();
	p->target = NULL;
	updatePortalOrientation(p, vect3Df(0.0f, 0.0f, 1.0f), vect3Df(-1.0f, 0.0f, 0.0f));
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

void drawPortals(portal_s* portals[], int n, renderSceneCallback callback, camera_s* c)
{
	if(!portals || !portalVertexData || !callback || !c)return;
	int i;

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

		GPUCMD_AddWrite(GPUREG_ATTRIBBUFFER0_CONFIG0, (u32)portalVertexData-portalBaseAddr);

		GPU_SetDepthTestAndWriteMask(true, GPU_GEQUAL, GPU_WRITE_COLOR);
		GPU_SetStencilOp(GPU_KEEP, GPU_KEEP, GPU_XOR);

		for(i=0; i<n; i++)
		{
			portal_s* p = portals[i];
			GPU_SetStencilTest(true, GPU_ALWAYS, 0x00, 0xFF, i+1);

			gsPushMatrix();
				gsTranslate(p->position.x, p->position.y, p->position.z);
				gsMultMatrix(p->matrix);

				gsUpdateTransformation();

				GPU_DrawArray(GPU_TRIANGLE_STRIP, portalNumVertices);
			gsPopMatrix();
		}

		shaderInstanceSetBool(portalProgram.vertexShader, 0, false);
		GPUCMD_AddWrite(GPUREG_VSH_BOOLUNIFORM, 0x7FFF0000|portalProgram.vertexShader->boolUniforms);

		GPU_SetDepthTestAndWriteMask(true, GPU_ALWAYS, GPU_WRITE_ALL);
		GPU_SetStencilOp(GPU_KEEP, GPU_KEEP, GPU_KEEP);

		for(i=0; i<n; i++)
		{
			portal_s* p = portals[i];
			GPU_SetStencilTest(true, GPU_NOTEQUAL, 0x00, 0xFF, i+1);

			gsPushMatrix();
				gsTranslate(p->position.x, p->position.y, p->position.z);
				gsMultMatrix(p->matrix);

				gsUpdateTransformation();

				GPU_DrawArray(GPU_TRIANGLE_STRIP, portalNumVertices);
			gsPopMatrix();
		}

	gsPopMatrix();

	GPU_SetDepthTestAndWriteMask(true, GPU_GREATER, GPU_WRITE_ALL);
	GPU_SetStencilOp(GPU_KEEP, GPU_KEEP, GPU_KEEP);

	for(i=0; i<n; i++)
	{
		portal_s* p = portals[i];
		
		GPU_SetStencilTest(true, GPU_NOTEQUAL, 0x00, 0xFF, i+1);

		gsPushMatrix();
			camera_s camera=*c;
			float tmp1[4*4], tmp2[4*4];
			transposeMatrix44(p->target->matrix, tmp1);
			camera.position = vaddf(p->target->position, warpPortalVector(p, vsubf(c->position, p->position)));
			multMatrix44((float*)camera.orientation, p->matrix, tmp2);
			rotateMatrixY(tmp1, M_PI, true);
			multMatrix44(tmp2, tmp1, (float*)camera.orientation);

			memcpy(camera.modelview, camera.orientation, sizeof(mtx44));
			translateMatrix((float*)camera.modelview, -camera.position.x, -camera.position.y, -camera.position.z);

			callback(&camera);
		gsPopMatrix();
	}
}
