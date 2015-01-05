#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include "utils/filesystem.h"

#define directoryName "content/"
#define defaultPath ("sdmc:/" directoryName)

static char* currentPath=NULL;

void filesystemInit(int argc, char** argv)
{
	if(argc && argv && !memcmp("sdmc:", argv[0], 5))
	{
		//grab path from arguments
		int l; for(l=strlen(argv[0]); l>=0 && argv[0][l]!='/'; l--);
		l++; currentPath=malloc(l+strlen(directoryName)+1);
		memcpy(currentPath, argv[0], l);
		strcpy(&currentPath[l], directoryName);
	}else{
		currentPath=(char*)defaultPath;
	}
	printf("%s\n%s\n",argv[0],currentPath);
}

void filesystemExit()
{

}

FILE* openFile(const char* fn, const char* mode)
{
	if(!fn || !mode)return NULL;
	static char fullPath[1024];
	sprintf(fullPath, "%s%s", currentPath, fn);
	printf("opening %s\n", fullPath);
	return fopen(fullPath, mode);
}
