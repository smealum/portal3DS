#ifndef FILESYSTEM_H
#define FILESYSTEM_H

void filesystemInit(int argc, char** argv);
void filesystemExit();

FILE* openFile(const char* fn, const char* mode);

#endif
