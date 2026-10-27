#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "activities/wiki/WikiReaderActivity.h"
#include "util/SdRawReader.h"
#include "util/WikiSearchEngine.h"
#include "util/WikiDecompressor.h"
#include "util/HtmlToPlainText.h"

GUI_class GUI;
ActivityManager activityManager;

extern "C" {
uint32_t uzlib_adler32(const void *data, unsigned int length, uint32_t prev_sum) { return 0; }
uint32_t uzlib_crc32(const void *data, unsigned int length, uint32_t prev_sum) { return 0; }
}

inline char hexToChar(char first, char second) {
    int digit = (first >= '0' && first <= '9') ? (first - '0') :
                (first >= 'A' && first <= 'F') ? (first - 'A' + 10) :
                (first >= 'a' && first <= 'f') ? (first - 'a' + 10) : 0;
    int digit2 = (second >= '0' && second <= '9') ? (second - '0') :
                 (second >= 'A' && second <= 'F') ? (second - 'A' + 10) :
                 (second >= 'a' && second <= 'f') ? (second - 'a' + 10) : 0;
    return (digit << 4) | digit2;
}

int main() {
    SdRawReader reader;
    if (!reader.mount(-1, -1, -1, -1)) return 1;
    WikiSearchEngine engine(reader, 0); 
    
    std::vector<WikiSearchResult> test_articles;
    const char* queries[] = {"computer", "algorithm", "algol", "software", "network", "c++", "physics"};
    for (const char* q : queries) {
        WikiSearchResult results[20];
        int count = engine.searchPrefix(q, results, 20);
        for (int i = 0; i < std::min(5, count); ++i) test_articles.push_back(results[i]);
    }

    int totalLinks = 0;
    int failedExact = 0;
    int resolvedByPrefix = 0;
    int totalFailures = 0;

    for (const auto& article : test_articles) {
        WikiDecompressor decompressor(reader);
        if (!decompressor.beginAtByteOffset(0, article.offset)) continue;

        std::vector<ExtractedLink> rawLinks;
        std::string hrefPool;
        bool isEof = false;
        streamHtmlToPlainTextChunk(decompressor, 1000 * 1024, 0, 10000, &rawLinks, &hrefPool, isEof);

        for (const auto& rl : rawLinks) {
            std::string rawHref(&hrefPool[rl.hrefOffset], rl.hrefLen);
            std::string href = rawHref;
            
            if (href.find("./") == 0) href = href.substr(2);
            size_t hashPos = href.find('#');
            if (hashPos != std::string::npos) href = href.substr(0, hashPos);
            if (href.empty()) continue;

            for (char& c : href) if (c == '_') c = ' ';
            
            std::string decoded;
            for (size_t i = 0; i < href.length(); ++i) {
                if (href[i] == '%' && i + 2 < href.length()) {
                    decoded += hexToChar(href[i+1], href[i+2]);
                    i += 2;
                } else {
                    decoded += href[i];
                }
            }
            href = decoded;

            // Skip http/https entirely since they obviously fail
            if (href.find("http://") == 0 || href.find("https://") == 0) continue;

            totalLinks++;
            uint32_t foundOffset = 0;
            if (!engine.search(href.c_str(), foundOffset)) {
                failedExact++;
                WikiSearchResult prefixRes[1];
                int prefixCount = engine.searchPrefix(href.c_str(), prefixRes, 1);
                if (prefixCount > 0) {
                    resolvedByPrefix++;
                } else {
                    totalFailures++;
                }
            }
        }
    }

    std::cout << "[SIMULATOR WITH URL DECODE + PREFIX FALLBACK]" << std::endl;
    std::cout << "Total Internal Links: " << totalLinks << std::endl;
    std::cout << "Failed Exact Matches: " << failedExact << std::endl;
    std::cout << "Rescued by Prefix  : " << resolvedByPrefix << std::endl;
    std::cout << "Remaining Failures : " << totalFailures << " (Out-of-domain articles)" << std::endl;
    return 0;
}
