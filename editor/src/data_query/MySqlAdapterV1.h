#pragma once
// Step 792: MySQL lowering/raising adapters.

#include "SqlCanonicalQueryIR.h"

class MySqlAdapterV1 {
public:
    static QueryLoweringPacket lower(const std::string& sql) { return SqlCanonicalQueryIR::lower(sql, "mysql"); }
    static QueryRaisingPacket raise(const std::string& ir, const std::string& profile) { return SqlCanonicalQueryIR::raise(ir, "mysql", profile); }
};
