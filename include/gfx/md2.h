#ifndef MD2_H
#define MD2_H

#include <3ds.h>

#include "utils/math.h"
#include "gfx/texture.h"

/* MD2 header */
typedef struct
{
	int ident;
	int version;

	int skinwidth;
	int skinheight;

	int framesize;

	int num_skins;
	int num_vertices;
	int num_st;
	int num_tris;
	int num_glcmds;
	int num_frames;

	int offset_skins;
	int offset_st;
	int offset_tris;
	int offset_frames;
	int offset_glcmds;
	int offset_end;
}md2_header_t;

/* Texture name */
typedef struct
{
	char name[64];
}md2_skin_t;

/* Texture coords */
typedef struct
{
	short s;
	short t;
}md2_texCoord_t;

/* Triangle info */
typedef struct
{
	unsigned short vertex[3];
	unsigned short st[3];
}md2_triangle_t;

/* Compressed vertex */
typedef struct
{
	unsigned char v[3];
	unsigned char normalIndex;
}md2_vertex_t;

/* Model frame */
typedef struct
{
	vect3Df_s scale;
	vect3Df_s translate;
	char name[16];
	md2_vertex_t *verts;
	u16 next;
}md2_frame_t;

typedef struct
{
	u16 v, st;
}md2_vertperm_t;

typedef struct
{
	u16 start, end;
}md2_anim_t;

/* MD2 model structure */
typedef struct
{
	md2_header_t header;

	md2_skin_t *skins;
	md2_texCoord_t *texcoords;
	md2_triangle_t *triangles;
	md2_frame_t *frames;

	u8 num_animations;
	md2_anim_t* animations;

	md2_vertperm_t* permutation;
	u32 permutation_size;

	u32 skin_width, skin_height;

	u16* indices;
}md2_model_t;

typedef struct
{
	u16 currentFrame, nextFrame;
	u8 currentAnim, oldAnim;
	float interpolation, speed;
	bool oneshot;
	texture_s* texture;
	md2_model_t* model;
}md2_instance_t;

void md2Init();
void md2Exit();
void md2StartDrawing();

int md2ReadModel(md2_model_t *mdl, const char *filename);
void md2FreeModel(md2_model_t *mdl);
void md2RenderFrame(md2_model_t *mdl, int n1, int n2, float interp, texture_s* t);

void md2InstanceInit(md2_instance_t* mi, md2_model_t* mdl, texture_s* t);
void md2InstanceChangeAnimation(md2_instance_t* mi, u16 newAnim, bool oneshot);
void md2InstanceUpdate(md2_instance_t* mi);
void md2InstanceDraw(md2_instance_t* mi);

#endif
