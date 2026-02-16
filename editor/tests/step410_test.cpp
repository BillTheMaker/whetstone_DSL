// Step 410: SQL AST Nodes (12 tests)
// Tests construction + serialization roundtrip for SQL-specific node types.

#include <cassert>
#include <iostream>

#include "ast/SqlNodes.h"
#include "ast/Serialization.h"
#include "CompactAST.h"

int main() {
    int passed = 0;

    // 1. TableDeclaration construction
    {
        auto* t = new TableDeclaration("t1", "users", "public");
        assert(t->conceptType == "TableDeclaration");
        assert(t->name == "users");
        assert(t->schema == "public");
        delete t;
        std::cout << "Test 1 PASSED: TableDeclaration construction\n";
        passed++;
    }

    // 2. ColumnDefinition construction
    {
        auto* c = new ColumnDefinition("c1", "id", "INTEGER", false);
        c->defaultValue = "0";
        assert(c->conceptType == "ColumnDefinition");
        assert(c->name == "id");
        assert(c->dataType == "INTEGER");
        assert(!c->nullable);
        assert(c->defaultValue == "0");
        delete c;
        std::cout << "Test 2 PASSED: ColumnDefinition construction\n";
        passed++;
    }

    // 3. SelectQuery + clauses children
    {
        auto* q = new SelectQuery();
        q->id = "q1";
        q->distinct = true;
        q->addChild("joins", new JoinClause("j1", "INNER", "orders"));
        q->setChild("where", new WhereClause("w1", "id > 10"));
        assert(q->distinct);
        assert(q->getChildren("joins").size() == 1);
        assert(q->getChild("where") != nullptr);
        delete q;
        std::cout << "Test 3 PASSED: SelectQuery with clauses\n";
        passed++;
    }

    // 4. InsertStatement construction
    {
        auto* ins = new InsertStatement("i1", "users");
        ins->addChild("columns", new ColumnDefinition("c2", "name", "TEXT"));
        assert(ins->conceptType == "InsertStatement");
        assert(ins->tableName == "users");
        assert(ins->getChildren("columns").size() == 1);
        delete ins;
        std::cout << "Test 4 PASSED: InsertStatement construction\n";
        passed++;
    }

    // 5. UpdateStatement construction
    {
        auto* up = new UpdateStatement("u1", "users");
        up->setChild("where", new WhereClause("w2", "id = 1"));
        assert(up->conceptType == "UpdateStatement");
        assert(up->tableName == "users");
        assert(up->getChild("where") != nullptr);
        delete up;
        std::cout << "Test 5 PASSED: UpdateStatement construction\n";
        passed++;
    }

    // 6. DeleteStatement construction
    {
        auto* del = new DeleteStatement("d1", "logs");
        del->setChild("where", new WhereClause("w3", "created_at < now()"));
        assert(del->conceptType == "DeleteStatement");
        assert(del->tableName == "logs");
        assert(del->getChild("where") != nullptr);
        delete del;
        std::cout << "Test 6 PASSED: DeleteStatement construction\n";
        passed++;
    }

    // 7. IndexDefinition construction
    {
        auto* idx = new IndexDefinition("x1", "idx_users_email", "users", true);
        idx->addChild("columns", new ColumnDefinition("c3", "email", "TEXT"));
        assert(idx->conceptType == "IndexDefinition");
        assert(idx->name == "idx_users_email");
        assert(idx->tableName == "users");
        assert(idx->unique);
        delete idx;
        std::cout << "Test 7 PASSED: IndexDefinition construction\n";
        passed++;
    }

    // 8. Table with columns + index JSON roundtrip
    {
        auto* t = new TableDeclaration("t2", "accounts", "public");
        t->addChild("columns", new ColumnDefinition("c4", "id", "BIGINT", false));
        t->addChild("columns", new ColumnDefinition("c5", "email", "TEXT", false));
        t->addChild("indexes", new IndexDefinition("x2", "idx_accounts_email", "accounts", true));
        json j = toJson(t);
        auto* r = fromJson(j);
        assert(r->conceptType == "TableDeclaration");
        auto* rt = static_cast<TableDeclaration*>(r);
        assert(rt->name == "accounts");
        assert(rt->getChildren("columns").size() == 2);
        assert(rt->getChildren("indexes").size() == 1);
        delete t;
        delete r;
        std::cout << "Test 8 PASSED: table JSON roundtrip\n";
        passed++;
    }

    // 9. SelectQuery JSON roundtrip
    {
        auto* q = new SelectQuery();
        q->id = "q2";
        q->distinct = true;
        q->addChild("joins", new JoinClause("j2", "LEFT", "profiles"));
        q->setChild("where", new WhereClause("w4", "active = true"));
        json j = toJson(q);
        auto* r = fromJson(j);
        assert(r->conceptType == "SelectQuery");
        auto* rq = static_cast<SelectQuery*>(r);
        assert(rq->distinct);
        assert(rq->getChildren("joins").size() == 1);
        assert(rq->getChild("where") != nullptr);
        delete q;
        delete r;
        std::cout << "Test 9 PASSED: select JSON roundtrip\n";
        passed++;
    }

    // 10. CompactAST names
    {
        auto* t = new TableDeclaration("t3", "sessions");
        auto* c = new ColumnDefinition("c6", "token", "TEXT");
        auto* q = new SelectQuery();
        q->id = "q3";
        auto* d = new DeleteStatement("d2", "sessions");
        auto* w = new WhereClause("w5", "expired = true");
        auto* x = new IndexDefinition("x3", "idx_sessions_token", "sessions");
        assert(getNodeName(t) == "sessions");
        assert(getNodeName(c) == "token");
        assert(getNodeName(q) == "select");
        assert(getNodeName(d) == "sessions");
        assert(getNodeName(w) == "where");
        assert(getNodeName(x) == "idx_sessions_token");
        delete t; delete c; delete q; delete d; delete w; delete x;
        std::cout << "Test 10 PASSED: CompactAST SQL names\n";
        passed++;
    }

    // 11. createNode dispatch for SQL concepts
    {
        ASTNode* t = createNode("TableDeclaration");
        ASTNode* c = createNode("ColumnDefinition");
        ASTNode* s = createNode("SelectQuery");
        ASTNode* i = createNode("InsertStatement");
        ASTNode* u = createNode("UpdateStatement");
        ASTNode* d = createNode("DeleteStatement");
        ASTNode* j = createNode("JoinClause");
        ASTNode* w = createNode("WhereClause");
        ASTNode* x = createNode("IndexDefinition");
        assert(t && c && s && i && u && d && j && w && x);
        delete t; delete c; delete s; delete i; delete u; delete d; delete j; delete w; delete x;
        std::cout << "Test 11 PASSED: createNode SQL dispatch\n";
        passed++;
    }

    // 12. Full query graph roundtrip
    {
        auto* q = new SelectQuery();
        q->id = "q4";
        q->addChild("columns", new ColumnDefinition("c7", "id", "INTEGER"));
        q->setChild("from", new TableDeclaration("t4", "users"));
        q->addChild("joins", new JoinClause("j3", "INNER", "orders"));
        q->setChild("where", new WhereClause("w6", "users.id = orders.user_id"));
        json j = toJson(q);
        auto* r = fromJson(j);
        assert(r->conceptType == "SelectQuery");
        assert(r->getChildren("columns").size() == 1);
        assert(r->getChild("from") != nullptr);
        assert(r->getChildren("joins").size() == 1);
        assert(r->getChild("where") != nullptr);
        delete q;
        delete r;
        std::cout << "Test 12 PASSED: full query graph roundtrip\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
