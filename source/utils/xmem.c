#include <3ds.h>
#include <malloc.h>
#include <unistd.h>

extern u8 __end__[];        // end of static code and data
extern u8* fake_heap_start;
extern u8* fake_heap_end;

u8 *getHeapStart() {
	return __end__;
}

u8 *getHeapEnd() {
	return (u8 *)sbrk(0);
}

u8 *getHeapLimit() {
	return fake_heap_end;
}

size_t latestUsed, latestFree;

size_t getMemUsed() {
	struct mallinfo mi = mallinfo();
	latestUsed=mi.uordblks;
	return latestUsed;
}

size_t getMemFree() {
	struct mallinfo mi = mallinfo();
	latestFree=mi.fordblks + (getHeapLimit() - getHeapEnd());
	return latestFree;
}
