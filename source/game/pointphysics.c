#include <3ds.h>
#include "game/pointphysics.h"
#include "game/room.h"
#include "game/portal.h"
#include "game/elevator.h"
#include "game/walldoor.h"

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

bool isPortalInRectangle(vect3Df_s pr, vect3Df_s sr, portal_s* p, vect3Df_s* o)
{
	*o=vsubf(pr,p->position);

	if(!sr.x)return fabs(o->x) < 0.1f;
	else if(!sr.y)return fabs(o->y) < 0.1f;
	else return fabs(o->z) < 0.1f;
}

void collidePortal(vect3Df_s pr, vect3Df_s sr, portal_s* p, vect3Df_s* point)
{
	vect3Df_s o;
	if(!isPortalInRectangle(pr,sr,p,&o))return;
	vect3Df_s v=vsubf(*point,p->position);

	const vect3Df_s u1=p->plane[0], u2=p->plane[1];

	float xp=vdotf(v,u1)+PORTAL_WIDTH;
	float yp=vdotf(v,u2)+PORTAL_HEIGHT;

	// printf("IN PORTAL ? %f %f\n", xp, yp);
	if(xp<0 || yp<0 || xp>=PORTAL_WIDTH*2 || yp>=PORTAL_HEIGHT*2)return;
	float d1=(xp), d2=(yp), d3=PORTAL_WIDTH*2-(xp), d4=PORTAL_HEIGHT*2-(yp);

	if(d1<d2 && d1<d3 && d1<d4)
	{
		*point=vaddf(*point,vmulf(u1,-d1));
	}else if(d2<d1 && d2<d3 && d2<d4)
	{
		*point=vaddf(*point,vmulf(u2,-d2));
	}else if(d3<d1 && d3<d2 && d3<d4)
	{
		*point=vaddf(*point,vmulf(u1,d3));
	}else{
		*point=vaddf(*point,vmulf(u2,d4));
	}
	// printf("YES %f %f %f %f\n",d1,d2,d3,d4);
}

vect3Df_s getClosestPointRectangle(vect3Df_s rectOrigin, vect3Df_s rectSize, vect3Df_s o)
{
	vect3Df_s u1, u2;
	float x,y,sx,sy;
	
	if(rectSize.x){sx=fabs(rectSize.x);u1=vect3Df((rectSize.x>0)?(1.0f):(-(1.0f)),0,0);}
	else{sx=fabs(rectSize.y);u1=vect3Df(0,(rectSize.y>0)?(1.0f):(-(1.0f)),0);}
	
	if(rectSize.z){sy=fabs(rectSize.z);u2=vect3Df(0,0,(rectSize.z>0)?(1.0f):(-(1.0f)));}
	else{sy=fabs(rectSize.y);u2=vect3Df(0,(rectSize.y>0)?(1.0f):(-(1.0f)),0);}
	
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

extern vect3Df_s normGravityVector; //TEMP
const float transY=4.0f; //TEMP

bool collideRectangle(physicalPoint_s* o, vect3Df_s p, vect3Df_s s)
{
	if(!o)return false;

	vect3Df_s o2 = getClosestPointRectangle(p, s, o->position);

	int i;
	for(i=0; i < NUM_PORTALS; i++)
	{
		if(portals[i].target)collidePortal(p, s, &portals[i], &o2);
	}

	vect3Df_s v = vsubf(o2, o->position);

	float gval = vdotf(v, normGravityVector);
	vect3Df_s v2 = vsubf(v, vmulf(normGravityVector, gval));
	float sqd = (v2.x*v2.x)+(v2.y*v2.y)+(v2.z*v2.z)+((gval*gval)/ transY);

	if(sqd < o->sqRadius)
	{
		float sqd = (v2.x*v2.x)+(v2.y*v2.y)+(v2.z*v2.z)+(gval*gval)/transY;
		float d = sqrtf(sqd);
		v = vdivf(vmulf(vect3Df(v.x, v.y, v.z), -(o->radius-d)), d);
		o->position = vaddf(o->position, v);
		return true;
	}
	return false;
}

u8 checkObjectElevatorCollision(physicalPoint_s* o, room_s* r, elevator_s* ev)
{
	if(!o || !r || !ev)return 0;

	u8 ret=0;

	if(collideRectangle(o,vaddf(ev->realPosition,vect3Df(-ELEVATOR_SIZE/2,0,-ELEVATOR_SIZE/2)),vect3Df(ELEVATOR_SIZE,0,ELEVATOR_SIZE)))ret=2;

	vect3Df_s u=vect3Df(o->position.x-ev->position.x,0,o->position.z-ev->position.z);
	float v=vmagf(u);

	if(fabs(o->position.y-ev->position.y)>ELEVATOR_HEIGHT)return ret;

	if(ev->state==ELEVATOR_OPEN)
	{
		switch(ev->direction&(~(1<<ELEVATOR_UPDOWNBIT)))
		{
			case 1:
				if(u.x<-(v*cos(ELEVATOR_ANGLE)))return ret;
				break;
			case 4:
				if(u.z>(v*cos(ELEVATOR_ANGLE)))return ret;
				break;
			case 5:
				if(u.z<-(v*cos(ELEVATOR_ANGLE)))return ret;
				break;
			default:
				if(u.x>(v*cos(ELEVATOR_ANGLE)))return ret;
				break;
		}
	}

	if(v<ELEVATOR_RADIUS_IN)
	{
		if(v+o->radius>=ELEVATOR_RADIUS_IN)
		{
			u=vdivf(vmulf(u,ELEVATOR_RADIUS_IN-o->radius-v),v);
			o->position=vaddf(o->position,u);
			ret=1;
		}
	}else if(v<o->radius+ELEVATOR_RADIUS_OUT)
	{
		u=vdivf(vmulf(u,o->radius+ELEVATOR_RADIUS_OUT-v),v);
		o->position=vaddf(o->position,u);
		ret=1;
	}

	return ret;
}

bool checkObjectCollision(physicalPoint_s* o, room_s* r)
{	
	bool ret=false;
	
	listCell_s* l = r->rectangles.first;
	while(l)
	{
		if(l->data.collides) ret = collideRectangle(o, convertRectangleVector(l->data.position), convertRectangleVector(l->data.size)) || ret;
		l = l->next;
	}

	// //platforms
	// int i;
	// for(i=0;i<NUMPLATFORMS;i++)
	// {
	// 	if(platform[i].used && collideRectangle(o,r,vaddf(platform[i].position,vect3Df(-PLATFORMSIZE,0,-PLATFORMSIZE)),vect3Df(PLATFORMSIZE*2,0,PLATFORMSIZE*2))) //add culling
	// 	{
	// 		platform[i].touched=true;
	// 		ret=true;
	// 	}
	// }

	//elevators
	u8 val=0;
	if((entryWallDoor.used && checkObjectElevatorCollision(o,r,&entryWallDoor.elevator)) || (exitWallDoor.used && (val=checkObjectElevatorCollision(o,r,&exitWallDoor.elevator))))ret=true;
	if(val==2)closeElevator(&exitWallDoor.elevator);
	
	// //timed buttons
	// if(checkObjectTimedButtonsCollision(o,r))ret=true;

	return ret;
}

void collideObjectRoom(physicalPoint_s* pp, room_s* r)
{
	if(!pp || !r)return;
	vect3Df_s oldPosition = pp->position;

	float length=vmagf(pp->speed);
	
	bool ret=false;
	
	pp->position = vaddf(pp->position,pp->speed);
	ret = checkObjectCollision(pp, r);
	
	pp->contact=ret;
	
	vect3Df_s os=pp->speed;
	pp->speed=vect3Df(pp->position.x-oldPosition.x, pp->position.y-oldPosition.y, pp->position.z-oldPosition.z);
	pp->speed=vect3Df((os.x*pp->speed.x>0)?(pp->speed.x):(0),(os.y*pp->speed.y>0)?(pp->speed.y):(0),(os.z*pp->speed.z>0)?(pp->speed.z):(0));

	if(pp->contact)
	{
		 //floor friction
		vect3Df_s s=vsubf(pp->speed,vmulf(normGravityVector,vdotf(normGravityVector,pp->speed)));
		pp->speed=vsubf(pp->speed,vdivf(s,2));
	}else{
		//air friction
		vect3Df_s s=vsubf(pp->speed,vmulf(normGravityVector,vdotf(normGravityVector,pp->speed)));
		pp->speed=vsubf(pp->speed,vdivf(s,32));
	}

	if(fabs(pp->speed.x)<0.01f)pp->speed.x=0;
	if(fabs(pp->speed.z)<0.01f)pp->speed.z=0;
}

