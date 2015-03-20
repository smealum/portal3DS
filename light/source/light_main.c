#include <stdio.h>
#include <stdlib.h>
#include "light_room.h"
#include "light_room_io.h"
#include "light_lightmap.h"

void* bufferizeFile(char* filename, u32* size, bool binary)
{
	FILE* file;
	
	if(!binary)file = fopen(filename, "r+");
	else file = fopen(filename, "rb+");
	
	if(!file)return NULL;
	
	u8* buffer;
	long lsize;
	fseek (file, 0 , SEEK_END);
	lsize = ftell (file);
	rewind (file);
	buffer=(u8*)malloc(lsize);
	if(size)*size=lsize;
	
	if(!buffer)
	{
		fclose(file);
		return NULL;
	}
		
	fread(buffer, 1, lsize, file);
	fclose(file);
	return buffer;
}

void writeVect3Di(vect3Di_s* v, FILE* f)
{
	if(!v || !f)return;

	fwrite(v, sizeof(vect3Di_s), 1, f);
}

extern u8 currentResolution;

int main(int argc, char** argv)
{
	if(argc < 3)
	{
		printf("AS3D lightmap generator\n");
		printf("\t usage : light <file.map> <out.map>\n");
		return 0;
	}

	room_s room;
	readRoom(argv[1], &room, 0);
	lightMapData_s ld;
	while(generateLightmaps(&room, &ld)<0)currentResolution--;
	printf("final resolution : %d\n",currentResolution);

	u32 size;
	u8* buffer = bufferizeFile(argv[1], &size, true);

	FILE* f = fopen(argv[2], "wb");
	
	mapHeader_s* h = (mapHeader_s*)buffer;
	h->lightPosition = size;

	fwrite(buffer, size, 1, f);

	writeVect3Di(&ld.lmSize,f);
	fwrite(ld.buffer, sizeof(u8), ld.lmSize.x*ld.lmSize.y, f);
	fwrite(ld.coords, sizeof(lightMapCoordinates_s), ld.num, f);

	fclose(f);

	return 0;
}
