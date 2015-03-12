#ifndef ACTIVATOR_H
#define ACTIVATOR_H

#define NUMACTIVATORSLOTS (4)

typedef enum
{
	DISPENSER_TARGET,
	PLATFORM_TARGET,
	DOOR_TARGET,
	WALLDOOR_TARGET,
	NOT_TARGET
}activatorTarget_t;

typedef struct
{
	bool active, oldActive;
}activatableObject_s;

typedef struct
{
	activatableObject_s* slot[NUMACTIVATORSLOTS];
	u8 numSlots;
}activator_s;

void initActivator(activator_s* a);
void initActivatableObject(activatableObject_s* ao);
void useActivator(activator_s* a);
void unuseActivator(activator_s* a);
void addActivatorTarget(activator_s* a, activatableObject_s* target, activatorTarget_t type);

#endif
