#ifndef PLATFORM_H
#define PLATFORM_H

#include "utils/math.h"
#include "game/room.h"
#include "gfx/md2.h"
#include "physics/physics.h"

#define NUMPLATFORMS (8)

typedef struct
{
	vect3Df_s position, velocity;
	vect3Df_s origin, destination;
	bool direction; //true=orig->dest
	bool touched, oldTouched;
	bool active, oldactive, backandforth;
	bool used;
	u8 id;
}platform_s;

extern platform_s platform[NUMPLATFORMS];

void initPlatforms(void);
void exitPlatforms(void);
void drawPlatforms(void);
void updatePlatforms(void);
platform_s* createPlatform(room_s* r, vect3Di_s orig, vect3Di_s dest, bool BAF);

#endif
