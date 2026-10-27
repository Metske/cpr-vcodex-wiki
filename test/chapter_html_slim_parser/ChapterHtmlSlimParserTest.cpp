#include <Epub/Page.h>
#include <GfxRenderer.h>
#include <HalStorage.h>
#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#define class struct
#define private public
#include "Epub/parsers/ChapterHtmlSlimParser.h"
#undef private
#undef class

namespace {

class ChapterHtmlSlimParserTest : public ::testing::TestWithParam<const char*> {
 protected:
  std::string filepath = "unused.xhtml";
  GfxRenderer renderer;
  CssParser cssParser{"/tmp"};
  ChapterHtmlSlimParser parser{nullptr,
                               filepath,
                               renderer,
                               0,
                               1.0f,
                               false,
                               false,  // forceParagraphIndents (fork)
                               0,
                               static_cast<uint16_t>(renderer.getScreenWidth()),
                               static_cast<uint16_t>(renderer.getScreenHeight()),
                               false,
                               false,
                               {},
                               true,
                               "",
                               "",
                               0,
                               {},
                               nullptr,
                               &cssParser};

  void SetUp() override { parser.currentTextBlock = std::make_unique<ParsedText>(false); }
};

TEST_P(ChapterHtmlSlimParserTest, KeepsCssVerticalAlignAndInternalLinkMetadata) {
  const char* verticalAlign = GetParam();
  const char* expectedHref = "#note-target";
  const XML_Char* attributes[] = {"href", expectedHref, "style", verticalAlign, nullptr};

  ChapterHtmlSlimParser::startElement(&parser, "a", attributes);
  const uint8_t linkId = parser.currentFootnoteLinkId;
  ASSERT_NE(linkId, 0u);
  ChapterHtmlSlimParser::characterData(&parser, "1", 1);
  ChapterHtmlSlimParser::endElement(&parser, "a");

  ASSERT_EQ(parser.currentTextBlock->size(), 1u);
  const auto style = parser.currentTextBlock->getWordStyleAt(0);
  const auto expectedStyle =
      std::string(verticalAlign).find("super") != std::string::npos ? EpdFontFamily::SUP : EpdFontFamily::SUB;
  EXPECT_NE(static_cast<uint8_t>(style) & static_cast<uint8_t>(expectedStyle), 0u);

  ASSERT_EQ(parser.pendingFootnotes.size(), 1u);
  const FootnoteEntry& footnote = parser.pendingFootnotes.front().second;
  EXPECT_STREQ(footnote.href, expectedHref);
  ASSERT_EQ(parser.currentTextBlock->wordLinkIds.size(), 1u);
  EXPECT_EQ(parser.currentTextBlock->wordLinkIds.front(), linkId);
  EXPECT_TRUE(parser.currentTextBlock->linkTargetMatches(linkId, expectedHref));
}

INSTANTIATE_TEST_SUITE_P(CssVerticalAlign, ChapterHtmlSlimParserTest,
                         ::testing::Values("vertical-align: super", "vertical-align: sub"));

std::string buildMultiPageXhtml() {
  std::ostringstream out;
  out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
      << "<html xmlns=\"http://www.w3.org/1999/xhtml\"><head><title>Test</title></head><body>\n"
      << "<h1 id=\"sec-1\">Chapter One</h1>\n";

  // Multi-paragraph text with bold/italic, ruby, line breaks, and horizontal rules
  for (int p = 0; p < 18; ++p) {
    if (p == 6) {
      out << "<h2 id=\"sec-2\">Part Two</h2>\n";
    }
    if (p == 11) {
      out << "<hr/>\n";
    }
    out << "<p>";
    for (int w = 0; w < 45; ++w) {
      if (w == 10 && (p % 3 == 0)) {
        out << "<ruby><rb>Kanji</rb><rt>kanji</rt></ruby> ";
      } else if (w == 20 && (p % 4 == 0)) {
        out << "<b>boldword</b> <i>italicword</i><br/> ";
      } else {
        out << "word" << p << "_" << w << " ";
      }
    }
    out << "</p>\n";
  }

  // Also include a giant single-div block (> 350 words) to exercise mid-block flushing
  out << "<div id=\"sec-3\">";
  for (int w = 0; w < 380; ++w) {
    out << "longdivword_" << w << " ";
  }
  out << "</div>\n</body></html>\n";
  return out.str();
}

TEST(ChapterHtmlSlimParserCountOnlyTest, MatchesFullBuildPageCountAndAnchorsAcrossFileAndStream) {
  const std::string xhtml = buildMultiPageXhtml();
  const std::string tempPath = "/tmp/cpr_count_only_parity_test.xhtml";
  {
    std::ofstream f(tempPath, std::ios::binary | std::ios::trunc);
    ASSERT_TRUE(f.is_open());
    f.write(xhtml.data(), static_cast<std::streamsize>(xhtml.size()));
  }

  GfxRenderer renderer;
  CssParser cssParser{"/tmp"};
  const uint16_t viewportW = static_cast<uint16_t>(renderer.getScreenWidth());
  const uint16_t viewportH = static_cast<uint16_t>(renderer.getScreenHeight());

  // 1. Full layout pass via parseAndBuildPages()
  uint16_t fullBuildPages = 0;
  ChapterHtmlSlimParser fullParser(
      nullptr, tempPath, renderer, 0, 1.2f, true, true, 0, viewportW, viewportH, false, false,
      [&fullBuildPages](std::unique_ptr<Page> page, ChapterHtmlSlimParser::ParagraphLutEntry) {
        ASSERT_NE(page, nullptr);
        ++fullBuildPages;
      },
      true, "", "", 0, {}, nullptr, &cssParser);
  ASSERT_TRUE(fullParser.parseAndBuildPages());
  EXPECT_GT(fullBuildPages, 3u);
  EXPECT_EQ(fullParser.getCompletedPageCount(), fullBuildPages);

  // 2. File-based CountOnly pass via parseAndBuildPages()
  uint16_t countOnlyCallbackInvocations = 0;
  ChapterHtmlSlimParser fileCountParser(
      nullptr, tempPath, renderer, 0, 1.2f, true, true, 0, viewportW, viewportH, false, false,
      [&countOnlyCallbackInvocations](std::unique_ptr<Page>, ChapterHtmlSlimParser::ParagraphLutEntry) {
        ++countOnlyCallbackInvocations;
      },
      true, "", "", 0, {}, nullptr, &cssParser);
  fileCountParser.setCountOnly(true);
  ASSERT_TRUE(fileCountParser.parseAndBuildPages());
  EXPECT_EQ(countOnlyCallbackInvocations, 0u);
  EXPECT_EQ(fileCountParser.getCompletedPageCount(), fullBuildPages);
  EXPECT_EQ(fileCountParser.tocAnchors, fullParser.tocAnchors);

  // 3. Stream-based CountOnly pass with various chunk sizes (7, 137, 1024, 4096 bytes)
  for (size_t chunkSize : {size_t{7}, size_t{137}, size_t{1024}, size_t{4096}}) {
    ChapterHtmlSlimParser streamCountParser(
        nullptr, tempPath, renderer, 0, 1.2f, true, true, 0, viewportW, viewportH, false, false,
        {}, true, "", "", 0, {}, nullptr, &cssParser);
    streamCountParser.setCountOnly(true);
    ASSERT_TRUE(streamCountParser.beginStreamParse());

    size_t pos = 0;
    while (pos < xhtml.size()) {
      const size_t n = std::min(chunkSize, xhtml.size() - pos);
      ASSERT_TRUE(streamCountParser.feedStreamChunk(
          reinterpret_cast<const uint8_t*>(xhtml.data() + pos), n));
      pos += n;
    }
    ASSERT_TRUE(streamCountParser.finishStreamParse());
    EXPECT_EQ(streamCountParser.getCompletedPageCount(), fullBuildPages)
        << "Mismatch for stream chunkSize=" << chunkSize;
    EXPECT_EQ(streamCountParser.tocAnchors, fullParser.tocAnchors)
        << "Anchor mismatch for stream chunkSize=" << chunkSize;
  }

  std::remove(tempPath.c_str());
}

}  // namespace
