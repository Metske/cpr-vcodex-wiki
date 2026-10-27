#pragma once
#include <string>
#include "fontIds.h"

class GfxRenderer {
public:
    int getScreenWidth() { return 300; }
    int getScreenHeight() { return 400; }
    int getLineHeight(int fontId) { return 20; }
    int getTextAdvanceX(int fontId, const char* text, EpdFontFamily family) { return 10; }
    void clearScreen() {}
    std::string truncatedText(int fontId, const char* text, int maxWidth) { return text; }
    void drawText(int fontId, int x, int y, const char* text, bool a, EpdFontFamily family) {}
    void drawLine(int x1, int y1, int x2, int y2, bool b) {}
    int getTextWidth(int fontId, const char* text) { return 10; }
    void displayBuffer() {}
};
