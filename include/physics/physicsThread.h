#ifndef PHYSICSTHREAD_H
#define PHYSICSTHREAD_H

#include <3ds.h>
#include "physics/request.h"

#define PHYSICSTHREAD_STACKSIZE (0x4000)
#define PHYSICSTHREAD_TMPBUFSIZE (4096*20)

typedef struct physicsThread_s
{
	Handle thread;
	Handle requestMutex;
	bool exit;
	requestQueue_s privateList; //only accessible from producer
	requestQueue_s requestList; //accessible by anyone, given they've locked requestMutex
	u64 stack[PHYSICSTHREAD_STACKSIZE/8];
	u64 tmpBuffer[PHYSICSTHREAD_TMPBUFSIZE/8];
}physicsThread_s;

void initPhysicsThread(physicsThread_s* p);
void exitPhysicsThread(physicsThread_s* p);

#endif
