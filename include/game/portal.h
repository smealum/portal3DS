#ifndef PORTAL_H
#define PORTAL_H

#include "utils/math.h"
#include "game/camera.h"

typedef struct portal_s
{
	vect3Df_s position;
	vect3Df_s normal, plane[2];
	float matrix[4*4];
	struct portal_s* target;
}portal_s;

typedef void (*renderSceneCallback)(camera_s* c);

void portalInit();
void portalExit();

void initPortal(portal_s* p);
void drawPortal(portal_s* p, renderSceneCallback callback, camera_s* c);

void updatePortalOrientation(portal_s* p, vect3Df_s plane0, vect3Df_s normal);

#endif
