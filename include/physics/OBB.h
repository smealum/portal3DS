#ifndef OBB_H
#define OBB_H

#include "utils/math.h"
#include "gfx/md2.h"

#define NUMOBJECTS (8)

#define NUMOBBSEGMENTS (12)
#define NUMOBBFACES (6)
#define MAXCONTACTPOINTS (32) // pool system ?
#define PENETRATIONTHRESHOLD (1<<6)
#define MAXPENETRATIONBOX (1<<6)

// #define SLEEPTHRESHOLD (50)
#define SLEEPTHRESHOLD (0.0f)
#define SLEEPTIMETHRESHOLD (48)

static const u8 OBBSegments[NUMOBBSEGMENTS][2]={{0,1},{1,2},{3,2},{0,3},
										 {5,4},{5,6},{6,7},{4,7},
										 {3,4},{0,5},{1,6},{2,7}};
static const u8 OBBSegmentsPD[NUMOBBSEGMENTS][2]={{0,0},{1,2},{3,0},{0,2},
										 {5,2},{5,0},{6,2},{4,0},
										 {3,1},{0,1},{1,1},{2,1}};

static const u8 OBBFaces[NUMOBBFACES][4]={{0,1,2,3},{4,5,6,7},{0,5,4,3},{0,1,6,5},{1,2,7,6},{2,3,4,7}};
static const s8 OBBFacesPDDN[NUMOBBFACES][4]={{0,0,2,-2},{5,0,2,2},{0,1,2,-1},{0,0,1,-3},{1,1,2,1},{3,0,1,3}};

typedef enum
{
	BOXCOLLISION,
	PLANECOLLISION,
	TESTPOINT,
	AARCOLLISION
}contactPoint_type;

typedef struct
{
	vect3Df_s point;
	vect3Df_s normal;
	u16 penetration;
	void* target;
	contactPoint_type type;
}contactPoint_struct;

contactPoint_struct contactPoints[MAXCONTACTPOINTS];

typedef struct
{
	float mass;
	float transformationMatrix[9]; //3x3
	float invInertiaMatrix[9]; //3x3
	float invWInertiaMatrix[9]; //3x3
	u16 maxPenetration;
	contactPoint_struct* contactPoints; //all point to the same array, temporary
	u8 numContactPoints;
	vect3Df_s size;
	vect3Df_s position;
	vect3Df_s velocity, angularVelocity, forces, moment;
	vect3Df_s angularMomentum;
	vect3Df_s AABBo, AABBs;
	md2_instance_t* modelInstance;
	u8 portal[2];
	u8 oldPortal[2];
	float oldPortalZ[2];
	float energy;
	u16 counter;
	s16 groundID;
	bool portaled;
	bool sleep;
	bool used;
}OBB_s;
// 4 + 9*4*3 + 4 + 

extern OBB_s objects[NUMOBJECTS];

void initOBB(OBB_s* o, vect3Df_s pos, vect3Df_s size, md2_instance_t* model, float mass, float angle);
void initObbTransformationMatrix(float* m, float angle);
void getOBBVertices(OBB_s* o, vect3Df_s* v);
void drawOBB(OBB_s* o);
void applyOBBImpulses(OBB_s* o);
void applyOBBForce(OBB_s* o, vect3Df_s p, vect3Df_s f);
void updateOBB(OBB_s* o);

void initOBBs(void);
void updateOBBs(void);
void drawOBBs(void);
void wakeOBBs(void);
OBB_s* createOBB(vect3Df_s position, vect3Df_s size, md2_instance_t* model, float mass, float angle);
void updateOBBPortals(OBB_s* o, u8 id, bool init);
void getVertices(vect3Df_s s, vect3Df_s p, vect3Df_s u1, vect3Df_s u2, vect3Df_s u3, vect3Df_s* v);
void collideOBBs(OBB_s* o1, OBB_s* o2);

bool clipSegmentOBB(float* ss, vect3Df_s *uu, vect3Df_s* p1, vect3Df_s* p2, vect3Df_s vv, vect3Df_s* uu1, vect3Df_s* uu2, vect3Df_s vv1, vect3Df_s* n1, vect3Df_s* n2, bool* b1, bool* b2, float* k1, float* k2);

#endif
