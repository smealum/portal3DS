#ifndef CAMERATRANSITION_H
#define CAMERATRANSITION_H

#include "game/camera.h"

typedef struct
{
	vect3Df_s position, angle;
}cameraState_s;

typedef struct
{
	cameraState_s *start, *finish;
	int progress, length;
}cameraTransition_s;

extern cameraState_s cameraStates[];
extern cameraTransition_s testTransition;

void applyCameraState(camera_s* c, cameraState_s* cs);

cameraTransition_s startCameraTransition(cameraState_s* s, cameraState_s* f, int length);
void updateCameraTransition(camera_s* c, cameraTransition_s* ct);

#endif
