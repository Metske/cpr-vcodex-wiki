#pragma once
#include <cstdint>
namespace freeink {
namespace ui {
struct Rect {
    int16_t x, y, width, height;
};
}
}
using freeink::ui::Rect;

class GfxRenderer;
class GUI_class {
public:
    void drawHeader(GfxRenderer& r, Rect rect, const char* title) {}
};
extern GUI_class GUI;
