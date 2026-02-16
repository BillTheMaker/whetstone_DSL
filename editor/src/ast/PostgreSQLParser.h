#pragma once

#include "ASTNode.h"
#include "Module.h"
#include "Function.h"
#include "SqlNodes.h"
#include "../ast/Parser.h"

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

class PostgreSQLParser {
public:
    static std::unique_ptr<Module> parsePostgreSQL(const std::string& source) {
        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_postgresql_module";
        module->targetLanguage = "postgresql";
        parseStatements(source, module.get());
        return module;
    }

    static ParseResult parsePostgreSQLWithDiagnostics(const std::string& source) {
        ParseResult result;
        result.module = parsePostgreSQL(source);
        return result;
    }

private:
    static void parseStatements(const std::string& source, Module* module) {
        for (const auto& stmtRaw : splitStatements(source)) {
            std::string stmt = trim(stmtRaw);
            if (stmt.empty()) continue;
            std::string lower = toLower(stmt);

            if (startsWith(lower, "create table ")) {
                parseCreateTable(stmt, module);
                continue;
            }
            if (startsWith(lower, "create index ") || startsWith(lower, "create unique index ")) {
                parseCreateIndex(stmt, module);
                continue;
            }
            if (startsWith(lower, "select ")) {
                parseSelect(stmt, module);
                continue;
            }
            if (startsWith(lower, "insert into ")) {
                parseInsert(stmt, module);
                continue;
            }
            if (startsWith(lower, "update ")) {
                parseUpdate(stmt, module);
                continue;
            }
            if (startsWith(lower, "delete from ")) {
                parseDelete(stmt, module);
                continue;
            }
            if (startsWith(lower, "do $$") || startsWith(lower, "create function ")) {
                auto* fn = new Function();
                fn->id = IdGenerator::next("fn");
                if (startsWith(lower, "do $$")) {
                    fn->name = "do_block";
                } else {
                    std::string raw = extractAfterKeyword(stmt, "FUNCTION ");
                    fn->name = trim(untilDelimiter(raw, "("));
                }
                module->addChild("functions", fn);
                continue;
            }
        }
    }

    static void parseCreateTable(const std::string& stmt, Module* module) {
        std::string tableName = extractAfterKeyword(stmt, "TABLE ");
        tableName = trim(untilDelimiter(tableName, "("));
        auto* table = new TableDeclaration();
        table->id = IdGenerator::next("table");
        table->name = tableName;

        auto dotPos = tableName.find('.');
        if (dotPos != std::string::npos) {
            table->schema = tableName.substr(0, dotPos);
        }

        auto lp = stmt.find('(');
        auto rp = stmt.rfind(')');
        if (lp != std::string::npos && rp != std::string::npos && rp > lp) {
            std::string cols = stmt.substr(lp + 1, rp - lp - 1);
            std::stringstream ss(cols);
            std::string item;
            while (std::getline(ss, item, ',')) {
                std::string colLine = trim(item);
                if (colLine.empty()) continue;
                if (startsWith(toLower(colLine), "constraint ") ||
                    startsWith(toLower(colLine), "primary key") ||
                    startsWith(toLower(colLine), "foreign key")) continue;

                std::vector<std::string> parts = splitWhitespace(colLine);
                if (parts.size() < 2) continue;
                auto* col = new ColumnDefinition();
                col->id = IdGenerator::next("col");
                col->name = parts[0];
                col->dataType = parts[1];
                col->nullable = (toLower(colLine).find("not null") == std::string::npos);
                table->addChild("columns", col);
            }
        }
        module->addChild("statements", table);
    }

    static void parseCreateIndex(const std::string& stmt, Module* module) {
        std::string lower = toLower(stmt);
        bool unique = startsWith(lower, "create unique index ");
        std::string idxName = unique ? extractAfterKeyword(stmt, "INDEX ") : extractAfterKeyword(stmt, "INDEX ");
        std::string tableName = extractAfterKeyword(stmt, "ON ");

        auto* idx = new IndexDefinition();
        idx->id = IdGenerator::next("idx");
        idx->name = trim(untilDelimiter(idxName, " ON"));
        idx->tableName = trim(untilDelimiter(tableName, "("));
        idx->unique = unique;
        module->addChild("statements", idx);
    }

    static void parseSelect(const std::string& stmt, Module* module) {
        std::string lower = toLower(stmt);
        auto* q = new SelectQuery();
        q->id = IdGenerator::next("sel");
        q->distinct = (lower.find("select distinct") == 0);

        auto fromPos = lower.find(" from ");
        if (fromPos != std::string::npos) {
            std::string afterFrom = trim(stmt.substr(fromPos + 6));
            std::string fromName = firstToken(afterFrom);
            auto* from = new TableDeclaration();
            from->id = IdGenerator::next("tbl");
            from->name = fromName;
            q->setChild("from", from);
        }

        size_t search = 0;
        while (true) {
            auto joinPos = lower.find(" join ", search);
            if (joinPos == std::string::npos) break;
            auto tableStart = joinPos + 6;
            std::string joinRest = stmt.substr(tableStart);
            std::string tableName = firstToken(joinRest);

            auto* j = new JoinClause();
            j->id = IdGenerator::next("join");
            j->joinType = detectJoinType(lower, joinPos);
            j->tableName = tableName;
            q->addChild("joins", j);
            search = tableStart + 1;
        }

        auto wherePos = lower.find(" where ");
        if (wherePos != std::string::npos) {
            auto* w = new WhereClause();
            w->id = IdGenerator::next("where");
            w->expression = trim(stmt.substr(wherePos + 7));
            q->setChild("where", w);
        }
        module->addChild("statements", q);
    }

    static void parseInsert(const std::string& stmt, Module* module) {
        auto* ins = new InsertStatement();
        ins->id = IdGenerator::next("ins");
        ins->tableName = extractAfterKeyword(stmt, "INTO ");
        ins->tableName = trim(untilDelimiter(ins->tableName, "("));
        module->addChild("statements", ins);
    }

    static void parseUpdate(const std::string& stmt, Module* module) {
        auto* up = new UpdateStatement();
        up->id = IdGenerator::next("upd");
        up->tableName = trim(untilDelimiter(extractAfterKeyword(stmt, "UPDATE "), " SET"));
        std::string lower = toLower(stmt);
        auto wherePos = lower.find(" where ");
        if (wherePos != std::string::npos) {
            auto* w = new WhereClause();
            w->id = IdGenerator::next("where");
            w->expression = trim(stmt.substr(wherePos + 7));
            up->setChild("where", w);
        }
        module->addChild("statements", up);
    }

    static void parseDelete(const std::string& stmt, Module* module) {
        auto* del = new DeleteStatement();
        del->id = IdGenerator::next("del");
        del->tableName = trim(untilDelimiter(extractAfterKeyword(stmt, "FROM "), " WHERE"));
        std::string lower = toLower(stmt);
        auto wherePos = lower.find(" where ");
        if (wherePos != std::string::npos) {
            auto* w = new WhereClause();
            w->id = IdGenerator::next("where");
            w->expression = trim(stmt.substr(wherePos + 7));
            del->setChild("where", w);
        }
        module->addChild("statements", del);
    }

    static std::string detectJoinType(const std::string& lower, size_t joinPos) {
        std::string prefix = lower.substr((joinPos > 8 ? joinPos - 8 : 0), 8);
        if (prefix.find("left") != std::string::npos) return "LEFT";
        if (prefix.find("right") != std::string::npos) return "RIGHT";
        if (prefix.find("full") != std::string::npos) return "FULL";
        return "INNER";
    }

    static std::vector<std::string> splitStatements(const std::string& source) {
        std::vector<std::string> out;
        std::string cur;
        int dollarDepth = 0;
        for (size_t i = 0; i < source.size(); ++i) {
            if (i + 1 < source.size() && source[i] == '$' && source[i + 1] == '$') {
                dollarDepth = 1 - dollarDepth;
                cur += "$$";
                ++i;
                continue;
            }
            if (source[i] == ';' && dollarDepth == 0) {
                out.push_back(cur);
                cur.clear();
                continue;
            }
            cur.push_back(source[i]);
        }
        if (!trim(cur).empty()) out.push_back(cur);
        return out;
    }

    static std::string extractAfterKeyword(const std::string& s, const std::string& keywordUpper) {
        std::string lower = toLower(s);
        std::string kw = toLower(keywordUpper);
        auto pos = lower.find(kw);
        if (pos == std::string::npos) return "";
        pos += kw.size();
        return trim(s.substr(pos));
    }

    static std::string firstToken(const std::string& s) {
        std::string t = trim(s);
        auto end = t.find_first_of(" \t\r\n");
        if (end == std::string::npos) return t;
        return t.substr(0, end);
    }

    static std::string untilDelimiter(const std::string& s, const std::string& delimiterUpper) {
        std::string lower = toLower(s);
        std::string d = toLower(delimiterUpper);
        auto pos = lower.find(d);
        if (pos == std::string::npos) return s;
        return s.substr(0, pos);
    }

    static std::vector<std::string> splitWhitespace(const std::string& s) {
        std::vector<std::string> out;
        std::stringstream ss(s);
        std::string token;
        while (ss >> token) out.push_back(token);
        return out;
    }

    static bool startsWith(const std::string& s, const std::string& prefix) {
        return s.rfind(prefix, 0) == 0;
    }

    static std::string toLower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return out;
    }

    static std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        auto end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }
};
