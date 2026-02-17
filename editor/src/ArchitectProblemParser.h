#pragma once

// Step 471: Problem Description Parser
// Heuristic structured requirement extraction from natural-language problem
// descriptions with per-field confidence scores.

#include <algorithm>
#include <map>
#include <string>
#include <vector>

struct ExtractedItem {
    std::string text;
    float confidence = 0.0f;
};

struct StructuredRequirements {
    std::vector<ExtractedItem> functionalRequirements;
    std::vector<ExtractedItem> nonFunctionalRequirements;
    std::vector<ExtractedItem> platformConstraints;
    std::vector<ExtractedItem> integrationRequirements;

    bool hasFunctionalKeyword(const std::string& keyword) const {
        for (const auto& r : functionalRequirements)
            if (toLower(r.text).find(toLower(keyword)) != std::string::npos) return true;
        return false;
    }

    bool hasPlatform(const std::string& keyword) const {
        for (const auto& r : platformConstraints)
            if (toLower(r.text).find(toLower(keyword)) != std::string::npos) return true;
        return false;
    }

private:
    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return s;
    }
};

class ArchitectProblemParser {
public:
    static StructuredRequirements parse(const std::string& description) {
        StructuredRequirements out;
        auto text = toLower(description);

        // Functional requirements
        addIfPresent(text, {"api", "rest", "endpoint", "crud", "dashboard", "auth", "login",
                            "signup", "search", "reporting", "notification", "upload"},
                     out.functionalRequirements, 0.86f, 0.96f);

        // Non-functional requirements
        addIfPresent(text, {"performance", "fast", "latency", "scalable", "scale",
                            "secure", "security", "reliable", "availability", "99.9",
                            "throughput", "compliance"},
                     out.nonFunctionalRequirements, 0.82f, 0.95f);

        // Platform constraints
        addIfPresent(text, {"web", "mobile", "ios", "android", "embedded", "server",
                            "backend", "frontend", "cli", "desktop"},
                     out.platformConstraints, 0.80f, 0.94f);

        // Integration requirements
        addIfPresent(text, {"postgres", "postgresql", "mysql", "sqlite", "redis", "kafka",
                            "api", "stripe", "s3", "salesforce", "ldap", "oauth", "jwt"},
                     out.integrationRequirements, 0.78f, 0.93f);

        // Fallback generic functional intent if nothing extracted
        if (out.functionalRequirements.empty()) {
            out.functionalRequirements.push_back({"core application behavior", 0.55f});
        }

        return out;
    }

private:
    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return s;
    }

    static void addIfPresent(const std::string& text,
                             const std::vector<std::string>& keywords,
                             std::vector<ExtractedItem>& bucket,
                             float lowConf,
                             float highConf) {
        for (const auto& k : keywords) {
            if (text.find(k) != std::string::npos) {
                float conf = confidenceForKeyword(k, lowConf, highConf);
                bucket.push_back({k, conf});
            }
        }
        dedupe(bucket);
    }

    static float confidenceForKeyword(const std::string& k, float lowConf, float highConf) {
        if (k.size() >= 7) return highConf;
        if (k.size() >= 4) return (lowConf + highConf) * 0.5f;
        return lowConf;
    }

    static void dedupe(std::vector<ExtractedItem>& items) {
        std::map<std::string, float> best;
        for (const auto& i : items) {
            auto it = best.find(i.text);
            if (it == best.end() || i.confidence > it->second) best[i.text] = i.confidence;
        }
        items.clear();
        for (const auto& kv : best) items.push_back({kv.first, kv.second});
    }
};
