#ifndef TEXT_H
#define TEXT_H

void textInit();
void textExit();
void textStartDrawing();
char* textMakeString(const char* s);
void textFreeString(char* s);
void textDrawString(float x, float y, const char* s);

#endif
