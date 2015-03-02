#ifndef BIGBUTTON_H
#define BIGBUTTON_H

#include "utils/math.h"
#include "game/room.h"
#include "gfx/md2.h"
#include "physics/physics.h"

#define NUMBIGBUTTONS (16)

typedef struct
{
	room_s* room;
	rectangle_s* surface;
	// activator_struct activator;
	md2_instance_t modelInstance;
	vect3Df_s position;
	bool active;
	bool used;
	u8 id;
}bigButton_s;

void initBigButtons(void);
void freeBigButtons(void);
bigButton_s* createBigButton(room_s* r, vect3Di_s position);
void drawBigButtons(void);
void updateBigButtons(void);

#endif
