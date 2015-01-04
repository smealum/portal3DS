#ifndef TEXT_H
#define TEXT_H

void textInit();
void textExit();
void textStartDrawing();
char* textMakeString(const char* s);
void textFreeString(char* s);
void textDrawString(int x, int y, const char* s);

#endif
