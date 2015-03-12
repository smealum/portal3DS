#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "game/activator.h"
#include "game/platform.h"
#include "game/cubes.h"
#include "utils/math.h"

void initActivator(activator_s* a)
{
	if(!a)return;
	
	a->numSlots=0;
}

void initActivatableObject(activatableObject_s* ao)
{
	if(!ao)return;

	ao->active = false;
	ao->oldActive = false;
}

void changeActivation(activatableObject_s* ao, bool value)
{
	if(!ao)return;
	
	ao->oldActive = ao->active;
	ao->active = value;
}

void useActivator(activator_s* a)
{
	if(!a)return;
	int i;
	for(i=0;i<a->numSlots;i++)
	{
		changeActivation(a->slot[i], true);
	}
}

void unuseActivator(activator_s* a)
{
	if(!a)return;
	int i;
	for(i=0;i<a->numSlots;i++)
	{
		changeActivation(a->slot[i], false);
	}
}

void addActivatorTarget(activator_s* a, activatableObject_s* target, activatorTarget_t type)
{
	if(!a)return;
	if(a->numSlots>=NUMACTIVATORSLOTS)return;
	
	a->slot[a->numSlots] = target;
	
	a->numSlots++;
	unuseActivator(a);
}
