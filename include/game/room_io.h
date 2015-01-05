#ifndef ROOM_IO_H
#define ROOM_IO_H

#include <3ds.h>
#include "room.h"

#define MAPHEADER_SIZE (256)

#define MAP_READ_LIGHT (1<<0)
#define MAP_READ_ENTITIES (1<<1)
#define MAP_READ_INFO (1<<7)

typedef struct
{
	u32 dataSize;
	u32 dataPosition;
	u32 rectanglesPosition;
	u32 entityPosition;
	u32 lightPosition;
	u32 sludgePosition;

	u8 reserved[MAPHEADER_SIZE-6*4]; //for future use
}mapHeader_s;

void readRoom(char* filename, room_s* r, u8 flags);

#endif
