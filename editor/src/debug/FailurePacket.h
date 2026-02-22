#pragma once
// Step 1449: deterministic failure packet schema + normalizer.

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct FailurePacket {
    std::string packetId;
    std::string failureClass = "unknown";
    std::string primaryFile;
    int primaryLine = 0;
    std::string primarySymbol;
    std::string normalizedMessage;
    std::string reproCommand;
    std::string firstFailingTest;
    double confidence = 0.0;
    std::string rawExcerpt;
};

class FailurePacketModel {
public:
    static std::string canonicalizePath(const std::string& path) {
        std::string p = path;
        const std::string home = "/home/bill/Documents/CLionProjects/whetstone_DSL/";
        auto pos = p.find(home);
        if (pos != std::string::npos) p = p.substr(pos + home.size());
        return p;
    }

    static std::string normalizeWhitespace(const std::string& in) {
        std::string out;
        bool ws = false;
        for (char c : in) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                ws = true;
            } else {
                if (ws && !out.empty()) out.push_back(' ');
                out.push_back(c);
                ws = false;
            }
        }
        return out;
    }

    static std::string stableTruncate(const std::string& in, size_t maxBytes) {
        if (in.size() <= maxBytes) return in;
        if (maxBytes < 16) return in.substr(0, maxBytes);
        return in.substr(0, maxBytes - 11) + "...[trunc]";
    }

    static std::string dedupLines(const std::string& in) {
        std::vector<std::string> lines;
        std::string cur;
        for (char c : in) {
            if (c == '\n') {
                lines.push_back(cur);
                cur.clear();
            } else cur.push_back(c);
        }
        if (!cur.empty()) lines.push_back(cur);

        std::vector<std::string> unique;
        for (const auto& l : lines) {
            if (l.empty()) continue;
            if (std::find(unique.begin(), unique.end(), l) == unique.end()) unique.push_back(l);
        }

        std::string out;
        for (size_t i = 0; i < unique.size(); ++i) {
            if (i) out.push_back('\n');
            out += unique[i];
        }
        return out;
    }

    static std::string classify(const std::string& text, int exitCode) {
        const std::string s = text;
        if (s.find("error:") != std::string::npos || s.find("undefined reference") != std::string::npos) return "compile_error";
        if (s.find("FAIL") != std::string::npos || s.find("Assertion") != std::string::npos) return "test_assertion";
        if (s.find("schema") != std::string::npos) return "schema_error";
        if (s.find("Unknown tool") != std::string::npos) return "tool_contract_error";
        if (exitCode != 0) return "runtime_error";
        return "unknown";
    }

    static void extractPrimaryLocation(const std::string& text,
                                       std::string* file,
                                       int* line,
                                       std::string* symbol) {
        if (file) *file = "";
        if (line) *line = 0;
        if (symbol) *symbol = "";

        std::string token;
        std::istringstream iss(text);
        while (iss >> token) {
            auto p1 = token.find(':');
            auto p2 = token.find(':', p1 == std::string::npos ? 0 : p1 + 1);
            if (p1 == std::string::npos || p2 == std::string::npos) continue;
            std::string f = canonicalizePath(token.substr(0, p1));
            std::string lineStr = token.substr(p1 + 1, p2 - p1 - 1);
            bool digits = !lineStr.empty();
            for (char c : lineStr) if (!std::isdigit(static_cast<unsigned char>(c))) digits = false;
            if (!digits) continue;
            if (file) *file = f;
            if (line) *line = std::stoi(lineStr);
            break;
        }

        auto pos = text.find("undefined symbol ");
        if (pos != std::string::npos && symbol) {
            std::string rem = text.substr(pos + 17);
            std::string s;
            for (char c : rem) {
                if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') s.push_back(c);
                else break;
            }
            *symbol = s;
        }
    }

    static std::string extractFirstFailingTest(const std::string& text) {
        std::istringstream iss(text);
        std::string line;
        while (std::getline(iss, line)) {
            auto p = line.find("... FAIL");
            if (p != std::string::npos) return normalizeWhitespace(line.substr(0, p));
        }
        return "";
    }

    static FailurePacket make(const std::string& command,
                              const std::string& rawOutput,
                              int exitCode,
                              size_t maxBytes = 8192) {
        FailurePacket p;
        p.reproCommand = command;
        p.failureClass = classify(rawOutput, exitCode);

        std::string deduped = dedupLines(rawOutput);
        std::string norm = normalizeWhitespace(deduped);
        p.normalizedMessage = stableTruncate(norm, maxBytes);
        p.rawExcerpt = stableTruncate(deduped, maxBytes);
        p.firstFailingTest = extractFirstFailingTest(rawOutput);
        extractPrimaryLocation(rawOutput, &p.primaryFile, &p.primaryLine, &p.primarySymbol);

        p.confidence = 0.5;
        if (p.failureClass == "compile_error" || p.failureClass == "test_assertion") p.confidence = 0.9;
        else if (p.failureClass == "runtime_error") p.confidence = 0.7;

        p.packetId = stableId(p);
        return p;
    }

    static nlohmann::json toJson(const FailurePacket& p) {
        return {
            {"packet_id", p.packetId},
            {"failure_class", p.failureClass},
            {"primary_file", p.primaryFile},
            {"primary_line", p.primaryLine},
            {"primary_symbol", p.primarySymbol},
            {"normalized_message", p.normalizedMessage},
            {"repro_command", p.reproCommand},
            {"first_failing_test", p.firstFailingTest},
            {"confidence", p.confidence},
            {"raw_excerpt", p.rawExcerpt}
        };
    }

    static FailurePacket fromJson(const nlohmann::json& j) {
        FailurePacket p;
        p.packetId = j.value("packet_id", "");
        p.failureClass = j.value("failure_class", "unknown");
        p.primaryFile = j.value("primary_file", "");
        p.primaryLine = j.value("primary_line", 0);
        p.primarySymbol = j.value("primary_symbol", "");
        p.normalizedMessage = j.value("normalized_message", "");
        p.reproCommand = j.value("repro_command", "");
        p.firstFailingTest = j.value("first_failing_test", "");
        p.confidence = j.value("confidence", 0.0);
        p.rawExcerpt = j.value("raw_excerpt", "");
        return p;
    }

private:
    static std::string stableId(const FailurePacket& p) {
        std::string key = p.failureClass + "|" + p.primaryFile + "|" + std::to_string(p.primaryLine) + "|" + p.normalizedMessage;
        uint64_t h = 1469598103934665603ull;
        for (unsigned char c : key) {
            h ^= c;
            h *= 1099511628211ull;
        }
        std::ostringstream oss;
        oss << "fp_" << std::hex << h;
        return oss.str();
    }
};
