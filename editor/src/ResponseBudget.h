#pragma once
// Step 254: Response Budget System
//
// Lets agents request only what they need. Query methods gain an optional
// `budget` parameter (max response chars). If exceeded, the response is
// truncated with `truncated: true` and a `continuation` token for follow-up.

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// -----------------------------------------------------------------------
//  Budget application: truncate a JSON result if it exceeds the budget
// -----------------------------------------------------------------------
struct BudgetResult {
    json       result;        // the (possibly truncated) response
    bool       truncated = false;
    std::string continuation; // opaque token for follow-up
};

// Apply budget to a result JSON. If the serialized size exceeds
// maxChars, tries to trim array fields to fit.
inline BudgetResult applyBudget(const json& fullResult, int maxChars) {
    BudgetResult br;

    if (maxChars <= 0) {
        // No budget → return as-is
        br.result = fullResult;
        return br;
    }

    std::string dump = fullResult.dump();
    if ((int)dump.size() <= maxChars) {
        // Fits within budget
        br.result = fullResult;
        return br;
    }

    // Need to truncate. Strategy: find the largest array in the result
    // and progressively shrink it until we fit.
    br.result = fullResult;
    br.truncated = true;

    // Find the array field to truncate
    std::string arrayField;
    int maxArraySize = 0;
    for (auto& [key, val] : br.result.items()) {
        if (val.is_array() && (int)val.size() > maxArraySize) {
            arrayField = key;
            maxArraySize = (int)val.size();
        }
    }

    if (arrayField.empty() || maxArraySize == 0) {
        // No array to truncate — mark as truncated but return as-is
        br.result["truncated"] = true;
        br.continuation = "none";
        return br;
    }

    // Binary search for the right number of elements
    int lo = 0, hi = maxArraySize;
    int bestFit = 0;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        json trial = br.result;
        json sliced = json::array();
        for (int i = 0; i < mid && i < maxArraySize; ++i)
            sliced.push_back(trial[arrayField][i]);
        trial[arrayField] = sliced;
        trial["truncated"] = true;
        trial["totalCount"] = maxArraySize;
        trial["returnedCount"] = mid;
        if ((int)trial.dump().size() <= maxChars) {
            bestFit = mid;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    // Build the truncated result
    json sliced = json::array();
    for (int i = 0; i < bestFit && i < maxArraySize; ++i)
        sliced.push_back(br.result[arrayField][i]);
    br.result[arrayField] = sliced;
    br.result["truncated"] = true;
    br.result["totalCount"] = maxArraySize;
    br.result["returnedCount"] = bestFit;

    // Continuation token: "field:offset:total"
    br.continuation = arrayField + ":" +
                      std::to_string(bestFit) + ":" +
                      std::to_string(maxArraySize);
    br.result["continuation"] = br.continuation;

    return br;
}

// -----------------------------------------------------------------------
//  Continue from a previous truncated response
// -----------------------------------------------------------------------
inline json applyContinuation(const json& fullResult,
                               const std::string& continuation,
                               int maxChars) {
    // Parse continuation token: "field:offset:total"
    auto pos1 = continuation.find(':');
    auto pos2 = continuation.find(':', pos1 + 1);
    if (pos1 == std::string::npos || pos2 == std::string::npos)
        return {{"error", "Invalid continuation token"}};

    std::string field = continuation.substr(0, pos1);
    int offset = std::stoi(continuation.substr(pos1 + 1, pos2 - pos1 - 1));
    int total = std::stoi(continuation.substr(pos2 + 1));

    if (!fullResult.contains(field) || !fullResult[field].is_array())
        return {{"error", "Continuation field not found"}};

    int arraySize = (int)fullResult[field].size();
    json result = fullResult;

    // Slice from offset
    json sliced = json::array();
    int count = 0;
    for (int i = offset; i < arraySize; ++i) {
        sliced.push_back(fullResult[field][i]);
        ++count;
        // Check if we're approaching the budget
        json trial = result;
        trial[field] = sliced;
        trial["returnedCount"] = count;
        trial["offset"] = offset;
        if (maxChars > 0 && (int)trial.dump().size() > maxChars)  {
            sliced.erase(sliced.size() - 1); // remove last
            --count;
            break;
        }
    }

    result[field] = sliced;
    result["returnedCount"] = count;
    result["offset"] = offset;
    result["totalCount"] = total;

    if (offset + count < arraySize) {
        result["truncated"] = true;
        result["continuation"] = field + ":" +
            std::to_string(offset + count) + ":" +
            std::to_string(total);
    } else {
        result["truncated"] = false;
    }

    return result;
}

// -----------------------------------------------------------------------
//  Priority-sort diagnostics (highest severity first) for budget
// -----------------------------------------------------------------------
inline void sortDiagnosticsByPriority(json& diagArray) {
    if (!diagArray.is_array()) return;
    std::sort(diagArray.begin(), diagArray.end(),
              [](const json& a, const json& b) {
                  return a.value("severityLevel", 4) <
                         b.value("severityLevel", 4);
              });
}
