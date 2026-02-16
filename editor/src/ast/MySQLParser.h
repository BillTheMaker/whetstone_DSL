#pragma once

#include "ASTNode.h"
#include "Module.h"
#include "Expression.h"
#include "Type.h"
#include "SqlNodes.h"
#include "../ast/Parser.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

class MySQLParser {
public:
    static std::unique_ptr<Module> parseMySQL(const std::string& source) {
        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_mysql_module";
        module->targetLanguage = "mysql";
        parseStatements(source, module.get());
        return module;
    }

    static ParseResult parseMySQLWithDiagnostics(const std::string& source) {
        ParseResult result;
        result.module = parseMySQL(source);
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
        }
    }

    static void parseCreateTable(const std::string& stmt, Module* module) {
        std::string tableName = extractAfterKeyword(stmt, "TABLE ");
        tableName = stripBackticks(trim(untilDelimiter(tableName, "(")));

        auto* table = new TableDeclaration();
        table->id = IdGenerator::next("table");
        table->name = tableName;

        auto lp = stmt.find('(');
        auto rp = stmt.rfind(')');
        if (lp != std::string::npos && rp != std::string::npos && rp > lp) {
            std::string cols = stmt.substr(lp + 1, rp - lp - 1);
            std::stringstream ss(cols);
            std::string item;
            while (std::getline(ss, item, ',')) {
                std::string colLine = trim(item);
                if (colLine.empty()) continue;
                std::string lowerCol = toLower(colLine);
                if (startsWith(lowerCol, "constraint ") ||
                    startsWith(lowerCol, "primary key") ||
                    startsWith(lowerCol, "foreign key") ||
                    startsWith(lowerCol, "key ") ||
                    startsWith(lowerCol, "unique key")) continue;

                std::vector<std::string> parts = splitWhitespace(colLine);
                if (parts.size() < 2) continue;

                auto* col = new ColumnDefinition();
                col->id = IdGenerator::next("col");
                col->name = stripBackticks(parts[0]);
                col->dataType = parts[1];
                if (parts.size() >= 3 &&
                    toLower(parts[2]).find("auto_increment") != std::string::npos) {
                    col->dataType += " AUTO_INCREMENT";
                }
                col->nullable = (lowerCol.find("not null") == std::string::npos);
                table->addChild("columns", col);
            }
        }

        std::string lowerStmt = toLower(stmt);
        auto enginePos = lowerStmt.find("engine=");
        if (enginePos != std::string::npos) {
            std::string engine = trim(stmt.substr(enginePos + 7));
            engine = untilDelimiter(engine, " ");
            engine = untilDelimiter(engine, ";");
            table->setChild("engine", new CustomType(IdGenerator::next("engine"), engine));
        }

        module->addChild("statements", table);
    }

    static void parseCreateIndex(const std::string& stmt, Module* module) {
        std::string lower = toLower(stmt);
        bool unique = startsWith(lower, "create unique index ");
        std::string idxName = extractAfterKeyword(stmt, "INDEX ");
        std::string tableName = extractAfterKeyword(stmt, "ON ");

        auto* idx = new IndexDefinition();
        idx->id = IdGenerator::next("idx");
        idx->name = stripBackticks(trim(untilDelimiter(idxName, " ON")));
        idx->tableName = stripBackticks(trim(untilDelimiter(tableName, "(")));
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
            std::string proj = trim(stmt.substr(6, fromPos - 6));
            parseSelectColumns(proj, q);

            std::string afterFrom = trim(stmt.substr(fromPos + 6));
            std::string fromName = stripBackticks(firstToken(afterFrom));
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
            std::string tableName = stripBackticks(firstToken(joinRest));

            auto* j = new JoinClause();
            j->id = IdGenerator::next("join");
            j->joinType = detectJoinType(lower, joinPos);
            j->tableName = tableName;
            q->addChild("joins", j);
            search = tableStart + 1;
        }

        auto wherePos = lower.find(" where ");
        if (wherePos != std::string::npos) {
            std::string expr = trim(stmt.substr(wherePos + 7));
            expr = untilDelimiter(expr, " LIMIT ");
            auto* w = new WhereClause();
            w->id = IdGenerator::next("where");
            w->expression = expr;
            q->setChild("where", w);
        }

        auto limitPos = lower.find(" limit ");
        if (limitPos != std::string::npos) {
            std::string limitRaw = trim(stmt.substr(limitPos + 7));
            limitRaw = untilDelimiter(limitRaw, ",");
            limitRaw = untilDelimiter(limitRaw, ";");
            if (!limitRaw.empty() &&
                std::all_of(limitRaw.begin(), limitRaw.end(), ::isdigit)) {
                q->setChild("limit",
                            new IntegerLiteral(IdGenerator::next("lit"), std::stoi(limitRaw)));
            }
        }

        module->addChild("statements", q);
    }

    static void parseInsert(const std::string& stmt, Module* module) {
        auto* ins = new InsertStatement();
        ins->id = IdGenerator::next("ins");
        ins->tableName = stripBackticks(extractAfterKeyword(stmt, "INTO "));
        ins->tableName = trim(untilDelimiter(ins->tableName, "("));
        module->addChild("statements", ins);
    }

    static void parseUpdate(const std::string& stmt, Module* module) {
        auto* up = new UpdateStatement();
        up->id = IdGenerator::next("upd");
        up->tableName = stripBackticks(trim(untilDelimiter(extractAfterKeyword(stmt, "UPDATE "), " SET")));
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
        del->tableName = stripBackticks(trim(untilDelimiter(extractAfterKeyword(stmt, "FROM "), " WHERE")));
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

    static void parseSelectColumns(const std::string& proj, SelectQuery* q) {
        std::stringstream ss(proj);
        std::string item;
        while (std::getline(ss, item, ',')) {
            std::string col = trim(item);
            if (col.empty() || col == "*") continue;
            std::string lower = toLower(col);
            if (lower.find("group_concat(") != std::string::npos) {
                auto* call = new FunctionCall();
                call->id = IdGenerator::next("call");
                call->functionName = "GROUP_CONCAT";
                q->addChild("columns", call);
            } else {
                auto* c = new ColumnDefinition();
                c->id = IdGenerator::next("col");
                c->name = stripBackticks(untilDelimiter(col, " "));
                c->dataType = "TEXT";
                q->addChild("columns", c);
            }
        }
    }

    static std::string detectJoinType(const std::string& lower, size_t joinPos) {
        std::string prefix = lower.substr((joinPos > 8 ? joinPos - 8 : 0), 8);
        if (prefix.find("left") != std::string::npos) return "LEFT";
        if (prefix.find("right") != std::string::npos) return "RIGHT";
        if (prefix.find("inner") != std::string::npos) return "INNER";
        return "JOIN";
    }

    static std::vector<std::string> splitStatements(const std::string& source) {
        std::vector<std::string> out;
        std::string cur;
        for (char c : source) {
            if (c == ';') {
                if (!trim(cur).empty()) out.push_back(cur);
                cur.clear();
                continue;
            }
            cur.push_back(c);
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

    static std::string stripBackticks(const std::string& s) {
        if (s.size() >= 2 && s.front() == '`' && s.back() == '`') {
            return s.substr(1, s.size() - 2);
        }
        return s;
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
