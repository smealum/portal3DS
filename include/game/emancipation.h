#ifndef EMANCIPATION_H
#define EMANCIPATION_H

#include "gfx/md2.h"
#include "game/room.h"
#include "game/player.h"
#include "physics/OBB.h"

#define NUMEMANCIPATIONGRIDS (16)
#define NUMEMANCIPATORS (16)

#define EMANCIPATIONGRIDHEIGHT (HEIGHTUNIT*8)
#define BLACKENINGTIME (16)
#define FADINGTIME (24)

typedef struct
{
	vect3Df_s position, velocity, axis;
	md2_instance_t modelInstance;
	float transformationMatrix[9];
	float angle;
	u16 counter;
	bool used;
}emancipator_s;

typedef struct
{
	vect3Df_s position;
	float length;
	bool direction; //true=Z, false=X
	bool used;
}emancipationGrid_s;

void initEmancipation(void);
void exitEmancipation(void);

void createEmancipator(md2_instance_t* mi, vect3Df_s pos, float* m);
void updateEmancipators(void);
void drawEmancipators(void);

void createEmancipationGrid(room_s* r, vect3Di_s pos, float l, bool dir);
void updateEmancipationGrids(player_s* pl);
void drawEmancipationGrids(void);

void getEmancipationGridAAR(emancipationGrid_s* eg, vect3Df_s* pos, vect3Df_s* sp);
bool collideBoxEmancipationGrids(OBB_s* o);
bool collideLineEmancipationGrids(vect3Df_s l, vect3Df_s v, float d);

#endif
