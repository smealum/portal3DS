#ifndef AAR_H //axis aligned rectangle !
#define AAR_H

#include "utils/math.h"
#include "physics/OBB.h"

#define NUMAARSEGMENTS (4)

#define ORIGNODESIZE (4096)
#define NODESIZE (nodeSize)

typedef struct
{
	vect3Df_s position, size, normal;
	bool touched;
	bool used;
}AAR_s;
// 4*3 + 4*3 + 4*3 + 1 = 37
// 2*2 + 1 + 3*1 + 1 + 1 = 10

typedef struct
{
	u16* data;
	u8 length;
}node_s;

typedef struct
{
	node_s* nodes;
	u16 width, height;
	vect3Df_s m, M;
}grid_s;

extern u32 nodeSize;

void initAARs(void);
void drawAARs(void);
void toggleAAR(u16 id);
void generateGrid(grid_s* g);
void updateAAR(u16 id, vect3Df_s position);
void AARsOBBContacts(OBB_s* o, bool sleep);
AAR_s* createAAR(vect3Df_s position, vect3Df_s size, vect3Df_s normal);
bool AAROBBContacts(AAR_s* a, OBB_s* o, vect3Df_s* v, bool port);
bool intersectAABBAAR(vect3Df_s o1, vect3Df_s s, vect3Df_s o2, vect3Df_s sp);

#endif
