#ifndef MATERIAL_H
#define MATERIAL_H

#include <3ds.h>
#include "gfx/texture.h"

#define NUMMATERIALS 256
#define NUMMATERIALSLICES 256

typedef struct
{
	texture_s* img;
	u16 id;
	s16 factorX;
	s16 factorY;
	bool align;
	bool used;
}materialSlice_s;

typedef struct
{
	materialSlice_s* top;
	materialSlice_s* side;
	materialSlice_s* bottom;
	u16 id;
	bool used;
}material_s;

static inline u16 getMaterialID(material_s* m){if(m)return m->id;return 0;}

void initMaterials(void);
material_s* createMaterial();
materialSlice_s* createMaterialSlice();
void loadMaterialSlice(materialSlice_s* ms, char* filename);
void loadMaterialSlices(char* filename);
void loadMaterials(char* filename);

material_s* getMaterial(u16 i);

char** getMaterialList(int* m, int** cl);
void freeMaterialList(char** l);

#endif
