#ifndef TIMEDBUTTON_H
#define TIMEDBUTTON_H

#include "utils/math.h"
#include "gfx/md2.h"
#include "game/room.h"
#include "game/activator.h"
#include "game/pointphysics.h"
#include "physics/physics.h"

#define NUMTIMEDBUTTONS (16)

typedef struct
{
	room_s* room;
	activator_s activator;
	md2_instance_t modelInstance;
	vect3Df_s position;
	float angle;
	u16 active;
	bool used;
	u8 id;
}timedButton_s;

void initTimedButtons(void);
void exitTimedButtons(void);
void activateTimedButton(timedButton_s* tb);
timedButton_s* collideRayTimedButtons(vect3Df_s o, vect3Df_s v, float l);
timedButton_s* createTimedButton(room_s* r, vect3Di_s position, float angle);
bool checkObjectTimedButtonsCollision(physicalPoint_s* o, room_s* r);
void drawTimedButtons(void);
void updateTimedButtons(void);

#endif
