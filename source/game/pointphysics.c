#include <3ds.h>
#include "game/pointphysics.h"
#include "game/room.h"

void initPhysicalPoint(physicalPoint_s* pp, vect3Df_s position, float radius)
{
	if(!pp)return;

	pp->position = position;
	pp->speed = vect3Df(0.0f, 0.0f, 0.0f);

	pp->radius = radius;
	pp->sqRadius = radius * radius;

	pp->contact = false;
}

void updatePhysicalPoint(physicalPoint_s* pp)
{
	if(!pp)return;

	// pp->speed=vaddf(pp->speed, gravityVector);
}

vect3Df_s getClosestPointRectangle(vect3Df_s rectOrigin, vect3Df_s rectSize, vect3Df_s o)
{
	vect3Df_s u1, u2;
	float x,y,sx,sy;
	
	if(rectSize.x){sx=fabs(rectSize.x);u1=vect3Df((rectSize.x>0)?inttof32(1):(-inttof32(1)),0,0);}
	else{sx=fabs(rectSize.y);u1=vect3Df(0,(rectSize.y>0)?inttof32(1):(-inttof32(1)),0);}
	
	if(rectSize.z){sy=fabs(rectSize.z);u2=vect3Df(0,0,(rectSize.z>0)?inttof32(1):(-inttof32(1)));}
	else{sy=fabs(rectSize.y);u2=vect3Df(0,(rectSize.y>0)?inttof32(1):(-inttof32(1)),0);}
	
	o=vsubf(o, rectOrigin);
	
	x=vdotf(o, u1);
	y=vdotf(o, u2);
	
	bool r=true;

	r=r && x<sx && x>=0;
	r=r && y<sy && y>=0;
	
	if(r)return vaddf(rectOrigin, vect3Df((x*u1.x)+(y*u2.x), (x*u1.y)+(y*u2.y), (x*u1.z)+(y*u2.z)));
	
	if(x<0)
	{
		x=0;
		if(y<0)y=0;
		else if(y>sy)y=sy;
	}else if(x>sx)
	{
		x=sx;
		if(y<0)y=0;
		else if(y>sy)y=sy;
	}else if(y<0)
	{
		y=0;
		if(x<0)x=0;
		else if(x>sx)y=sx;
	}else if(y>sy)
	{
		y=sy;
		if(x<0)x=0;
		else if(x>sx)x=sx;
	}
	
	return vaddf(rectOrigin, vect3Df((x*u1.x)+(y*u2.x), (x*u1.y)+(y*u2.y), (x*u1.z)+(y*u2.z)));
}

// bool collideRectangle(physicsObject_s* o, room_s* r, vect3Df_s p, vect3Df_s s)
// {
// 	if(!o || !r)return false;
// 	vect3Df_s o2 = getClosestPointRectangle(p, s, o->position);
// 	vect3Df_s v = vectDifference(o2, o->position);
// 	float gval = vdotf(v, normGravityVector);
// 	vect3Df_s v2 = vectDifference(v, vectMult(normGravityVector, gval));
// 	float sqd = mulf32(v2.x, v2.x)+mulf32(v2.y, v2.y)+mulf32(v2.z, v2.z)+divf32(mulf32(gval, gval), transY);
// 	if(sqd<o->sqRadius)
// 	{
// 		float sqd = (v2.x*v2.x)+(v2.y*v2.y)+(v2.z*v2.z)+divf32(gval*gval, transY);
// 		u32 d = sqrtf32((sqd));
// 		v = divideVect(vectMult(vect(v.x, v.y, v.z), -((o->radius<<6)-d)), d);
// 		o->position = addVect(o->position, v);
// 		return true;
// 	}
// 	return false;
// }

void collideObjectRoom(physicalPoint_s* pp, room_s* r)
{
	if(!pp || !r)return;
	vect3Df_s oldPosition = pp->position;

	float length=vmagf(pp->speed);
	
	bool ret=false;
	
	pp->position = vaddf(pp->position,pp->speed);
	// ret = checkObjectCollision(o,r);
	
	// pp->contact=ret;
	
	// vect3Df_s os=pp->speed;
	// pp->speed=vect3Df(pp->position.x-oldPosition.x, pp->position.y-oldPosition.y, pp->position.z-oldPosition.z);
	// pp->speed=vect((os.x*pp->speed.x>0)?(pp->speed.x):(0),(os.y*pp->speed.y>0)?(pp->speed.y):(0),(os.z*pp->speed.z>0)?(pp->speed.z):(0));

	// if(pp->contact)
	// {
	// 	 //floor friction
	// 	vect3Df_s s=vsubf(pp->speed,vectMult(normGravityVector,dotProduct(normGravityVector,pp->speed)));
	// 	pp->speed=vsubf(pp->speed,vectDivInt(s,2));
	// }else{
	// 	//air friction
	// 	vect3Df_s s=vsubf(pp->speed,vectMult(normGravityVector,dotProduct(normGravityVector,pp->speed)));
	// 	pp->speed=vsubf(pp->speed,vectDivInt(s,32));
	// }

	// if(abs(pp->speed.x)<3)pp->speed.x=0;
	// if(abs(pp->speed.z)<3)pp->speed.z=0;
}

