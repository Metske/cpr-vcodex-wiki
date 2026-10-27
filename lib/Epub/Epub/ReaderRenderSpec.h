#pragma once
#include <cstdint>

// Shared section layout cache format version. Incrementing this invalidates
// both per-spine section .bin caches and per-book page_counts.bin caches.
constexpr uint8_t kSectionFileVersion = 49;

// The resolved text-rendering configuration a reader hands to the layout
// engine. Section-cache validation keys on every field: a section file built
// with a different spec is discarded and rebuilt.
//
// Build one via CrossPointSettings::readerRenderSpec(width, height), which
// fills every field: the settings-derived ones from the store, the viewport
// from the caller. Taking the viewport as arguments is what keeps a spec from
// existing in a half-filled state — the 0 defaults below are a last-resort
// backstop (a 0x0 viewport lays out nothing), not an invitation to omit it.
struct ReaderRenderSpec {
  int fontId = 0;
  float lineCompression = 1.0f;
  bool extraParagraphSpacing = false;
  bool forceParagraphIndents = false;
  uint8_t paragraphAlignment = 0;
  uint16_t viewportWidth = 0;
  uint16_t viewportHeight = 0;
  bool hyphenationEnabled = false;
  bool embeddedStyle = true;
  uint8_t imageRendering = 0;
  bool focusReadingEnabled = false;

  bool operator==(const ReaderRenderSpec& other) const {
    return fontId == other.fontId && lineCompression == other.lineCompression &&
           extraParagraphSpacing == other.extraParagraphSpacing &&
           forceParagraphIndents == other.forceParagraphIndents && paragraphAlignment == other.paragraphAlignment &&
           viewportWidth == other.viewportWidth && viewportHeight == other.viewportHeight &&
           hyphenationEnabled == other.hyphenationEnabled && embeddedStyle == other.embeddedStyle &&
           imageRendering == other.imageRendering && focusReadingEnabled == other.focusReadingEnabled;
  }

  bool operator!=(const ReaderRenderSpec& other) const { return !(*this == other); }
};
