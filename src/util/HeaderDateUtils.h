#pragma once

#include <string>
#include <cstdint>

class GfxRenderer;

namespace HeaderDateUtils {

struct DisplayDateInfo {
  uint32_t timestamp = 0;
  bool usedFallback = false;
  DisplayDateInfo(uint32_t t=0, bool u=false) : timestamp(t), usedFallback(u) {}
};

DisplayDateInfo getDisplayDateInfo();
std::string getDisplayDateText();
std::string getSyncDayReminderText();
void drawTopLine(GfxRenderer& renderer, const std::string& dateText);
void drawHeaderWithDate(GfxRenderer& renderer, const char* title, const char* subtitle = nullptr);

}  // namespace HeaderDateUtils
