#include <string.h>
#include <3ds.h>
#include "gfx/text.h"
#include "gfx/texture.h"
#include "gfx/gs.h"

#include "text_vsh_shbin.h"

DVLB_s* textDvlb;
shaderProgram_s textProgram;
texture_s textTexture;
const u32 textBaseAddr=0x14000000;

void textInit()
{
	textDvlb = DVLB_ParseFile((u32*)text_vsh_shbin, text_vsh_shbin_size);

	if(!textDvlb)return;

	shaderProgramInit(&textProgram);
	shaderProgramSetVsh(&textProgram, &textDvlb->DVLE[0]);
	shaderProgramSetGsh(&textProgram, &textDvlb->DVLE[1], 4);

	textureLoad(&textTexture, "font.png", GPU_TEXTURE_MAG_FILTER(GPU_NEAREST)|GPU_TEXTURE_MIN_FILTER(GPU_NEAREST), 0);
}

void textExit()
{
	textureFree(&textTexture);
	shaderProgramFree(&textProgram);
	DVLB_Free(textDvlb);
}

void textStartDrawing()
{
	gsSetShader(&textProgram);

	textureBind(&textTexture, GPU_TEXUNIT0);

	GPU_SetAttributeBuffers(
		1, // number of attributes
		(u32*)osConvertVirtToPhys(textBaseAddr), // we use the start of linear heap as base since that's where all our buffers are located
		GPU_ATTRIBFMT(0, 1, GPU_UNSIGNED_BYTE), // we want v0 (character)
		0xFFE, // mask : we want v0
		0x0, // permutation : we use identity
		1, // number of buffers : we have one attribute per buffer
		(u32[]){(u32)0x0}, // buffer offsets (placeholder)
		(u64[]){0x0}, // attribute permutations for each buffer
		(u8[]){1} // number of attributes for each buffer
		);
}

char* textMakeString(const char* s)
{
	if(!s)return NULL;

	int l = strlen(s);
	char* ret = linearAlloc(l);
	if(!ret)return ret;
	memcpy(ret, s, l);
	return ret;
}

void textFreeString(char* s)
{
	if(!s)return;

	linearFree(s);
}

void textDrawString(int x, int y, const char* s)
{
	if(!s)return;
	if((u32)s < textBaseAddr)return;

	GPUCMD_AddWrite(GPUREG_ATTRIBBUFFER0_CONFIG0, (u32)s-textBaseAddr);

	GPU_DrawArray(GPU_UNKPRIM, strlen(s));
}
