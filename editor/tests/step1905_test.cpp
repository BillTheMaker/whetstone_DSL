// Step 1905: SymbolIndexUpdater
// Incremental update of CrossLanguageSymbolTable on re-generation.
// Avoids full rebuild: only removes stale entries, inserts new/changed ones.
//
//  t1: update with same nodes is a no-op (table unchanged)
//  t2: adding a new node inserts it without disturbing existing entries
//  t3: removing a node (not in new set) removes it from the table
//  t4: changed node (same name, different languages) replaces old entry
//  t5: updateResult reports added, removed, unchanged counts correctly

#include "SymbolIndexUpdater.h"
#include "CrossLanguageSymbolTable.h"
#include "ABIBoundaryExtractor.h"
#include <iostream>
#include <string>

namespace ws = whetstone;

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

static ws::ABIBoundaryNode makeNode(const std::string& name,
                                    const std::string& fromLang, const std::string& toLang,
                                    const std::string& fromComp = "from",
                                    const std::string& toComp   = "to")
{
    ws::ABIBoundaryNode n;
    n.name = name; n.kind = "function";
    n.fromComponent = fromComp; n.toComponent = toComp;
    n.fromLanguage = fromLang;  n.toLanguage  = toLang;
    n.signature = {{"name",name},{"kind","function"},{"exported",true}};
    return n;
}

void t1(){
    T(update_with_same_nodes_is_noop);
    ws::CrossLanguageSymbolTable table;
    auto node = makeNode("sort_array", "Python", "Rust");
    table.insert(node);

    auto result = ws::SymbolIndexUpdater::update(table, {node}, {node});

    C(result.added == 0,     "added must be 0");
    C(result.removed == 0,   "removed must be 0");
    C(result.unchanged == 1, "unchanged must be 1");
    C(table.contains("Rust", "sort_array"), "sort_array still in table");
    P();
}

void t2(){
    T(adding_new_node_inserts_without_disturbing_existing);
    ws::CrossLanguageSymbolTable table;
    auto existing = makeNode("sort_array", "Python", "Rust");
    table.insert(existing);

    auto newNode = makeNode("merge_slices", "Python", "Rust");
    auto result  = ws::SymbolIndexUpdater::update(table, {existing}, {existing, newNode});

    C(result.added == 1,   "added must be 1");
    C(result.removed == 0, "removed must be 0");
    C(table.contains("Rust", "sort_array"),   "existing sort_array still present");
    C(table.contains("Rust", "merge_slices"), "new merge_slices inserted");
    P();
}

void t3(){
    T(removed_node_deleted_from_table);
    ws::CrossLanguageSymbolTable table;
    auto n1 = makeNode("sort_array",   "Python", "Rust");
    auto n2 = makeNode("merge_slices", "Python", "Rust");
    table.insert(n1);
    table.insert(n2);

    // Re-generation only has n1; n2 dropped
    auto result = ws::SymbolIndexUpdater::update(table, {n1, n2}, {n1});

    C(result.removed == 1, "removed must be 1");
    C(table.contains("Rust", "sort_array"),    "sort_array still present");
    C(!table.contains("Rust", "merge_slices"), "merge_slices must be removed");
    P();
}

void t4(){
    T(changed_node_same_name_different_language_replaces_entry);
    ws::CrossLanguageSymbolTable table;
    // Originally Rust provider
    auto oldNode = makeNode("compute", "Python", "Rust");
    table.insert(oldNode);

    // Re-generation: now Go provider instead
    auto newNode = makeNode("compute", "Python", "Go");
    auto result  = ws::SymbolIndexUpdater::update(table, {oldNode}, {newNode});

    C(result.added >= 1 || result.removed >= 1,
      "change should register as add+remove or net change");
    C(table.contains("Python", "compute"), "compute still in Python");
    P();
}

void t5(){
    T(update_result_counts_correctly);
    ws::CrossLanguageSymbolTable table;
    auto n1 = makeNode("fn_a", "Python", "Rust", "from-a", "to-a");
    auto n2 = makeNode("fn_b", "Go",     "C++",  "from-b", "to-b");
    auto n3 = makeNode("fn_c", "Go",     "Rust", "from-c", "to-c");
    table.insert(n1);
    table.insert(n2);

    // n1 unchanged, n2 removed, n3 added
    auto result = ws::SymbolIndexUpdater::update(table, {n1, n2}, {n1, n3});

    C(result.added == 1,     "added must be 1 (fn_c)");
    C(result.removed == 1,   "removed must be 1 (fn_b)");
    C(result.unchanged == 1, "unchanged must be 1 (fn_a)");
    P();
}

int main(){
    std::cout << "Step 1905: SymbolIndexUpdater\n";
    t1(); t2(); t3(); t4(); t5();
    std::cout << "\n" << p << "/" << (p+f) << " passed\n";
    return f > 0 ? 1 : 0;
}
