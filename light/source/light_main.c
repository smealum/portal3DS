#include <stdio.h>
#include <stdlib.h>
#include "light_room.h"
#include "light_lightmap.h"

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		printf("AS3D lightmap generator\n");
		printf("\t usage : light <file.map>\n");
		return 0;
	}

	room_s room;
	readRoom(argv[1], &room, 0);
	lightMapData_s ld;
	generateLightmaps(&room, &ld);

	return 0;
}
