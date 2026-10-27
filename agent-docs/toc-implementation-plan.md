# CPR-vCodex: Wikipedia TOC & Section Jumper Architecture

## Overview
This document outlines the architectural plan to implement an on-device Table of Contents (TOC) for Wikipedia articles. Because of the ESP32's hardware constraints and the forward-only formatting engine, we cannot dynamically paginate to arbitrary indices. Instead, we will generate the TOC on-the-fly and implement a "Section Snap-Back" workaround for the `Previous Page` button.

## Phase 1: TOC Extraction via HTML Streaming
We need to modify the plain-text converter to extract headings without allocating memory for the rest of the article.

1. **Define the Data Structure:**
   In `src/util/HtmlToPlainText.h`, define:
   ```cpp
   struct TOCEntry {
       uint8_t level; // 2 for <h2>, 3 for <h3>
       std::string title;
       uint32_t charIdx; // The plain-text character index where this heading starts
   };
   ```

2. **Modify `streamHtmlToPlainTextImpl`:**
   * Update the template function in `src/util/HtmlToPlainText.cpp` to accept an optional `std::vector<TOCEntry>* outTOC`.
   * When the parser detects an `<h2>` or `<h3>` tag start, set a boolean flag `inHeading = true` and clear a temporary string buffer.
   * While `inHeading == true`, intercept any valid characters being appended and also append them to the temporary string buffer.
   * When the parser detects the closing tag (`</h2>` or `</h3>`), push the `TOCEntry` to `outTOC`.
   * **Crucial:** The `charIdx` must be captured as `metrics.exactTextSize` at the *start* of the heading.

3. **Expose a Public Method:**
   Create `std::vector<TOCEntry> extractArticleTOC(WikiDecompressor& decompressor)` that runs the stream in `Mode 0` (measurement mode) so it runs at raw decompression speed without building the full article string in RAM.

## Phase 2: The Reader UI & Menu
We need to trigger and display the TOC in `WikiReaderActivity`.

1. **State Management:**
   * Add `std::vector<TOCEntry> currentToc_` to `WikiReaderActivity.h`.
   * Clear it whenever `loadArticle()` is called.

2. **Long-Press Intercept:**
   * In `WikiReaderActivity::loop()`, listen for the long-press event on the SELECT or MENU button.
   * When triggered, if `currentToc_` is empty, reset the `WikiDecompressor` to `articleOffset_` and call `extractArticleTOC`.
   * Push a menu (e.g., re-using `MenuActivity` or a custom `TOCMenuActivity`) populated with the strings from `currentToc_`. Indent `<h3>` items with a few spaces for visual hierarchy.

## Phase 3: The Jump and Snap-Back Logic
This is the core workaround that bypasses the need for reverse-pagination.

1. **Implement `jumpToCharIdx`:**
   Add a method `WikiReaderActivity::jumpTo(uint32_t charIdx)`:
   ```cpp
   void WikiReaderActivity::jumpTo(uint32_t charIdx) {
       pageStartOffsets_.clear();
       pageStartOffsets_.push_back(charIdx);
       currentPage_ = 0;
       formatCurrentPage();
       requestUpdate();
   }
   ```
   *When a user selects an item from the TOC Menu, call `jumpTo(entry.charIdx)`.*

2. **Override `onPrevPage`:**
   Modify `WikiReaderActivity::onPrevPage()` to implement the Snap-Back:
   ```cpp
   void WikiReaderActivity::onPrevPage() {
       if (currentPage_ > 0) {
           // Normal previous page
           currentPage_--;
           formatCurrentPage();
           requestUpdate();
       } else {
           // We are at Page 0 of the current breadcrumb trail.
           // Snap back to the previous TOC section!
           if (currentToc_.empty()) {
               // Silently load TOC if we don't have it yet
               loadTOC();
           }
           
           uint32_t currentStart = pageStartOffsets_[0];
           uint32_t targetIdx = 0;
           
           // Find the last heading that is strictly BEFORE our current start index
           for (auto it = currentToc_.rbegin(); it != currentToc_.rend(); ++it) {
               if (it->charIdx < currentStart) {
                   targetIdx = it->charIdx;
                   break;
               }
           }
           
           if (currentStart > 0) {
               jumpTo(targetIdx);
           }
       }
   }
   ```
