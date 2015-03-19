#include <3ds.h>
#include <math.h>
#include "game/controls.h"
#include "game/timedbutton.h"

void initControls()
{

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

	rotatePlayer(p, vect3Df((abs(cstick.dy)<5)?0:(-cstick.dy*0.001f), (abs(cstick.dx)<5)?0:(cstick.dx*0.001f), 0.0f));

	if(abs(cpad.dx) > 15 || abs(cpad.dy) > 15) //dead zone
	{
		float factor = 0.0015f;

		if(p->flying)factor*=2;
		else if(!p->object.contact)factor=0;
		else updatePlayerWalk(p, cpad.dy*factor*2, cpad.dx*factor);

		movePlayer(p, vect3Df(cpad.dx*factor, 0.0f, -cpad.dy*factor));
	}

	if(keysDown()&KEY_ZL)
	{
		vect3Df_s u = moveCameraVector(&p->camera, vect3Df(0.0f, 0.0f, 1.0f), false);
		timedButton_s* tb=collideRayTimedButtons(p->object.position, u, TILESIZE_FLOAT*2);
		if(tb)
		{
			activateTimedButton(tb);
		}
	}
}
