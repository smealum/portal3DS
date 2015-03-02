#ifndef ENERGYBALL_H
#define ENERGYBALL_H

#define NUMENERGYDEVICES (8)
#define NUMENERGYBALLS (8)

#include "utils/math.h"
#include "game/room.h"
#include "gfx/md2.h"
#include "physics/physics.h"

typedef enum
{
	pX=0,
	mX=1,
	pY=2,
	mY=3,
	pZ=4,
	mZ=5
}deviceOrientation_t;

typedef struct
{
	md2_instance_t modelInstance;
	deviceOrientation_t orientation;
	rectangle_s* surface;
	vect3Df_s position;
	// activator_struct activator;
	bool type; //true=launcher
	bool active;
	bool used;
	u8 id;
}energyDevice_s;

typedef struct
{
	md2_instance_t modelInstance;
	energyDevice_s* launcher;
	vect3Df_s position, direction;
	float speed;
	u16 maxLife, life;
	bool used;
	u8 id;
}energyBall_s;

void initEnergyBalls(void);
void exitEnergyBalls(void);

energyDevice_s* createEnergyDevice(room_s* r, vect3Di_s pos, deviceOrientation_t or, bool type);
void drawEnergyDevices(void);
void updateEnergyDevices(void);

energyBall_s* createEnergyBall(energyDevice_s* launcher, vect3Df_s pos, vect3Df_s dir, u16 life);
void drawEnergyBalls(void);
void updateEnergyBalls(void);

#endif