#pragma once
// Step 659: Entropy scanner - editor side

#include <set>
#include <string>
#include <vector>

struct EntropyObservation {
    int duplicateSignatures = 0;
    int similarModuleNames = 0;
    int unusedExports = 0;
};

class EntropyScannerModel {
public:
    static EntropyObservation scan(const std::vector<std::string>& signatures,
                                   const std::vector<std::string>& moduleNames,
                                   const std::vector<std::string>& exports,
                                   const std::set<std::string>& usedExports) {
        EntropyObservation o;
        o.duplicateSignatures = countDuplicates(signatures);
        o.similarModuleNames = countPrefixSimilar(moduleNames);
        o.unusedExports = countUnused(exports, usedExports);
        return o;
    }

    static int entropyScore(const EntropyObservation& o) {
        return o.duplicateSignatures + o.similarModuleNames + o.unusedExports;
    }

    static bool shouldSuggestRefactor(const EntropyObservation& o, int threshold = 3) {
        return entropyScore(o) >= threshold;
    }

private:
    static int countDuplicates(const std::vector<std::string>& values) {
        std::set<std::string> seen;
        int duplicates = 0;
        for (const auto& v : values) {
            if (!seen.insert(v).second) ++duplicates;
        }
        return duplicates;
    }

    static int countPrefixSimilar(const std::vector<std::string>& names) {
        int similar = 0;
        for (std::size_t i = 0; i < names.size(); ++i) {
            for (std::size_t j = i + 1; j < names.size(); ++j) {
                if (names[i].size() >= 3 && names[j].rfind(names[i].substr(0, 3), 0) == 0) {
                    ++similar;
                }
            }
        }
        return similar;
    }

    static int countUnused(const std::vector<std::string>& exports,
                           const std::set<std::string>& used) {
        int unused = 0;
        for (const auto& e : exports) {
            if (used.find(e) == used.end()) ++unused;
        }
        return unused;
    }
};
