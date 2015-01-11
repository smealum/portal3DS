#ifndef CAMERA_H
#define CAMERA_H

#include <3ds.h>
#include "utils/math.h"

typedef struct
{
	mtx44 projection;
	mtx44 orientation;
	mtx44 modelview; //modelview = orientation*translate(position)
	vect3Df_s position;
	vect4Df_s frustumPlane[6];
}camera_s;

void initCamera(camera_s* c);
void updateCamera(camera_s* c);
void useCamera(camera_s* c);

void moveCamera(camera_s* c, vect3Df_s v);
void rotateCamera(camera_s* c, vect3Df_s a);
void setCameraPosition(camera_s* c, vect3Df_s v);

bool pointInCameraFrustum(camera_s* c, vect3Df_s pt);
bool aabbInCameraFrustum(camera_s* c, vect3Df_s o, vect3Df_s s, int planes);

vect3Df_s projectPointCamera(camera_s* c, vect3Df_s p);
vect3Df_s moveCameraVector(camera_s* c, vect3Df_s v);

#endif
