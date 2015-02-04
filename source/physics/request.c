#include <3ds.h>
#include <stdlib.h>

#include "physics/physicsThread.h"
#include "physics/request.h"

#define REQUESTPOOL_ALLOCSIZE (128)

request_s* requestPool;

void initRequestPool(void)
{
	requestPool=NULL;
}

void allocatePoolRequests(void)
{
	request_s* newRequests=malloc(sizeof(request_s)*REQUESTPOOL_ALLOCSIZE);
	int i; for(i=0;i<REQUESTPOOL_ALLOCSIZE-1;i++){newRequests[i].data=NULL;newRequests[i].next=&newRequests[i+1];}
	newRequests[REQUESTPOOL_ALLOCSIZE-1].data=NULL; newRequests[REQUESTPOOL_ALLOCSIZE-1].next=requestPool;
	requestPool=newRequests;
}

request_s* getNewRequest(void)
{
	if(!requestPool)allocatePoolRequests();
	request_s* r=requestPool;
	if(!r)return r;
	requestPool=r->next;
	r->next=NULL;
	return r;
}

request_s* createNewRequest(requestTypes_t t)
{
	if(t<0 || t>=NUM_REQUEST_TYPES)return NULL;
	request_s* r=getNewRequest();
	if(!r)return r;
	r->type=t;
	r->data=malloc(requestTypes[t].dataSize);
	return r;
}

void freeRequest(request_s* r)
{
	if(!r)return;
	if(r->data){free(r->data);r->data=NULL;}
	r->next=requestPool;
	requestPool=r;
}

//REQUEST_CREATE_OBB
typedef struct
{
	OBB_s** out;
	vect3Df_s position, size;
	float mass, angle;
}requestCreateObbData_s;

request_s* createRequestCreateObb(OBB_s** out, vect3Df_s position, vect3Df_s size, float mass, float angle)
{
	request_s* r=createNewRequest(REQUEST_CREATE_OBB);
	if(!r)return r;
	requestCreateObbData_s* d=(requestCreateObbData_s*)r->data;

	d->out = out;
	d->position = position;
	d->size = size;
	d->mass = mass;
	d->angle = angle;

	return r;
}

void requestCreateObbHandler(struct physicsThread_s* p, request_s* r)
{
	if(!p || !r)return;
	requestCreateObbData_s* d=(requestCreateObbData_s*)r->data;

	if(d->out)*d->out = createOBB(d->position, d->size, d->mass, d->angle);
}

requestType_s requestTypes[NUM_REQUEST_TYPES]= {
	(requestType_s){requestCreateObbHandler, sizeof(requestCreateObbData_s)}, // REQUEST_CREATE_OBB
};

//request
void handleRequest(physicsThread_s* p, request_s* r)
{
	if(!p || !r || r->type>=NUM_REQUEST_TYPES)return;

	requestTypes[r->type].handler(p,r);
}

//request queue
void initRequestQueue(requestQueue_s* rq)
{
	if(!rq)return;

	rq->first=rq->last=NULL;
	rq->length=0;
}

void queueRequest(requestQueue_s* rq, request_s* r)
{
	if(!rq || !r)return;

	r->next=NULL;
	if(!rq->length)rq->first=rq->last=r;
	else rq->last=rq->last->next=r;
	rq->length++;
}

void appendRequestQueue(requestQueue_s* rq1, requestQueue_s* rq2)
{
	if(!rq1 || !rq2 || !rq2->length)return;

	if(!rq1->length)*rq1=*rq2;
	else{
		rq1->last->next=rq2->first;
		rq1->last=rq2->last;
		rq1->length+=rq2->length;
	}
	rq2->length=0;rq2->first=rq2->last=NULL;
}

request_s* unqueueRequest(requestQueue_s* rq)
{
	if(!rq || !rq->length)return NULL;

	request_s* r=rq->first;
	if(rq->length==1)rq->first=rq->last=NULL;
	else rq->first=r->next;
	r->next=NULL;
	rq->length--;

	return r;
}
