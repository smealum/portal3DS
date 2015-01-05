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

#include "game/material.h"
#include "game/room_io.h"

char* testString;
md2_instance_t gladosInstance;
md2_model_t gladosModel;
texture_s gladosTexture;
room_s testRoom;

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

	textStartDrawing();
	textDrawString(0, 0, testString);

	//initialize projection matrix to standard perspective stuff
		gsMatrixMode(GS_PROJECTION);
		// gsProjectionMatrix(80.0f*M_PI/180.0f, 240.0f/400.0f, 0.01f, 200.0f);
		gsProjectionMatrix(80.0f*M_PI/180.0f, 240.0f/400.0f, 0.01f, 10000.0f);
		gsRotateZ(M_PI/2); //because framebuffer is sideways...

	//draw object
		gsMatrixMode(GS_MODELVIEW);
		gsPushMatrix();
			gsRotateX(angle.x);
			gsRotateY(angle.y);
			gsRotateZ(angle.z);
			gsTranslate(-position.x, -position.y, -position.z);

			md2StartDrawing();
			md2InstanceDraw(&gladosInstance);

			gsScale(TILESIZE_FLOAT*2, TILESIZE_FLOAT, TILESIZE_FLOAT*2);
			drawRoom(&testRoom);

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

	//init materials
	initMaterials();

	//init text
	textInit();
	testString = textMakeString("\1hello this is a test\nwith newline support");

	//init md2
	md2Init();
	textureLoad(&gladosTexture, "sdmc:/glados.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)|GPU_TEXTURE_WRAP_S(GPU_REPEAT)|GPU_TEXTURE_WRAP_T(GPU_REPEAT));
	md2ReadModel(&gladosModel, "sdmc:/glados.md2");
	md2InstanceInit(&gladosInstance, &gladosModel, &gladosTexture);
	md2InstanceChangeAnimation(&gladosInstance, 1, false);

	//init room
	roomInit();
	readRoom("sdmc:/test1.map", &testRoom, MAP_READ_ENTITIES);

	//initialize object position and angle
	// position=vect3Df(0.0f, -15.0f, -40.0f);
	angle=vect3Df(M_PI/2, 0.0f, M_PI/2);
	angle=vect3Df(0.0f, 0.0f, 0.0f);

	//background color (blue)
	gsSetBackgroundColor(RGBA8(0x68, 0xB0, 0xD8, 0xFF));

	printf("ready\n");

	while(aptMainLoop())
	{
		//controls
		hidScanInput();
		//START to exit to hbmenu
		if(keysDown()&KEY_START)break;

		//rotate object
		if(keysHeld()&KEY_CPAD_UP)angle.x+=0.025f;
		if(keysHeld()&KEY_CPAD_DOWN)angle.x-=0.025f;
		if(keysHeld()&KEY_CPAD_LEFT)angle.z+=0.025f;
		if(keysHeld()&KEY_CPAD_RIGHT)angle.z-=0.025f;

		if(keysHeld()&KEY_DUP)position.y+=1.0f;
		if(keysHeld()&KEY_DDOWN)position.y-=1.0f;
		if(keysHeld()&KEY_DLEFT)position.x+=1.0f;
		if(keysHeld()&KEY_DRIGHT)position.x-=1.0f;

		//R/L to bring object closer to or move it further from the camera
		if(keysHeld()&KEY_R)position.z+=1.0f;
		if(keysHeld()&KEY_L)position.z-=1.0f;

		md2InstanceUpdate(&gladosInstance);

		gsDrawFrame();

		gspWaitForEvent(GSPEVENT_VBlank0, true);
	}

	gsExit();
	gfxExit();
	return 0;
}
