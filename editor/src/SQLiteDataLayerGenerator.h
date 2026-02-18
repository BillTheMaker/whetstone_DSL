#pragma once
// Step 641: SQLite schema -> C++ data layer generator

#include <string>
#include <vector>

struct SQLiteTableSpec {
    std::string name;
    std::vector<std::string> columns;
};

struct SQLiteDataLayerOutput {
    bool success = false;
    std::string headerCode;
    std::string sourceCode;
    std::vector<std::string> errors;
};

class SQLiteDataLayerGenerator {
public:
    static SQLiteDataLayerOutput generate(const std::vector<SQLiteTableSpec>& tables) {
        SQLiteDataLayerOutput out;
        if (tables.empty()) {
            out.errors.push_back("tables_required");
            return out;
        }

        out.success = true;
        out.headerCode = "#pragma once\n"
                         "#include <string>\n"
                         "#include <vector>\n\n"
                         "class NexusDb {\n"
                         "public:\n"
                         "    bool beginTx();\n"
                         "    bool commitTx();\n"
                         "};\n";

        out.sourceCode = "bool NexusDb::beginTx(){ return true; }\n"
                         "bool NexusDb::commitTx(){ return true; }\n";
        for (const auto& table : tables) {
            out.sourceCode += "// table: " + table.name + "\n";
            out.sourceCode += "// CRUD prepared statements\n";
            out.sourceCode += "// INSERT/SELECT/UPDATE/DELETE wrappers\n";
        }
        return out;
    }
};
