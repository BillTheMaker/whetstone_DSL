#pragma once
// Step 793: Transaction/isolation semantics packet model.

#include <string>

#include <nlohmann/json.hpp>

struct TransactionIsolationPacket {
    bool transactionDetected = false;
    std::string isolationLevel;
    bool riskOnDowngrade = false;
};

class TransactionIsolationModel {
public:
    static TransactionIsolationPacket analyze(const std::string& sql, const std::string& targetIsolation) {
        TransactionIsolationPacket p;
        p.transactionDetected = (sql.find("BEGIN") != std::string::npos || sql.find("COMMIT") != std::string::npos || sql.find("TRANSACTION") != std::string::npos);
        if (sql.find("SERIALIZABLE") != std::string::npos) p.isolationLevel = "serializable";
        else if (sql.find("REPEATABLE READ") != std::string::npos) p.isolationLevel = "repeatable_read";
        else p.isolationLevel = "read_committed";
        p.riskOnDowngrade = (p.isolationLevel == "serializable" && targetIsolation != "serializable");
        return p;
    }

    static nlohmann::json toJson(const TransactionIsolationPacket& p) {
        return {{"transaction_detected", p.transactionDetected}, {"isolation_level", p.isolationLevel}, {"risk_on_downgrade", p.riskOnDowngrade}};
    }
};
