#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "gfx/md2.h"
#include "utils/math.h"
#include "game/room.h"

#define ELEVATOR_UPDOWNBIT (4)
#define ELEVATOR_SIZE (TILESIZE*3)

#define ELEVATOR_RADIUS_IN (TILESIZE*2-0.03125/2)
#define ELEVATOR_RADIUS_OUT (TILESIZE*2+0.03125)
#define ELEVATOR_HEIGHT (TILESIZE*16)

#define ELEVATOR_ANGLE (M_PI * 3 / 16)

typedef enum
{
	ELEVATOR_ARRIVING,
	ELEVATOR_OPENING,
	ELEVATOR_OPEN,
	ELEVATOR_CLOSING,
	ELEVATOR_LEAVING
}elevatorState_t;

typedef struct
{
	vect3Df_s position, realPosition;
	rectangle_s* doorSurface;
	float progress;
	u8 direction;
	md2_instance_t modelInstance;
	elevatorState_t state;
	rectangle_s* floor;
}elevator_s;

void initElevators(void);
void freeElevators(void);
void initElevator(elevator_s* ev, room_s* r, vect3Di_s position, u8 direction, bool up);
void setElevatorArriving(elevator_s* ev, float distance);
void closeElevator(elevator_s* ev);
void updateElevator(elevator_s* ev);
void drawElevator(elevator_s* ev);

#endif
