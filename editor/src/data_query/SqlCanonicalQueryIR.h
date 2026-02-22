#pragma once
// Step 789: SQL canonical query IR layer.

#include <string>

#include <nlohmann/json.hpp>

struct QueryLoweringPacket {
    std::string sourceDialect;
    std::string irSummary;
    bool hasJoin = false;
    bool hasAggregate = false;
    bool hasNullSemantics = false;
};

struct QueryRaisingPacket {
    std::string targetDialect;
    std::string queryPreview;
    std::string profile;
};

class SqlCanonicalQueryIR {
public:
    static QueryLoweringPacket lower(const std::string& sql, const std::string& dialect) {
        QueryLoweringPacket p;
        p.sourceDialect = dialect;
        p.irSummary = sql.empty() ? "empty_query" : "sql_query_ir_v1";
        p.hasJoin = (sql.find("JOIN") != std::string::npos || sql.find("join") != std::string::npos);
        p.hasAggregate = (sql.find("COUNT") != std::string::npos || sql.find("SUM") != std::string::npos || sql.find("GROUP BY") != std::string::npos);
        p.hasNullSemantics = (sql.find("NULL") != std::string::npos || sql.find("IS NULL") != std::string::npos);
        return p;
    }

    static QueryRaisingPacket raise(const std::string& ir, const std::string& dialect, const std::string& profile) {
        QueryRaisingPacket p;
        p.targetDialect = dialect;
        p.queryPreview = "-- " + dialect + " raised from " + ir;
        p.profile = profile.empty() ? "safe" : profile;
        return p;
    }

    static nlohmann::json toJson(const QueryLoweringPacket& p) {
        return {{"source_dialect", p.sourceDialect}, {"ir_summary", p.irSummary}, {"has_join", p.hasJoin}, {"has_aggregate", p.hasAggregate}, {"has_null_semantics", p.hasNullSemantics}};
    }

    static nlohmann::json toJson(const QueryRaisingPacket& p) {
        return {{"target_dialect", p.targetDialect}, {"query_preview", p.queryPreview}, {"profile", p.profile}};
    }
};
