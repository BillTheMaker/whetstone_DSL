#pragma once
// Step 675: Extract minimal context slices from source files.

#include "WorkspaceFileIndex.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

struct SliceRequest {
    std::string filePath;
    std::string symbolName;
    int lineHint = 0;
    int headLines = 40;
};

struct ContextSlice {
    std::string filePath;
    std::string symbolName;
    int lineStart = 0;
    int lineEnd = 0;
    std::string content;
    bool found = false;
};

class ContextSliceAssembler {
public:
    static std::vector<ContextSlice> assemble(const std::vector<SliceRequest>& requests,
                                              const WorkspaceFileIndex& index) {
        std::vector<ContextSlice> out;
        out.reserve(requests.size());

        for (const auto& req : requests) {
            ContextSlice slice;
            slice.filePath = req.filePath;
            slice.symbolName = req.symbolName;

            std::vector<std::string> lines;
            if (!readFileLines(req.filePath, &lines)) {
                slice.found = false;
                out.push_back(std::move(slice));
                continue;
            }

            if (!req.symbolName.empty()) {
                auto symbols = index.findInFile(normalizePath(req.filePath));
                if (symbols.empty()) {
                    namespace fs = std::filesystem;
                    fs::path p(req.filePath);
                    symbols = index.findInFile(normalizePath(p.filename().generic_string()));
                }
                if (symbols.empty()) {
                    namespace fs = std::filesystem;
                    fs::path p(req.filePath);
                    if (p.is_absolute()) {
                        std::error_code ec;
                        const std::string rel = normalizePath(p.lexically_relative(fs::current_path(ec)).generic_string());
                        if (!rel.empty() && rel != p.generic_string()) symbols = index.findInFile(rel);
                        if (symbols.empty()) symbols = index.findInFile(normalizePath(p.filename().generic_string()));
                    }
                }
                const auto it = std::find_if(symbols.begin(), symbols.end(),
                                             [&](const SymbolLocation& s) {
                                                 return s.symbolName == req.symbolName;
                                             });
                if (it != symbols.end()) {
                    int lineStart = clampLine(it->lineStart, static_cast<int>(lines.size()));
                    int lineEnd = findDeclarationEnd(lines, lineStart);
                    fillSlice(lines, lineStart, lineEnd, true, &slice);
                    out.push_back(std::move(slice));
                    continue;
                }

                const int fallbackHead = req.headLines > 0 ? req.headLines : 40;
                fillSlice(lines, 1, std::min<int>(fallbackHead, static_cast<int>(lines.size())), false, &slice);
                out.push_back(std::move(slice));
                continue;
            }

            if (req.lineHint > 0) {
                const int start = clampLine(req.lineHint, static_cast<int>(lines.size()));
                const int span = req.headLines > 0 ? req.headLines : 1;
                const int end = std::min<int>(static_cast<int>(lines.size()), start + span - 1);
                fillSlice(lines, start, end, true, &slice);
                out.push_back(std::move(slice));
                continue;
            }

            const int head = req.headLines > 0 ? req.headLines : 40;
            fillSlice(lines, 1, std::min<int>(head, static_cast<int>(lines.size())), false, &slice);
            out.push_back(std::move(slice));
        }

        return out;
    }

private:
    static std::string normalizePath(const std::string& path) {
        std::string out = path;
        for (char& c : out) {
            if (c == '\\') c = '/';
        }
        return out;
    }

    static bool readFileLines(const std::string& filePath, std::vector<std::string>* out) {
        if (!out) return false;
        out->clear();

        namespace fs = std::filesystem;
        fs::path p(filePath);
        if (!fs::exists(p)) {
            p = fs::current_path() / p;
            if (!fs::exists(p)) return false;
        }

        std::ifstream in(p);
        if (!in.good()) return false;

        std::string line;
        while (std::getline(in, line)) {
            out->push_back(line);
        }
        return true;
    }

    static int clampLine(int line, int maxLine) {
        if (maxLine <= 0) return 0;
        if (line < 1) return 1;
        if (line > maxLine) return maxLine;
        return line;
    }

    static int findDeclarationEnd(const std::vector<std::string>& lines, int lineStart) {
        if (lineStart <= 0 || lineStart > static_cast<int>(lines.size())) return lineStart;

        int braceDepth = 0;
        bool seenBrace = false;
        for (int i = lineStart; i <= static_cast<int>(lines.size()); ++i) {
            const std::string& line = lines[static_cast<size_t>(i - 1)];
            for (char c : line) {
                if (c == '{') {
                    ++braceDepth;
                    seenBrace = true;
                } else if (c == '}') {
                    if (braceDepth > 0) --braceDepth;
                    if (seenBrace && braceDepth == 0) return i;
                }
            }
            if (!seenBrace && line.find(';') != std::string::npos) return i;
        }
        return static_cast<int>(lines.size());
    }

    static void fillSlice(const std::vector<std::string>& lines,
                          int lineStart,
                          int lineEnd,
                          bool found,
                          ContextSlice* out) {
        if (!out) return;
        out->lineStart = lineStart;
        out->lineEnd = lineEnd;
        out->found = found;
        out->content.clear();
        if (lineStart <= 0 || lineEnd <= 0 || lineStart > lineEnd) return;

        for (int i = lineStart; i <= lineEnd && i <= static_cast<int>(lines.size()); ++i) {
            out->content += lines[static_cast<size_t>(i - 1)];
            if (i < lineEnd) out->content += "\n";
        }
    }
};
