#include <3ds.h>
#include <stdio.h>

#include "physics/physicsThread.h"
#include "physics/OBB.h"

void physicsThreadMain(u32 arg)
{
	physicsThread_s* p=(physicsThread_s*)arg;
	while(!p->exit)
	{
		svcWaitSynchronization(p->requestMutex, U64_MAX);
		appendRequestQueue(&p->privateList, &p->requestList);
		svcReleaseMutex(p->requestMutex);

		// bool debug=false;
		// u64 val=svcGetSystemTick();
		
		request_s* r=NULL;
		while((r=unqueueRequest(&p->privateList)) && !p->exit)
		{
			handleRequest(p, r);
			svcSleepThread(1000);
		}

		// if(debug)print("%d ticks\n",(int)(svcGetSystemTick()-val));
		updateOBBs();

		svcSleepThread(1000000);
	}
	svcExitThread();
}

void initPhysicsThread(physicsThread_s* p)
{
	if(!p)return;

	initRequestQueue(&p->privateList);
	initRequestQueue(&p->requestList);

	p->exit=false;
	svcCreateMutex(&p->requestMutex, false);
	Result val = svcCreateThread(&p->thread, (void*)physicsThreadMain, (u32)p, (u32*)&p->stack[PHYSICSTHREAD_STACKSIZE/8], 0x18, 1);
	printf("%08X (%08X)\n",(unsigned int)val,(unsigned int)p->thread);
	if(val)
	{
		//thread creation failed ! what do we do ?!
		printf("physics thread creation failed ! what do we do ?!\n");
		printf("%08X %08X %08X\n", (u32)physicsThreadMain, (u32)p, (u32)&p->stack[PHYSICSTHREAD_STACKSIZE/8]);
		while(1);
	}
}

void exitPhysicsThread(physicsThread_s* p)
{
	if(!p)return;

	p->exit=true;
	svcWaitSynchronization(p->thread, U64_MAX);
	svcCloseHandle(p->requestMutex);
	svcCloseHandle(p->thread);
}
