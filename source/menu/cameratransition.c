#include <3ds.h>
#include "menu/cameratransition.h"

cameraState_s cameraStates[]={{(vect3Df_s){3.48f, -0.9f, 10.84f}, (vect3Df_s){0.31f, -0.31f, 0.33f}},
								{(vect3Df_s){0.2617f, 1.274f, 13.23f}, (vect3Df_s){-0.10f, -0.04f, -0.01f}},
								{(vect3Df_s){-4.34f, -3.91f, 9.19f}, (vect3Df_s){0.34f, 0.41f, 0.26f}},
								};

void applyCameraState(camera_s* c, cameraState_s* cs)
{
	if(!c || !cs)return;

	c->position=cs->position;

	loadIdentity44((float*)c->orientation);
	rotateMatrixX((float*)c->orientation, cs->angle.x, false);
	rotateMatrixY((float*)c->orientation, cs->angle.y, false);
	rotateMatrixZ((float*)c->orientation, cs->angle.z, false);

	// printf("p : %f %f %f\n",cs->position.x,cs->position.y,cs->position.z);
	// printf("a : %f %f %f\n",cs->angle.x,cs->angle.y,cs->angle.z);
}

cameraTransition_s startCameraTransition(cameraState_s* s, cameraState_s* f, int length)
{
	cameraTransition_s ct;

	ct.start=s;
	ct.finish=f;

	ct.progress=0;
	ct.length=length;

	return ct;
}

void updateCameraTransition(camera_s* c, cameraTransition_s* ct)
{
	if(!c || !ct || !ct->start || !ct->finish || ct->progress>=ct->length)return;

	cameraState_s cs;

	//TEMP TEST
	cs.position=vect3Df(ct->start->position.x+((ct->finish->position.x-ct->start->position.x)*ct->progress)/ct->length,
				ct->start->position.y+((ct->finish->position.y-ct->start->position.y)*ct->progress)/ct->length,
				ct->start->position.z+((ct->finish->position.z-ct->start->position.z)*ct->progress)/ct->length);

	cs.angle=vect3Df(ct->start->angle.x+((ct->finish->angle.x-ct->start->angle.x)*ct->progress)/ct->length,
				ct->start->angle.y+((ct->finish->angle.y-ct->start->angle.y)*ct->progress)/ct->length,
				ct->start->angle.z+((ct->finish->angle.z-ct->start->angle.z)*ct->progress)/ct->length);

	applyCameraState(c, &cs);

	ct->progress++;
}
