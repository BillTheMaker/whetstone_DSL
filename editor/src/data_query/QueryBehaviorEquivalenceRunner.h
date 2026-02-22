#pragma once
// Step 794: Query behavior equivalence runner (set-based diff checks).

#include <nlohmann/json.hpp>

struct QueryBehaviorEquivalenceResult {
    bool equivalent = false;
    int differingRows = 0;
};

class QueryBehaviorEquivalenceRunner {
public:
    static QueryBehaviorEquivalenceResult compare(const nlohmann::json& lhsRows, const nlohmann::json& rhsRows) {
        QueryBehaviorEquivalenceResult r;
        r.equivalent = lhsRows.dump() == rhsRows.dump();
        if (!r.equivalent) {
            int l = lhsRows.is_array() ? static_cast<int>(lhsRows.size()) : 0;
            int rr = rhsRows.is_array() ? static_cast<int>(rhsRows.size()) : 0;
            r.differingRows = l > rr ? (l - rr) : (rr - l);
            if (r.differingRows == 0) r.differingRows = 1;
        }
        return r;
    }

    static nlohmann::json toJson(const QueryBehaviorEquivalenceResult& r) {
        return {{"equivalent", r.equivalent}, {"differing_rows", r.differingRows}};
    }
};
