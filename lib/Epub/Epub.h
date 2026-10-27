#pragma once

#include <Print.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Epub/BookMetadataCache.h"
#include "Epub/ReaderRenderSpec.h"
#include "Epub/css/CssParser.h"

class ZipFile;

class Epub {
  // the ncx file (EPUB 2)
  std::string tocNcxItem;
  // the nav file (EPUB 3)
  std::string tocNavItem;
  // where is the EPUBfile?
  std::string filepath;
  // the base path for items in the EPUB file
  std::string contentBasePath;
  // Uniq cache key based on filepath
  std::string cachePath;
  // Spine and TOC cache
  std::unique_ptr<BookMetadataCache> bookMetadataCache;
  // CSS parser for styling
  std::unique_ptr<CssParser> cssParser;
  // CSS files
  std::vector<std::string> cssFiles;
  // Per-spine exact page counts for the active render spec (0xFFFF = unknown)
  mutable std::vector<uint16_t> spinePageCounts_;
  mutable ReaderRenderSpec pageCountSpec_{};
  mutable bool pageCountsLoaded_ = false;
  mutable bool pageCountsDirty_ = false;

  void ensurePageCountCacheLoaded(const ReaderRenderSpec& spec) const;
  bool findContentOpfFile(std::string* contentOpfFile) const;
  bool parseContentOpf(BookMetadataCache::BookMetadata& bookMetadata, bool writeSpineEntries = true);
  bool parseTocNcxFile() const;
  bool parseTocNavFile() const;
  void discoverCssFilesFromZip();
  bool generateThumbBmpToPath(int width, int height, const std::string& thumbPath) const;
  CssParser::ParseResult parseCssFiles(CssParser::CacheStatus existingCacheStatus) const;

 public:
  explicit Epub(std::string filepath, const std::string& cacheDir) : filepath(std::move(filepath)) {
    // create a cache key based on the filepath
    cachePath = cacheDir + "/epub_" + std::to_string(std::hash<std::string>{}(this->filepath));
  }
  ~Epub() = default;
  std::string& getBasePath() { return contentBasePath; }
  bool load(bool buildIfMissing = true, bool skipLoadingCss = false);
  bool clearCache() const;
  void setupCacheDir() const;
  const std::string& getCachePath() const;
  const std::string& getPath() const;
  const std::string& getTitle() const;
  const std::string& getAuthor() const;
  const std::string& getLanguage() const;
  std::string getCoverBmpPath(bool cropped = false) const;
  bool generateCoverBmp(bool cropped = false) const;
  std::string getThumbBmpPath() const;
  std::string getThumbBmpPath(int height) const;
  std::string getThumbBmpPath(int width, int height) const;
  bool generateThumbBmp(int height) const;
  bool generateThumbBmp(int width, int height) const;
  uint8_t* readItemContentsToBytes(const std::string& itemHref, size_t* size = nullptr,
                                   bool trailingNullByte = false) const;
  bool readItemContentsToStream(const std::string& itemHref, Print& out, size_t chunkSize,
                                bool allowEarlyStop = false) const;
  // Extract an item to a file on SD. On failure the partial file is removed.
  bool extractItemToFile(const std::string& itemHref, const std::string& destPath) const;
  bool getItemSize(const std::string& itemHref, size_t* size) const;
  BookMetadataCache::SpineEntry getSpineItem(int spineIndex) const;
  BookMetadataCache::TocEntry getTocItem(int tocIndex) const;
  int getSpineItemsCount() const;
  int getTocItemsCount() const;
  int getSpineIndexForTocIndex(int tocIndex) const;
  int getTocIndexForSpineIndex(int spineIndex) const;
  bool getLogicalChapterSpan(int spineIndex, int& outStartSpine, int& outEndSpine) const;
  size_t getCumulativeSpineItemSize(int spineIndex) const;
  size_t getSpineItemSize(int spineIndex) const;
  int getSpineIndexForTextReference() const;

  static constexpr uint16_t kUnknownSpinePageCount = 0xFFFF;
  std::optional<uint16_t> getCachedSpinePageCount(const ReaderRenderSpec& spec, int spineIndex) const;
  void setCachedSpinePageCount(const ReaderRenderSpec& spec, int spineIndex, uint16_t pageCount,
                               bool flushToDisk = true) const;
  bool savePageCountCache(const ReaderRenderSpec& spec) const;
  void invalidatePageCountCache() const;

  size_t getBookSize() const;
  float calculateProgress(int currentSpineIndex, float currentSpineRead) const;
  float calculateSpanProgress(int startSpine, int endSpine, float spanRead) const;
  CssParser* getCssParser() const { return cssParser.get(); }
  int resolveHrefToSpineIndex(const std::string& href) const;
};
