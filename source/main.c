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

#include "game/sfx.h"
#include "game/controls.h"
#include "game/camera.h"
#include "game/material.h"
#include "game/room_io.h"
#include "game/portal.h"
#include "game/player.h"
#include "game/cubes.h"
#include "game/energyball.h"
#include "game/platform.h"
#include "game/bigbutton.h"
#include "game/timedbutton.h"
#include "game/emancipation.h"
#include "game/sludge.h"
#include "game/walldoor.h"
#include "game/door.h"

#include "physics/physics.h"

#define RGB10(r, g, b) ((((r)&0x3FF)<<20)|(((g)&0x3FF)<<10)|(((b)&0x3FF)))

float debugVal[10];

extern md2_model_t cubeModel, cubeDispenserModel;
extern texture_s storageCubeTexture, companionCubeTexture, cubeDispenserTexture;

room_s testRoom;
player_s testPlayer;

//object position and rotation angle
vect3Df_s position, angle;

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

void drawScene(camera_s* c, portal_s* p, int depth, u8 stencil)
{
	if(!c)return;

	gsMatrixMode(GS_MODELVIEW);
	gsPushMatrix();
		gsLoadIdentity();
		useCamera(c);

		drawRoom(&testRoom);

		drawCubeDispensers();
		drawEnergyDevices();
		drawEnergyBalls();
		drawPlatforms();
		drawBigButtons();
		drawTimedButtons();
		drawWallDoors();
		drawDoors();
		drawOBBs();

		if(p && depth)drawPlayer(&testPlayer);
		
		drawSludge(&testRoom);

		drawEmancipators();
		drawEmancipationGrids();

		if(p == &portals[0])drawPortals((portal_s*[]){&portals[0]}, 1, drawScene, c, depth, stencil);
		else if(p == &portals[1])drawPortals((portal_s*[]){&portals[1]}, 1, drawScene, c, depth, stencil);
		else drawPortals((portal_s*[]){&portals[0], &portals[1]}, 2, drawScene, c, depth, stencil);
	gsPopMatrix();
}

// topscreen
void renderFrame(u32* outBuffer, u32* outDepthBuffer)
{
	GPU_SetViewport((u32*)osConvertVirtToPhys((u32)outDepthBuffer),(u32*)osConvertVirtToPhys((u32)outBuffer),0,0,240,400);
	
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
		// GPU_TEVSOURCES(0x1, 0x1, 0x1),
		// GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
		GPU_TEVOPERANDS(0,0,0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_MODULATE, GPU_MODULATE, 
		0xFFFFFFFF);
	GPU_SetDummyTexEnv(1);
	GPU_SetDummyTexEnv(2);
	GPU_SetDummyTexEnv(3);
	GPU_SetDummyTexEnv(4);
	GPU_SetDummyTexEnv(5);

	// // GPUCMD_AddMaskedWrite(GPUREG_01C6, 0x1, 0x00000001);
	// GPUCMD_AddMaskedWrite(GPUREG_01C6, 0x1, 0x00000000);
	// GPUCMD_AddMaskedWrite(GPUREG_008F, 0x1, 0x00000001);
	// // GPUCMD_AddSingleParam(0x000100E0, 0x00000005);
	// GPUCMD_AddSingleParam(0x000100E0, 0x00000000);
	// GPUCMD_AddMaskedWrite(GPUREG_006D, 0x1, 0x00000001);

	// static u32 param[0x100];

	// param[0x0]=RGB10(0xFF, 0xFF, 0xFF);
	// param[0x1]=0x00000000;
	// param[0x2]=RGB10(0xFF, 0xFF, 0xFF);
	// param[0x3]=RGB10(0x20, 0x20, 0x20);
	// GPUCMD_Add(0x800F0140, param, 0x00000004);

	// // param[0x0]=0x7A0E8000;
	// // param[0x1]=0x0000EFC5;
	// ((__fp16*)param)[0]=(__fp16)debugVal[0];
	// ((__fp16*)param)[1]=(__fp16)debugVal[1];
	// ((__fp16*)param)[2]=(__fp16)debugVal[2];
	// GPUCMD_Add(0x800F0144, param, 0x00000002);

	// // GPUCMD_AddSingleParam(0x00010149, 0x00000000);
	// // GPUCMD_AddSingleParam(0x00010149, 0x00000001); // light type (directional ?)
	// // GPUCMD_AddSingleParam(0x00010149, 0x00000002); // light type (point ?)
	// // GPUCMD_AddSingleParam(0x00010149, 0x00000003); // light type (point ?)
	// GPUCMD_AddSingleParam(0x00010149, 0x00000004); // light type

	// // param[0x0]=0x000C3400;
	// // param[0x1]=0x00033A36;
	// param[0x0]=f32tof24(debugVal[3]);
	// param[0x1]=f32tof24(debugVal[4]);
	// GPUCMD_Add(0x8007014A, param, 0x00000002);

	// GPUCMD_AddSingleParam(0x000F01C0, RGB10(0x00, 0x00, 0x00)); //global ambient

	// GPUCMD_AddSingleParam(0x000101C2, 0x00000000); //num lights - 1
	// GPUCMD_AddSingleParam(0x000F01D9, 0x00000000); //permutation

	// // GPUCMD_AddSingleParam(0x000101C2, 0x00000003);
	// // GPUCMD_AddSingleParam(0x000F01D9, 0x00003210);

	// // GPUCMD_AddSingleParam(0x000F01C3, 0x80000400); //frag func0 ?
	// // GPUCMD_AddSingleParam(0x000F01C3, 0x90000400);
	// GPUCMD_AddSingleParam(0x000F01C3, 0x10000400);

	// // GPUCMD_AddSingleParam(0x000B01C4, 0xFF00FFFF); //frag func1 ?
	// GPUCMD_AddSingleParam(0x000B01C4, 0xF600FFFF);

	// // GPUCMD_AddSingleParam(0x000401C4, 0x007F0000); //frag func1 ?
	// GPUCMD_AddSingleParam(0x000401C4, 0x007E0000);

	// param[0x0]=0x00000000;
	// param[0x1]=0x00000000;
	// param[0x2]=0x00000000;
	// GPUCMD_Add(0x800F01D0, param, 0x00000003); //frag LUT selection ?

	// param[0x0]=0x00BF0000;
	// param[0x1]=0x00AF8000;
	// GPUCMD_Add(0x8007004D, param, 0x00000002); //frag scaling factors ? (float24)

	// //draw object
		gsMatrixMode(GS_MODELVIEW);
		drawScene(&testPlayer.camera, NULL, 2, 0);
		GPU_SetStencilTest(false, GPU_ALWAYS, 0x00, 0xFF, 0x00);
		GPU_SetScissorTest_(GPU_SCISSOR_NORMAL, 0, 0, 240, 400);
		drawPlayerGun(&testPlayer);

	GPU_FinishDrawing();
}

char* levelNames[] = {"out1.map", "out2.map", "out3.map", "out4.map", "out5.map", "out6.map", "out7.map", "out8.map"};
int currentLevel;

void gameInit()
{
	//initialize GS
	gsInit(NULL, renderFrame, drawBottom);

	//init materials
	initMaterials();
	loadMaterialSlices("slices.ini");
	loadMaterials("materials.ini");

	//init text
	textInit();

	//init sfx
	initSound();

	//init md2
	md2Init();

	//init player
	playerInit();
	initPlayer(&testPlayer);
	initControls();

	//init physics
	initPhysics();

	//init game elements
	initSludge();
	initCubes();
	initEnergyBalls();
	initPlatforms();
	initBigButtons();
	initTimedButtons();
	initEmancipation();
	initElevators();
	initWallDoors();
	initDoors();

	//init room
	roomInit();
	// readRoom("test1.map", &testRoom, MAP_READ_ENTITIES);
	readRoom(levelNames[currentLevel], &testRoom, MAP_READ_ENTITIES|MAP_READ_LIGHT);
	generateSludgeGeometry();

	//init portal
	portalInit();
	portals[0].target = &portals[1];
	portals[1].target = &portals[0];
	portals[0].color = vect3Df(1.0f, 0.7f, 0.0f);
	portals[1].color = vect3Df(0.0f, 0.7f, 1.0f);

	//init physics
	transferRoomRectangles(&testRoom);
	physicsGenerateGrid();

	//background color (blue)
	gsSetBackgroundColor(RGBA8(0x68, 0xB0, 0xD8, 0xFF));

	rotatePlayer(&testPlayer, vect3Df(0.0f, M_PI, 0.0f));

	printf("ready\n");
}

void gameExit()
{
	portalExit();
	roomExit();

	freeRoom(&testRoom);

	exitDoors();
	exitWallDoors();
	exitElevators();
	exitEmancipation();
	exitTimedButtons();
	exitBigButtons();
	exitPlatforms();
	exitEnergyBalls();
	exitCubes();
	exitSludge();

	textureExit();

	exitPhysics();

	playerExit();
	md2Exit();
	textExit();

	exitSound();

	gsExit();
}

bool gameFrame()
{
	//controls
	hidScanInput();
	//START to exit to hbmenu
	if(keysDown()&KEY_START)return true;
	if(testPlayer.life < 0)return true;

	// if(keysHeld()&KEY_X)debugVal[0]+=0.05f*10;
	// if(keysHeld()&KEY_B)debugVal[0]-=0.05f*10;
	// if(keysHeld()&KEY_Y)debugVal[1]+=0.05f*10;
	// if(keysHeld()&KEY_A)debugVal[1]-=0.05f*10;

	if(keysHeld()&KEY_DUP)debugVal[2]+=0.05f*10;
	if(keysHeld()&KEY_DDOWN)debugVal[2]-=0.05f*10;
	if(keysHeld()&KEY_DLEFT)debugVal[3]+=0.05f;
	if(keysHeld()&KEY_DRIGHT)debugVal[3]-=0.05f;
	// if(keysHeld()&KEY_ZL)debugVal[4]+=0.05f;
	// if(keysHeld()&KEY_ZR)debugVal[4]-=0.05f;
	if(keysDown()&KEY_ZR && keysDown()&KEY_ZL)testPlayer.flying^=1;

	// printf("%4.2f %4.2f %4.2f %4.2f %4.2f\n",debugVal[0],debugVal[1],debugVal[2],debugVal[3],debugVal[4]);

	if(keysDown()&KEY_R){shootPlayerGun(&testPlayer, &testRoom, &portals[0]);playSFX(gunSFX1);testPlayer.gunInstance.texture=&gunTextureOrange;}
	if(keysDown()&KEY_L){shootPlayerGun(&testPlayer, &testRoom, &portals[1]);playSFX(gunSFX2);testPlayer.gunInstance.texture=&gunTextureBlue;}

	updateControls(&testPlayer);

	updatePlayer(&testPlayer, &testRoom);
	updateSludge(&testPlayer);

	updateCubeDispensers();
	updateEnergyDevices();
	updateEnergyBalls(&testRoom);
	updatePlatforms(&testPlayer);
	updateBigButtons();
	updateTimedButtons();
	updateWallDoors(&testPlayer);
	updateDoors();
	updateEmancipators();
	updateEmancipationGrids(&testPlayer);

	gsDrawFrame();

	gspWaitForEvent(GSPEVENT_VBlank0, true);

	return false;
}

size_t getMemUsed();
size_t getMemFree();
extern int totaltextures;

int main(int argc, char** argv)
{
	//setup services
	gfxInit();

	consoleInit(GFX_BOTTOM, NULL);
	//let GFX know we're ok with doing stereoscopic 3D rendering
	gfxSet3D(true);

	//init fs
	filesystemInit(argc, argv);

	//menu
	{
		menuInit();

		bool done = false;
		while(aptMainLoop() && !done)
		{
			done = menuFrame();
		}

		menuExit();
	}

	// currentLevel = 0;
	// currentLevel = 4;

	bool again = true;
	// bool again = false;
	while(again)
	{
		gameInit();

		bool done = false;
		while(aptMainLoop() && !done)
		{
			done = gameFrame();
		}

		gameExit();

		hidScanInput();
		hidScanInput();
		hidScanInput();
		hidScanInput();
		printf("heap : %08X\n",getMemFree());
		printf("linear heap : %08X\n",linearSpaceFree());
		printf("textures : %d\n",totaltextures);
		done = false;
		again = false;
		while(!done)
		{
			hidScanInput();
			if(keysDown()&KEY_START)
			{
				done = true;
				again = true;
			}
			if(keysDown()&KEY_SELECT)
			{
				done = true;
			}
		}
		currentLevel %= 8;
	}

	gfxExit();
	return 0;
}
