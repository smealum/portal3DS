#ifndef PORTAL_H
#define PORTAL_H

#include "utils/math.h"
#include "game/camera.h"
#include "physics/AAR.h"

#define NUM_PORTALS (2)

#define PORTAL_HEIGHT (4.0f)
#define PORTAL_WIDTH (2.0f)

typedef struct portal_s
{
	vect3Df_s position;
	vect3Df_s normal, plane[2];
	float matrix[4*4];
	struct portal_s* target;
	AAR_s guideAAR[4];
	float oldPlayerZ;
	bool oldPlayerR;
}portal_s;

extern portal_s portals[NUM_PORTALS];

typedef void (*renderSceneCallback_t)(camera_s* c, int depth, u8 stencil);

void portalInit();
void portalExit();

void initPortal(portal_s* p);
void drawPortals(portal_s* portals[], int n, renderSceneCallback_t callback, camera_s* c, int depth, u8 stencil);

vect3Df_s warpPortalVector(portal_s* p, vect3Df_s v);
void warpPortalMatrix(portal_s* p, float* m);

bool isPointInPortal(portal_s* p, vect3Df_s o, vect3Df_s *v, float* x, float* y, float* z);

void updatePortalOrientation(portal_s* p, vect3Df_s plane0, vect3Df_s normal);

void generateGuideAAR(portal_s* p);

#endif
