#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <3ds.h>

#include "gfx/gs.h"
#include "gfx/text.h"
#include "gfx/md2.h"
#include "gfx/texture.h"
#include "utils/math.h"
#include "utils/filesystem.h"

#include "game/camera.h"
#include "game/material.h"
#include "game/room_io.h"
#include "game/portal.h"

char* testString;
md2_instance_t gladosInstance;
md2_model_t gladosModel;
texture_s gladosTexture;
room_s testRoom;
camera_s testCamera;

//object position and rotation angle
vect3Df_s position, angle;

portal_s testPortal1, testPortal2;

void drawBottom(u32* outBuffer, u32* outDepthBuffer)
{

}

//stolen from staplebutt
void GPU_SetDummyTexEnv(u8 num)
{
	GPU_SetTexEnv(num, 
		GPU_TEVSOURCES(GPU_PREVIOUS, 0, 0), 
		GPU_TEVSOURCES(GPU_PREVIOUS, 0, 0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_REPLACE, 
		GPU_REPLACE, 
		0xFFFFFFFF);
}

void drawScene(camera_s* c)
{
	if(!c)return;

	gsMatrixMode(GS_MODELVIEW);
	gsPushMatrix();
		gsLoadIdentity();
		useCamera(c);

		md2StartDrawing();
		md2InstanceDraw(&gladosInstance);

		drawRoom(&testRoom);
	gsPopMatrix();
}

// topscreen
void renderFrame(u32* outBuffer, u32* outDepthBuffer)
{
	GPU_SetViewport((u32*)osConvertVirtToPhys((u32)outDepthBuffer),(u32*)osConvertVirtToPhys((u32)outBuffer),0,0,240*2,400);
	
	GPU_DepthRange(-1.0f, 0.0f);
	GPU_SetFaceCulling(GPU_CULL_FRONT_CCW);
	GPU_SetStencilTest(false, GPU_ALWAYS, 0x00, 0xFF, 0x00);
	GPU_SetStencilOp(GPU_KEEP, GPU_KEEP, GPU_KEEP);
	GPU_SetBlendingColor(0,0,0,0);
	GPU_SetDepthTestAndWriteMask(true, GPU_GREATER, GPU_WRITE_ALL);
	
	GPUCMD_AddMaskedWrite(GPUREG_0062, 0x1, 0); 
	GPUCMD_AddWrite(GPUREG_0118, 0);
	
	GPU_SetAlphaBlending(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
	GPU_SetAlphaTest(false, GPU_ALWAYS, 0x00);
	
	GPU_SetTextureEnable(GPU_TEXUNIT0);
	
	GPU_SetTexEnv(0, 
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR), 
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
		GPU_TEVOPERANDS(0,0,0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_MODULATE, GPU_MODULATE, 
		0xFFFFFFFF);
	GPU_SetDummyTexEnv(1);
	GPU_SetDummyTexEnv(2);
	GPU_SetDummyTexEnv(3);
	GPU_SetDummyTexEnv(4);
	GPU_SetDummyTexEnv(5);

	// textStartDrawing();
	// textDrawString(0, 0, testString);

	// //draw object
		gsMatrixMode(GS_MODELVIEW);
		gsPushMatrix();

			useCamera(&testCamera);

			drawScene(&testCamera);

			drawPortals((portal_s*[]){&testPortal1, &testPortal2}, 2, drawScene, &testCamera);

		gsPopMatrix();

	GPU_FinishDrawing();
}

int main(int argc, char** argv)
{
	//setup services
	gfxInit();

	consoleInit(GFX_BOTTOM, NULL);

	//let GFX know we're ok with doing stereoscopic 3D rendering
	gfxSet3D(true);

	//initialize GS
	gsInit(NULL, renderFrame, drawBottom);

	//init fs
	filesystemInit(argc, argv);

	//init materials
	initMaterials();
	loadMaterialSlices("slices.ini");
	loadMaterials("materials.ini");

	//init text
	textInit();
	testString = textMakeString("\1hello this is a test\nwith newline support");

	//init md2
	md2Init();
	textureLoad(&gladosTexture, "glados.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)|GPU_TEXTURE_WRAP_S(GPU_REPEAT)|GPU_TEXTURE_WRAP_T(GPU_REPEAT));
	md2ReadModel(&gladosModel, "glados.md2");
	md2InstanceInit(&gladosInstance, &gladosModel, &gladosTexture);
	md2InstanceChangeAnimation(&gladosInstance, 1, false);

	//init camera
	initCamera(&testCamera);

	//init room
	roomInit();
	readRoom("test1.map", &testRoom, MAP_READ_ENTITIES);

	//init portal
	portalInit();
	initPortal(&testPortal1);
	initPortal(&testPortal2);

	updatePortalOrientation(&testPortal2, vect3Df(1.0f, 0.0f, 0.0f), vect3Df(0.0f, 1.0f, 0.0f));
	testPortal2.position.y-=4.0f;

	testPortal1.target = &testPortal2;
	testPortal2.target = &testPortal1;

	//background color (blue)
	gsSetBackgroundColor(RGBA8(0x68, 0xB0, 0xD8, 0xFF));

	rotateCamera(&testCamera, vect3Df(0.0f, M_PI, 0.0f));

	printf("ready\n");

	while(aptMainLoop())
	{
		//controls
		hidScanInput();
		//START to exit to hbmenu
		if(keysDown()&KEY_START)break;

		//rotate object
		if(keysHeld()&KEY_CSTICK_UP)rotateCamera(&testCamera, vect3Df(-0.05f, 0.0f, 0.0f));
		if(keysHeld()&KEY_CSTICK_DOWN)rotateCamera(&testCamera, vect3Df(0.05f, 0.0f, 0.0f));
		if(keysHeld()&KEY_CSTICK_LEFT)rotateCamera(&testCamera, vect3Df(0.0f, -0.05f, 0.0f));
		if(keysHeld()&KEY_CSTICK_RIGHT)rotateCamera(&testCamera, vect3Df(0.0f, 0.05f, 0.0f));

		if(keysHeld()&KEY_CPAD_UP)moveCamera(&testCamera, vect3Df(0.0f, 0.0f, -0.4f));
		if(keysHeld()&KEY_CPAD_DOWN)moveCamera(&testCamera, vect3Df(0.0f, 0.0f, 0.4f));
		if(keysHeld()&KEY_CPAD_LEFT)moveCamera(&testCamera, vect3Df(-0.4f, 0.0f, 0.0f));
		if(keysHeld()&KEY_CPAD_RIGHT)moveCamera(&testCamera, vect3Df(0.4f, 0.0f, 0.0f));


		if(keysHeld()&KEY_X)testPortal1.position = vaddf(testPortal1.position, vect3Df(0.0f, 0.0f, -0.4f));
		if(keysHeld()&KEY_B)testPortal1.position = vaddf(testPortal1.position, vect3Df(0.0f, 0.0f, 0.4f));
		if(keysHeld()&KEY_Y)testPortal1.position = vaddf(testPortal1.position, vect3Df(-0.4f, 0.0f, 0.0f));
		if(keysHeld()&KEY_A)testPortal1.position = vaddf(testPortal1.position, vect3Df(0.4f, 0.0f, 0.0f));

		if(keysHeld()&KEY_R || keysHeld()&KEY_L)
		{
			vect3Df_s position;
			rectangle_s* rec = collideLineMapClosest(&testRoom, NULL, testCamera.position, vect3Df(-testCamera.orientation[2][0], -testCamera.orientation[2][1], -testCamera.orientation[2][2]), 1000.0f, &position, NULL);
			if(rec)
			{
				printf("%p\n", rec);
				vect3Df_s normal = rec->normal;
				vect3Df_s plane0 = vect3Df(testCamera.orientation[0][0], testCamera.orientation[0][1], testCamera.orientation[0][2]);
				plane0 = vnormf(vsubf(plane0, vmulf(normal, vdotf(normal, plane0))));

				if(keysHeld()&KEY_L)
				{
					testPortal2.position = position;
					updatePortalOrientation(&testPortal2, plane0, normal);
				}else
				{
					testPortal1.position = position;
					updatePortalOrientation(&testPortal1, plane0, normal);
				}
			}
		}

		md2InstanceUpdate(&gladosInstance);
		updateCamera(&testCamera);

		gsDrawFrame();

		gspWaitForEvent(GSPEVENT_VBlank0, true);
	}

	gsExit();
	gfxExit();
	return 0;
}
