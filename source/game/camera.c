#include <string.h>
#include <3ds.h>

#include "game/camera.h"
#include "gfx/gs.h"

vect3Df_s normGravityVector; //TEMP

void initCamera(camera_s* c)
{
	if(!c)return;

	initProjectionMatrix((float*)c->projection, 1.3962634f, 240.0f/400.0f, 0.01f, 1000.0f);
	rotateMatrixZ((float*)c->projection, M_PI/2, false); //because framebuffer is sideways...
	loadIdentity44((float*)c->orientation);
	c->position=vect3Df(0.0f, 0.0f, 0.0f);

	normGravityVector=vect3Df(0.0f, -1.0f, 0.0f); //TEMP
}

void updateCameraFrustum(camera_s* c)
{
	if(!c)return;

	float final[4*4];
	multMatrix44((float*)c->projection, (float*)c->modelview, final);

	const vect4Df_s rowX = getMatrixColumn4(final, 0);
	const vect4Df_s rowY = getMatrixColumn4(final, 1);
	const vect4Df_s rowZ = getMatrixColumn4(final, 2);
	const vect4Df_s rowW = getMatrixColumn4(final, 3);

	//ordered by priority for culling
	c->frustumPlane[0] = vnormf4(vsubf4(rowW, rowZ)); //near plane
	c->frustumPlane[1] = vnormf4(vaddf4(rowW, rowX)); //right plane
	c->frustumPlane[2] = vnormf4(vsubf4(rowW, rowX)); //left plane
	c->frustumPlane[3] = vnormf4(vaddf4(rowW, rowY)); //top plane
	c->frustumPlane[4] = vnormf4(vsubf4(rowW, rowY)); //bottom plane
	c->frustumPlane[5] = vnormf4(vaddf4(rowW, rowZ)); //far plane
}

void rotateCamera(camera_s* c, vect3Df_s a)
{
	if(!c)return;
	
	rotateMatrixX((float*)c->orientation, -a.x, true);
	rotateMatrixAxis((float*)c->orientation, -a.y, normGravityVector, false);
}

void setCameraPosition(camera_s* c, vect3Df_s v)
{
	if(!c)NULL;

	c->position=v;
}

void moveCamera(camera_s* c, vect3Df_s v)
{
	if(!c)NULL;
	
	vect3Df_s v1=vect3Df(c->orientation[2][0],c->orientation[2][1],c->orientation[2][2]);
	v1=vnormf(vsubf(v1,vmulf(normGravityVector,vdotf(normGravityVector,v1))));
	
	vect3Df_s u=vect3Df((v.z*v1.x)+(v.x*c->orientation[0][0]), (v.z*v1.y)+(v.x*c->orientation[0][1]), (v.z*v1.z)+(v.x*c->orientation[0][2]));

	c->position=vaddf(c->position, u);
	// c->object.speed=addVect(c->object.speed,u);
}

void updateCamera(camera_s* c)
{
	if(!c)return;

	memcpy(c->modelview, c->orientation, sizeof(mtx44));
	translateMatrix((float*)c->modelview, -c->position.x, -c->position.y, -c->position.z);

	updateCameraFrustum(c);
}

void useCamera(camera_s* c)
{
	if(!c)return;

	gsMatrixMode(GS_PROJECTION);
	gsLoadIdentity();
	gsMultMatrix((float*)c->projection);

	gsMatrixMode(GS_MODELVIEW);
	gsMultMatrix((float*)c->modelview);
}

bool pointInCameraFrustum(camera_s* c, vect3Df_s pt)
{
	if(!c)return false;
	const vect4Df_s pt4=vect4Df(pt.x,pt.y,pt.z,1.0f);
	int i; for(i=0;i<6;i++)if(vdotf4(pt4,c->frustumPlane[i])<0.0f)return false;
	return true;
}

vect3Df_s box[]={(vect3Df_s){0.f,0.f,0.f},
				(vect3Df_s){1.f,0.f,0.f},
				(vect3Df_s){0.f,1.f,0.f},
				(vect3Df_s){0.f,0.f,1.f},
				(vect3Df_s){1.f,1.f,0.f},
				(vect3Df_s){1.f,0.f,1.f},
				(vect3Df_s){0.f,1.f,1.f},
				(vect3Df_s){1.f,1.f,1.f}};

//et "Assarsson and Moller report that they found no observable penalty in the rendering when skipping further tests"
bool aabbInCameraFrustum(camera_s* c, vect3Df_s o, vect3Df_s s, int planes)
{
	if(!c)return false;
	if(planes<=0 || planes>6)return false;
	int i, j;
	for(i=0;i<planes;i++)
	{
		int in=0, out=0;
		for(j=0;j<8 && (!in || !out);j++)
		{
			const vect3Df_s pt=vaddf(o,vscalef(box[j],s));
			if(vdotf4(vect4Df(pt.x,pt.y,pt.z,1.0f),c->frustumPlane[i])<0.0f)out++;
			else in++;
		}
		if(!in)return false;
	}
	return true;
}
