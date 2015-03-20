#ifndef REQUEST_H
#define REQUEST_H

#include "utils/math.h"
#include "physics/OBB.h"
#include "physics/AAR.h"

struct physicsThread_s;

typedef enum
{
	REQUEST_CREATE_OBB,
	REQUEST_RESET_OBB,
	REQUEST_CREATE_AAR,
	REQUEST_GENERATE_GRID,
	NUM_REQUEST_TYPES
}requestTypes_t;

//request
typedef struct request_s
{
	requestTypes_t type;
	void* data;
	struct request_s* next;
}request_s;

void initRequestPool(void);
void allocatePoolRequests(void);
request_s* createNewRequest(requestTypes_t t);
request_s* getNewRequest(void);
void freeRequest(request_s* r);

void handleRequest(struct physicsThread_s* p, request_s* r);

//request type
typedef void (*requestHandler_func)(struct physicsThread_s* p, request_s* r);

typedef struct
{
	requestHandler_func handler; //executed by one of the producer threads
	u32 dataSize;
}requestType_s;

request_s* createRequestCreateObb(OBB_s** out, vect3Df_s position, vect3Df_s size, md2_instance_t* model, float mass, float angle);
request_s* createRequestResetObb(OBB_s* target, vect3Df_s position, vect3Df_s size, md2_instance_t* model, float mass, float angle);
request_s* createRequestCreateAar(AAR_s** out, vect3Df_s position, vect3Df_s size, vect3Df_s normal);
request_s* createRequestGenerateGrid();

extern requestType_s requestTypes[NUM_REQUEST_TYPES];

//request queue (FIFO)
typedef struct
{
	request_s* first;
	request_s* last;
	int length;
}requestQueue_s;

void initRequestQueue(requestQueue_s* rq);
void queueRequest(requestQueue_s* rq, request_s* r);
request_s* unqueueRequest(requestQueue_s* rq);
void appendRequestQueue(requestQueue_s* rq1, requestQueue_s* rq2);

#endif
