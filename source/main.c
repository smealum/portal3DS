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
#include "game/player.h"

char* testString;
md2_instance_t gladosInstance;
md2_model_t gladosModel;
texture_s gladosTexture;
room_s testRoom;
player_s testPlayer;

//object position and rotation angle
vect3Df_s position, angle;

portal_s testPortal1, testPortal2;
portal_s testPortal3, testPortal4;

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

void drawScene(camera_s* c, int depth, u8 stencil)
{
	if(!c)return;

	gsMatrixMode(GS_MODELVIEW);
	gsPushMatrix();
		gsLoadIdentity();
		useCamera(c);

		md2StartDrawing();
		md2InstanceDraw(&gladosInstance);

		drawRoom(&testRoom);

		drawPortals((portal_s*[]){&testPortal1, &testPortal2}, 2, drawScene, c, depth, stencil);
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
		drawPlayerGun(&testPlayer);
		drawScene(&testPlayer.camera, 2, 0);

	GPU_FinishDrawing();
}

float debugVal[10] = {1.3, -1.65, -3.1, -0.1};

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
	textureLoad(&gladosTexture, "glados.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)|GPU_TEXTURE_WRAP_S(GPU_REPEAT)|GPU_TEXTURE_WRAP_T(GPU_REPEAT), 0);
	md2ReadModel(&gladosModel, "glados.md2");
	md2InstanceInit(&gladosInstance, &gladosModel, &gladosTexture);
	md2InstanceChangeAnimation(&gladosInstance, 1, false);

	//init player
	playerInit();
	initPlayer(&testPlayer);

	//init room
	roomInit();
	readRoom("test1.map", &testRoom, MAP_READ_ENTITIES);

	//init portal
	portalInit();
	initPortal(&testPortal1);
	initPortal(&testPortal2);
	initPortal(&testPortal3);
	initPortal(&testPortal4);

	updatePortalOrientation(&testPortal2, vect3Df(1.0f, 0.0f, 0.0f), vect3Df(0.0f, 1.0f, 0.0f));
	testPortal2.position.y-=4.0f;

	testPortal1.target = &testPortal2;
	testPortal2.target = &testPortal1;

	testPortal3.target = &testPortal4;
	testPortal4.target = &testPortal3;

	//background color (blue)
	gsSetBackgroundColor(RGBA8(0x68, 0xB0, 0xD8, 0xFF));

	rotatePlayer(&testPlayer, vect3Df(0.0f, M_PI, 0.0f));

	printf("ready\n");

	while(aptMainLoop())
	{
		//controls
		hidScanInput();
		//START to exit to hbmenu
		if(keysDown()&KEY_START)break;

		circlePosition cstick;
		irrstCstickRead(&cstick);
		rotatePlayer(&testPlayer, vect3Df((abs(cstick.dy)<5)?0:(-cstick.dy*0.001f), (abs(cstick.dx)<5)?0:(cstick.dx*0.001f), 0.0f));

		if(keysHeld()&KEY_CPAD_UP)movePlayer(&testPlayer, vect3Df(0.0f, 0.0f, -0.4f));
		if(keysHeld()&KEY_CPAD_DOWN)movePlayer(&testPlayer, vect3Df(0.0f, 0.0f, 0.4f));
		if(keysHeld()&KEY_CPAD_LEFT)movePlayer(&testPlayer, vect3Df(-0.4f, 0.0f, 0.0f));
		if(keysHeld()&KEY_CPAD_RIGHT)movePlayer(&testPlayer, vect3Df(0.4f, 0.0f, 0.0f));

		if(keysHeld()&KEY_X)debugVal[0]+=0.05f;
		if(keysHeld()&KEY_B)debugVal[0]-=0.05f;
		if(keysHeld()&KEY_Y)debugVal[1]+=0.05f;
		if(keysHeld()&KEY_A)debugVal[1]-=0.05f;

		if(keysHeld()&KEY_DUP)debugVal[2]+=0.05f;
		if(keysHeld()&KEY_DDOWN)debugVal[2]-=0.05f;
		if(keysHeld()&KEY_DLEFT)debugVal[3]+=0.05f;
		if(keysHeld()&KEY_DRIGHT)debugVal[3]-=0.05f;
		if(keysHeld()&KEY_ZL)debugVal[4]+=0.05f;
		if(keysHeld()&KEY_ZR)debugVal[4]-=0.05f;

		printf("%f %f %f %f\n",debugVal[0],debugVal[1],debugVal[2],debugVal[3]);

		if(keysDown()&KEY_R)shootPlayerGun(&testPlayer, &testRoom, &testPortal1);
		if(keysDown()&KEY_L)shootPlayerGun(&testPlayer, &testRoom, &testPortal2);

		md2InstanceUpdate(&gladosInstance);
		updatePlayer(&testPlayer, &testRoom);

		gsDrawFrame();

		gspWaitForEvent(GSPEVENT_VBlank0, true);
	}

	gsExit();
	gfxExit();
	return 0;
}
