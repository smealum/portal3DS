#include <stdio.h>
#include <stdlib.h>
#include "light_room.h"

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

	return 0;
}
