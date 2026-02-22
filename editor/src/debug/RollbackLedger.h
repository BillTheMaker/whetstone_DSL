#pragma once
// Step 1480: rollback ledger model.

#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "PatchExecutionRecord.h"

class RollbackLedger {
public:
    static bool append(const std::string& path, const PatchExecutionRecord& rec) {
        nlohmann::json arr = nlohmann::json::array();
        std::ifstream in(path);
        if (in.good()) in >> arr;
        arr.push_back(PatchExecutionRecordModel::toJson(rec));
        std::ofstream out(path);
        if (!out.good()) return false;
        out << arr.dump(2);
        return true;
    }

    static nlohmann::json readAll(const std::string& path) {
        std::ifstream in(path);
        if (!in.good()) return nlohmann::json::array();
        nlohmann::json arr;
        in >> arr;
        return arr.is_array() ? arr : nlohmann::json::array();
    }

    static nlohmann::json last(const std::string& path) {
        auto arr = readAll(path);
        if (arr.empty()) return nlohmann::json::object();
        return arr.back();
    }
};
