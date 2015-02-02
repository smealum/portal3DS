#ifndef PORTAL_H
#define PORTAL_H

#include "utils/math.h"
#include "game/camera.h"
#include "physics/AAR.h"

typedef struct portal_s
{
	vect3Df_s position;
	vect3Df_s normal, plane[2];
	float matrix[4*4];
	struct portal_s* target;
	AAR_s guideAAR[4];
}portal_s;

typedef void (*renderSceneCallback_t)(camera_s* c, int depth, u8 stencil);

void portalInit();
void portalExit();

void initPortal(portal_s* p);
void drawPortals(portal_s* portals[], int n, renderSceneCallback_t callback, camera_s* c, int depth, u8 stencil);

void updatePortalOrientation(portal_s* p, vect3Df_s plane0, vect3Df_s normal);

#endif
