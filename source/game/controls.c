#include <3ds.h>
#include <math.h>
#include "game/controls.h"
#include "game/timedbutton.h"

OBB_s* gravityGunObject;

void initControls()
{
	gravityGunObject = NULL;
}

void exitControls()
{

}

void updateControls(player_s* p)
{
	circlePosition cpad;
	circlePosition cstick;
	
	hidCircleRead(&cpad);
	irrstCstickRead(&cstick);

	rotatePlayer(p, vect3Df((abs(cstick.dy)<2)?0:(-cstick.dy*0.0005f), (abs(cstick.dx)<2)?0:(cstick.dx*0.0005f), 0.0f));

	if(abs(cpad.dx) > 15 || abs(cpad.dy) > 15) //dead zone
	{
		float factor = 0.0015f;

		if(p->flying)factor*=2;
		else if(!p->object.contact)factor*=0.06f;
		else updatePlayerWalk(p, cpad.dy*factor*2, cpad.dx*factor);

		movePlayer(p, vect3Df(cpad.dx*factor, 0.0f, -cpad.dy*factor));
	}

	if(keysDown()&KEY_ZL)
	{
		// "USE" key
		vect3Df_s u = moveCameraVector(&p->camera, vect3Df(0.0f, 0.0f, -1.0f), true);
		timedButton_s* tb = collideRayTimedButtons(p->object.position, u, TILESIZE_FLOAT*2);
		if(tb)
		{
			activateTimedButton(tb);
		}else{
			OBB_s* o = collideRayBoxes(p->object.position, u, TILESIZE_FLOAT*4);
			if(o)
			{
				gravityGunObject = o;
			}
		}
	}

	if(keysDown()&KEY_ZR)
	{
		// JUMP key
		if(p->object.contact)
		{
			p->object.speed.y += 0.6f;			
		}
	}

	if(gravityGunObject)
	{
		if(!(keysHeld()&KEY_ZL))
		{
			gravityGunObject = NULL;
			md2InstanceChangeAnimation(&p->gunInstance, 0, false);
			md2InstanceChangeAnimation(&p->gunInstance, 1, true);
		}else{
			const vect3Df_s u = moveCameraVector(&p->camera, vect3Df(0.0f, 0.0f, -5.0f), true);
			const vect3Df_s t = vaddf(u, p->object.position);
			const vect3Df_s v = vmulf(vsubf(t, gravityGunObject->position), 1.75f);
			setObbVelocity(gravityGunObject, v);
			md2InstanceChangeAnimation(&p->gunInstance, 2, false);
		}
	}else if(p->gunInstance.currentAnim == 2){
		md2InstanceChangeAnimation(&p->gunInstance, 0, false);
		md2InstanceChangeAnimation(&p->gunInstance, 1, true);
	}
}
