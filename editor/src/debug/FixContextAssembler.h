#pragma once
// Step 1451: fix-context assembler with budget modes.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct FixContextSlice {
    std::string path;
    int startLine = 1;
    int endLine = 1;
    std::string text;
};

struct FixContextPacket {
    std::string mode;
    int budgetChars = 0;
    int totalChars = 0;
    std::vector<FixContextSlice> slices;
};

class FixContextAssembler {
public:
    static int budgetForMode(const std::string& mode) {
        if (mode == "tiny") return 2000;
        if (mode == "small") return 6000;
        return 12000;
    }

    static FixContextPacket assemble(const std::string& mode,
                                     const std::vector<FixContextSlice>& input) {
        FixContextPacket out;
        out.mode = mode;
        out.budgetChars = budgetForMode(mode);

        std::vector<FixContextSlice> sorted = input;
        std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
            if (a.path != b.path) return a.path < b.path;
            return a.startLine < b.startLine;
        });

        for (const auto& s : sorted) {
            if (out.totalChars >= out.budgetChars) break;
            FixContextSlice keep = s;
            int remaining = out.budgetChars - out.totalChars;
            if (static_cast<int>(keep.text.size()) > remaining) keep.text = keep.text.substr(0, remaining);
            out.totalChars += static_cast<int>(keep.text.size());
            out.slices.push_back(std::move(keep));
        }
        return out;
    }

    static nlohmann::json toJson(const FixContextPacket& p) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& s : p.slices) {
            arr.push_back({{"path", s.path}, {"start_line", s.startLine}, {"end_line", s.endLine}, {"text", s.text}});
        }
        return {{"mode", p.mode}, {"budget_chars", p.budgetChars}, {"total_chars", p.totalChars}, {"slices", arr}};
    }
};
