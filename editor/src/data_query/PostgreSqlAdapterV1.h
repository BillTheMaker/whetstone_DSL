#pragma once
// Step 790: PostgreSQL lowering/raising adapters.

#include "SqlCanonicalQueryIR.h"

class PostgreSqlAdapterV1 {
public:
    static QueryLoweringPacket lower(const std::string& sql) { return SqlCanonicalQueryIR::lower(sql, "postgresql"); }
    static QueryRaisingPacket raise(const std::string& ir, const std::string& profile) { return SqlCanonicalQueryIR::raise(ir, "postgresql", profile); }
};
