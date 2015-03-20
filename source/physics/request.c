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
	r->data=requestTypes[t].dataSize?malloc(requestTypes[t].dataSize):NULL;
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
	md2_instance_t* model;
	float mass, angle;
}requestCreateObbData_s;

request_s* createRequestCreateObb(OBB_s** out, vect3Df_s position, vect3Df_s size, md2_instance_t* model, float mass, float angle)
{
	request_s* r=createNewRequest(REQUEST_CREATE_OBB);
	if(!r)return r;
	requestCreateObbData_s* d=(requestCreateObbData_s*)r->data;

	d->out = out;
	d->position = position;
	d->size = size;
	d->model = model;
	d->mass = mass;
	d->angle = angle;

	return r;
}

void requestCreateObbHandler(struct physicsThread_s* p, request_s* r)
{
	if(!p || !r)return;
	requestCreateObbData_s* d=(requestCreateObbData_s*)r->data;

	OBB_s* o = createOBB(d->position, d->size, d->model, d->mass, d->angle);
	if(d->out)*d->out = o;
}

//REQUEST_RESET_OBB
typedef struct
{
	OBB_s* target;
	vect3Df_s position, size;
	md2_instance_t* model;
	float mass, angle;
}requestResetObbData_s;

request_s* createRequestResetObb(OBB_s* target, vect3Df_s position, vect3Df_s size, md2_instance_t* model, float mass, float angle)
{
	request_s* r=createNewRequest(REQUEST_RESET_OBB);
	if(!r)return r;
	requestResetObbData_s* d=(requestResetObbData_s*)r->data;

	d->target = target;
	d->position = position;
	d->size = size;
	d->model = model;
	d->mass = mass;
	d->angle = angle;

	return r;
}

void requestResetObbHandler(struct physicsThread_s* p, request_s* r)
{
	if(!p || !r)return;
	requestResetObbData_s* d=(requestResetObbData_s*)r->data;

	initOBB(d->target, d->position, d->size, d->model, d->mass, d->angle);
}

//REQUEST_CREATE_AAR
typedef struct
{
	AAR_s** out;
	vect3Df_s position, size, normal;
}requestCreateAarData_s;

request_s* createRequestCreateAar(AAR_s** out, vect3Df_s position, vect3Df_s size, vect3Df_s normal)
{
	request_s* r=createNewRequest(REQUEST_CREATE_AAR);
	if(!r)return r;
	requestCreateAarData_s* d=(requestCreateAarData_s*)r->data;

	if(size.x<0){position.x+=size.x;size.x=-size.x;}
	if(size.y<0){position.y+=size.y;size.y=-size.y;}
	if(size.z<0){position.z+=size.z;size.z=-size.z;}

	d->out = out;
	d->position = position;
	d->size = size;
	d->normal = normal;

	return r;
}

void requestCreateAarHandler(struct physicsThread_s* p, request_s* r)
{
	if(!p || !r)return;
	requestCreateAarData_s* d=(requestCreateAarData_s*)r->data;

	AAR_s* a = createAAR(d->position, d->size, d->normal);
	if(d->out)*d->out = a;
}

//REQUEST_GENERATE_GRID

request_s* createRequestGenerateGrid()
{
	return createNewRequest(REQUEST_GENERATE_GRID);
}

void requestGenerateGridHandler(struct physicsThread_s* p, request_s* r)
{
	if(!p || !r)return;

	generateGrid(NULL);
}

requestType_s requestTypes[NUM_REQUEST_TYPES]= {
	(requestType_s){requestCreateObbHandler, sizeof(requestCreateObbData_s)}, // REQUEST_CREATE_OBB
	(requestType_s){requestResetObbHandler, sizeof(requestResetObbData_s)}, // REQUEST_RESET_AAR
	(requestType_s){requestCreateAarHandler, sizeof(requestCreateAarData_s)}, // REQUEST_CREATE_AAR
	(requestType_s){requestGenerateGridHandler, 0}, // REQUEST_GENERATE_GRID
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
