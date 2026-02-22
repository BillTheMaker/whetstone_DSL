#pragma once
// Step 851: Failure aggregation indexer.
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "graduation/FailureTaxonomy.h"

struct AggregationBucket {
    std::string code;
    int count = 0;
    std::vector<std::string> failureIds;
};

class FailureAggregationIndexer {
public:
    void index(const TaggedFailure& f) {
        buckets_[f.primaryCode].code = f.primaryCode;
        ++buckets_[f.primaryCode].count;
        buckets_[f.primaryCode].failureIds.push_back(f.failureId);
    }

    std::vector<AggregationBucket> buckets() const {
        std::vector<AggregationBucket> r;
        for (const auto& kv : buckets_) r.push_back(kv.second);
        return r;
    }

    int totalIndexed() const {
        int n = 0;
        for (const auto& kv : buckets_) n += kv.second.count;
        return n;
    }

    AggregationBucket topBucket() const {
        AggregationBucket best;
        for (const auto& kv : buckets_)
            if (kv.second.count > best.count) best = kv.second;
        return best;
    }

    static nlohmann::json toJson(const std::vector<AggregationBucket>& b) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& bk : b)
            arr.push_back({{"code", bk.code}, {"count", bk.count}});
        return arr;
    }

private:
    std::map<std::string, AggregationBucket> buckets_;
};
