#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfx/gs.h"
#include "gfx/text.h"
#include "gfx/md2.h"
#include "menu/menu.h"
#include "menu/cameratransition.h"
#include "game/camera.h"

#include "logo_vsh_shbin.h"

camera_s menuCamera;
char* startString;
char* menuString;

md2_instance_t gladosInstance, gladosLairInstance;
md2_model_t gladosModel, gladosLairModel;
texture_s gladosTexture, gladosLairTexture;

cameraState_s testCameraState;
cameraTransition_s cameraTransition;
texture_s logoTexture, rotateLogoTexture;

int currentCameraState;

DVLB_s* logoDvlb;
shaderProgram_s logoProgram;

float logoRectangleData[] = {1.0f, 1.0f, 0.0f,
						1.0f, -1.0f, 0.0f,
						0.0f, -1.0f, 0.0f,
						1.0f, 1.0f, 0.0f,
						0.0f, -1.0f, 0.0f,
						0.0f, 1.0f, 0.0f};

float logoangle;

u32* logoRectangleVertexData = NULL;

const u32 logoBaseAddr=0x14000000;

int logoUniformTextureDimensions;

char* menuItems[] = {"level 1", "level 2", "level 3", "level 4", "..."};
char* menuItems2[] = {"level 5", "level 6", "level 7", "level 8", "back"};
int menuSel;

void generateMenuString(char* out, char** items, int n, int sel)
{
	if(!out || !items)return;
	int cnt=0;

	out[cnt++] = '\1';

	int i;
	for(i=0; i<n; i++)
	{
		if(sel == i)
		{
			sprintf(&out[cnt], "> %s\n", items[i]);
			cnt += 3 + strlen(items[i]);
		}else{
			sprintf(&out[cnt], "%s\n", items[i]);
			cnt += 1 + strlen(items[i]);
		}
	}
}

// topscreen
void renderMenuFrame(u32* outBuffer, u32* outDepthBuffer)
{
	GPU_SetViewport((u32*)osConvertVirtToPhys((u32)outDepthBuffer),(u32*)osConvertVirtToPhys((u32)outBuffer),0,0,240,400);
	
	GPU_DepthMap(-1.0f, 0.0f);
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

	gsMatrixMode(GS_MODELVIEW);
	gsPushMatrix();
		gsLoadIdentity();
		useCamera(&menuCamera);

		gsSwitchRenderMode(md2GsMode);
		md2InstanceDraw(&gladosInstance);
		md2InstanceDraw(&gladosLairInstance);

	gsPopMatrix();

	GPU_SetDepthTestAndWriteMask(true, GPU_ALWAYS, GPU_WRITE_ALL);
	
	if(currentCameraState==2)
	{
		gsPushMatrix();
			gsSwitchRenderMode(-1);

			GPU_SetAttributeBuffers(
				1, // number of attributes
				(u32*)osConvertVirtToPhys(logoBaseAddr), // we use the start of linear heap as base since that's where all our buffers are located
				GPU_ATTRIBFMT(0, 3, GPU_FLOAT), // we want v0 (vertex position)
				0xFFE, // mask : we want v0
				0x0, // permutation : we use identity
				1, // number of buffers : we have one attribute per buffer
				(u32[]){(u32)logoRectangleVertexData-logoBaseAddr}, // buffer offsets (placeholders)
				(u64[]){0x0}, // attribute permutations for each buffer
				(u8[]){1} // number of attributes for each buffer
				);

			gsSetShader(&logoProgram);

			gsMatrixMode(GS_MODELVIEW);
			gsLoadIdentity();

			gsPushMatrix();
				gsTranslate(0.15f, 0.325f, 0.0f);
				gsScale(64.0f * 2.0f / 240.0f, 64.0f * 2.0f / 400.0f, 1.0f);

				gsTranslate(0.5f, 0.5f, 0.0f);
				gsRotateZ(logoangle+=0.01f);
				gsTranslate(-0.5f, -0.5f, 0.0f);

				gsUpdateTransformation();

				textureBind(&rotateLogoTexture, GPU_TEXUNIT0);
				GPU_SetFloatUniform(GPU_VERTEX_SHADER, logoUniformTextureDimensions, (u32*)(float[]){0.0f, 0.0f, 1.0f, 1.0f}, 1);

				GPU_DrawArray(GPU_TRIANGLES, 6);
			gsPopMatrix();

			gsPushMatrix();
				gsTranslate(-0.25f, 1.0f, 0.0f);
				gsScale(256.0f * 2.0f / 400.0f, 128.0f * 2.0f / 240.0f, 1.0f);
				gsRotateZ(M_PI/2);

				gsUpdateTransformation();

				textureBind(&logoTexture, GPU_TEXUNIT0);
				GPU_SetFloatUniform(GPU_VERTEX_SHADER, logoUniformTextureDimensions, (u32*)(float[]){0.0f, 0.0f, 1.0f, 1.0f}, 1);

				GPU_DrawArray(GPU_TRIANGLES, 6);
			gsPopMatrix();
		gsPopMatrix();
	}

	gsPushMatrix();
		gsSwitchRenderMode(-1);
		textStartDrawing();

		if(currentCameraState==2)
		{
			textDrawString(0.3f, -0.5f, startString);
		}else{
			textDrawString(0.95f, -0.4f, menuString);
		}
	gsPopMatrix();

	GPU_FinishDrawing();
}

void drawMenuBottom(u32* outBuffer, u32* outDepthBuffer)
{

}

void menuInit()
{
	//initialize GS
	gsInit(NULL, renderMenuFrame, drawMenuBottom);

	//init text
	textInit();
	startString = textMakeString("\1press start");
	menuString = textMakeString("\1> menu item 1\n> menu item 2\n> menu item 3\n> menu item 4");
	menuSel = 0;

	//init md2
	md2Init();

	//load resources
	textureLoad(&gladosTexture, "glados.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)|GPU_TEXTURE_WRAP_S(GPU_REPEAT)|GPU_TEXTURE_WRAP_T(GPU_REPEAT), 0);
	md2ReadModel(&gladosModel, "glados.md2");
	textureLoad(&gladosLairTexture, "gladoslair.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)|GPU_TEXTURE_WRAP_S(GPU_REPEAT)|GPU_TEXTURE_WRAP_T(GPU_REPEAT), 0);
	md2ReadModel(&gladosLairModel, "gladoslairv2.md2");

	md2InstanceInit(&gladosInstance, &gladosModel, &gladosTexture);
	md2InstanceChangeAnimation(&gladosInstance, 1, false);
	gladosInstance.speed /= 2;

	md2InstanceInit(&gladosLairInstance, &gladosLairModel, &gladosLairTexture);

	initCamera(&menuCamera);

	testCameraState.position = vect3Df(0,0,0);
	testCameraState.angle = vect3Df(0,0,0);

	currentCameraState = 2;
	cameraTransition = startCameraTransition(&cameraStates[currentCameraState], &cameraStates[currentCameraState], 1);
	
	textureLoad(&logoTexture, "menu_logo.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);
	textureLoad(&rotateLogoTexture, "rotate_logo.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR), 0);

	logoRectangleVertexData = linearAlloc(sizeof(logoRectangleData));
	memcpy(logoRectangleVertexData, logoRectangleData, sizeof(logoRectangleData));

	logoDvlb = DVLB_ParseFile((u32*)logo_vsh_shbin, logo_vsh_shbin_size);
	if(logoDvlb)
	{
		shaderProgramInit(&logoProgram);
		shaderProgramSetVsh(&logoProgram, &logoDvlb->DVLE[0]);

		logoUniformTextureDimensions = shaderInstanceGetUniformLocation(logoProgram.vertexShader, "textureDimensions");	
		printf("tdim %d\n", logoUniformTextureDimensions);
	}

	logoangle = 0.0f;

	printf("ready\n");
}

void menuExit()
{
	textureFree(&gladosTexture);
	md2FreeModel(&gladosModel);
	textureFree(&gladosLairTexture);
	md2FreeModel(&gladosLairModel);
	textureFree(&logoTexture);
	textureFree(&rotateLogoTexture);

	md2Exit();
	textExit();

	gsExit();
}

extern int currentLevel;

bool menuFrame()
{
	//controls
	hidScanInput();
	//START to exit to hbmenu
	if(keysDown()&KEY_SELECT)return true;

	// if(keysHeld()&KEY_CPAD_UP)testCameraState.position = vaddf(testCameraState.position, moveCameraVector(&menuCamera, vect3Df(0.0f, 0.0f, -0.1f), true));
	// if(keysHeld()&KEY_CPAD_DOWN)testCameraState.position = vaddf(testCameraState.position, moveCameraVector(&menuCamera, vect3Df(0.0f, 0.0f, 0.1f), true));
	// if(keysHeld()&KEY_CPAD_LEFT)testCameraState.position = vaddf(testCameraState.position, moveCameraVector(&menuCamera, vect3Df(-0.1f, 0.0f, 0.0f), true));
	// if(keysHeld()&KEY_CPAD_RIGHT)testCameraState.position = vaddf(testCameraState.position, moveCameraVector(&menuCamera, vect3Df(0.1f, 0.0f, 0.0f), true));

	// if(keysHeld()&KEY_A)testCameraState.angle.x += 0.01f;
	// if(keysHeld()&KEY_Y)testCameraState.angle.x -= 0.01f;
	// if(keysHeld()&KEY_X)testCameraState.angle.y += 0.01f;
	// if(keysHeld()&KEY_B)testCameraState.angle.y -= 0.01f;
	// if(keysHeld()&KEY_R)testCameraState.angle.z += 0.01f;
	// if(keysHeld()&KEY_L)testCameraState.angle.z -= 0.01f;

	// if(keysDown()&KEY_A)
	// {
	// 	cameraTransition = startCameraTransition(&cameraStates[currentCameraState], &cameraStates[0], 24);
	// 	currentCameraState = 0;
	// }

	// if(keysDown()&KEY_B)
	// {
	// 	cameraTransition = startCameraTransition(&cameraStates[currentCameraState], &cameraStates[1], 24);
	// 	currentCameraState = 1;
	// }

	switch(currentCameraState)
	{
		case 2:
			// "PRESS START"
			if(keysDown()&KEY_START)
			{
				cameraTransition = startCameraTransition(&cameraStates[currentCameraState], &cameraStates[0], 24);
				currentCameraState = 0;
			}
			break;
		case 0:
			// menu 1
			generateMenuString(menuString, menuItems, 5, menuSel);

			if(keysDown()&KEY_UP)menuSel--;
			if(keysDown()&KEY_DOWN)menuSel++;
			if(keysDown()&KEY_A)
			{
				if(menuSel < 4)
				{
					currentLevel = menuSel;
					return true;
				}else{
					cameraTransition = startCameraTransition(&cameraStates[currentCameraState], &cameraStates[1], 24);
					currentCameraState = 1;
					menuSel = 0;
				}
			}

			if(menuSel<0)menuSel+=5;
			menuSel %= 5;

			break;
		case 1:
			// menu 2
			generateMenuString(menuString, menuItems2, 5, menuSel);

			if(keysDown()&KEY_UP)menuSel--;
			if(keysDown()&KEY_DOWN)menuSel++;
			if(keysDown()&KEY_A)
			{
				if(menuSel < 4)
				{
					currentLevel = menuSel + 4;
					return true;
				}else{
					cameraTransition = startCameraTransition(&cameraStates[currentCameraState], &cameraStates[0], 24);
					currentCameraState = 0;
				}
			}

			if(menuSel<0)menuSel+=5;
			menuSel %= 5;

			break;
	}

	// applyCameraState(&menuCamera, &testCameraState);
	updateCameraTransition(&menuCamera, &cameraTransition);
	updateCamera(&menuCamera);

	// printf("pos %f %f %f\n",testCameraState.position.x,testCameraState.position.y,testCameraState.position.z);
	// printf("ang %f %f %f\n",testCameraState.angle.x,testCameraState.angle.y,testCameraState.angle.z);

	md2InstanceUpdate(&gladosInstance);

	gsDrawFrame();

	gspWaitForEvent(GSPEVENT_VBlank0, true);

	return false;
}
