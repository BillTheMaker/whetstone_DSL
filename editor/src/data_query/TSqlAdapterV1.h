#pragma once
// Step 791: T-SQL lowering/raising adapters.

#include "SqlCanonicalQueryIR.h"

class TSqlAdapterV1 {
public:
    static QueryLoweringPacket lower(const std::string& sql) { return SqlCanonicalQueryIR::lower(sql, "tsql"); }
    static QueryRaisingPacket raise(const std::string& ir, const std::string& profile) { return SqlCanonicalQueryIR::raise(ir, "tsql", profile); }
};
