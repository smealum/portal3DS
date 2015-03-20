#include <3ds.h>
#include "gfx/gs.h"
#include "gfx/text.h"
#include "gfx/md2.h"
#include "menu/menu.h"
#include "game/camera.h"

camera_s menuCamera;
char* testString;

md2_instance_t gladosInstance, gladosLairInstance;
md2_model_t gladosModel, gladosLairModel;
texture_s gladosTexture, gladosLairTexture;

// topscreen
void renderMenuFrame(u32* outBuffer, u32* outDepthBuffer)
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
		GPU_TEVOPERANDS(0,0,0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_MODULATE, GPU_MODULATE, 
		0xFFFFFFFF);
	GPU_SetDummyTexEnv(1);
	GPU_SetDummyTexEnv(2);
	GPU_SetDummyTexEnv(3);
	GPU_SetDummyTexEnv(4);
	GPU_SetDummyTexEnv(5);

	//draw object

	gsMatrixMode(GS_MODELVIEW);
	gsPushMatrix();
		gsLoadIdentity();
		useCamera(&menuCamera);

		gsSwitchRenderMode(md2GsMode);
		md2InstanceDraw(&gladosInstance);
		md2InstanceDraw(&gladosLairInstance);

	gsPopMatrix();

	gsSwitchRenderMode(-1);
	textStartDrawing();
	textDrawString(0, 0, testString);

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
	testString = textMakeString("\1hello this is a test\nwith newline support");

	//init md2
	md2Init();

	//load resources
	textureLoad(&gladosTexture, "glados.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)|GPU_TEXTURE_WRAP_S(GPU_REPEAT)|GPU_TEXTURE_WRAP_T(GPU_REPEAT), 0);
	md2ReadModel(&gladosModel, "glados.md2");
	textureLoad(&gladosLairTexture, "gladoslair.png", GPU_TEXTURE_MAG_FILTER(GPU_LINEAR)|GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)|GPU_TEXTURE_WRAP_S(GPU_REPEAT)|GPU_TEXTURE_WRAP_T(GPU_REPEAT), 0);
	md2ReadModel(&gladosLairModel, "gladoslairv2.md2");

	md2InstanceInit(&gladosInstance, &gladosModel, &gladosTexture);
	md2InstanceChangeAnimation(&gladosInstance, 1, false);

	md2InstanceInit(&gladosLairInstance, &gladosLairModel, &gladosLairTexture);

	initCamera(&menuCamera);

	printf("ready\n");
}

void menuExit()
{
	md2Exit();
	textExit();

	gsExit();
}

bool menuFrame()
{
	//controls
	hidScanInput();
	//START to exit to hbmenu
	if(keysDown()&KEY_START)return true;

	if(keysHeld()&KEY_CPAD_UP)moveCamera(&menuCamera, moveCameraVector(&menuCamera, vect3Df(0.0f, 0.0f, -0.4f), true));
	if(keysHeld()&KEY_CPAD_DOWN)moveCamera(&menuCamera, moveCameraVector(&menuCamera, vect3Df(0.0f, 0.0f, 0.4f), true));
	if(keysHeld()&KEY_CPAD_LEFT)moveCamera(&menuCamera, moveCameraVector(&menuCamera, vect3Df(-0.4f, 0.0f, 0.0f), true));
	if(keysHeld()&KEY_CPAD_RIGHT)moveCamera(&menuCamera, moveCameraVector(&menuCamera, vect3Df(0.4f, 0.0f, 0.0f), true));

	updateCamera(&menuCamera);

	// printf("cam %f %f %f\n",menuCamera.position.x,menuCamera.position.y,menuCamera.position.z);

	md2InstanceUpdate(&gladosInstance);

	gsDrawFrame();

	gspWaitForEvent(GSPEVENT_VBlank0, true);

	return false;
}
