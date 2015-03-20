#ifndef SLUDGE_H
#define SLUDGE_H

#include "utils/math.h"
#include "game/room.h"
#include "physics/physics.h"

#define SLUDGEMARGIN (TILESIZE)

void initSludge(void);
void exitSludge(void);
void generateSludgeGeometry(void);
void addSludgeRectangle(rectangle_s* rec);
void drawSludge(room_s* r);
void updateSludge(void);

bool collideBoxSludge(OBB_s* o);
bool collideAABBSludge(vect3Df_s p, vect3Df_s s);

#endif
