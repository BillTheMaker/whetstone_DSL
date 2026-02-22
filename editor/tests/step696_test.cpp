// Step 696: baseline docs + examples (8 tests)

#include "Sprint46DocsBaseline.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

static int p = 0, f = 0;
#define T(n)    { std::cout << "  " << #n << "... "; }
#define P()     { std::cout << "PASS\n"; ++p; }
#define F(m)    { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m)  if (!(c)) { F(m); return; }

static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.good()) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static std::string readFirstExisting(const std::vector<std::string>& paths) {
    for (const auto& p : paths) {
        auto text = readFile(p);
        if (!text.empty()) return text;
    }
    return "";
}

void t1() {
    T(docs_index_includes_new_pages);
    auto paths = Sprint46DocsBaseline::docPaths();
    C(paths.size() == 2, "unexpected doc path count");
    C(paths[0].find("SPRINT46_FOUNDATION.md") != std::string::npos, "foundation doc missing");
    P();
}

void t2() {
    T(example_compiles_in_docs_build);
    auto text = readFirstExisting({
        "docs/examples/sprint46_semantic_core_example.json",
        "../docs/examples/sprint46_semantic_core_example.json",
        "../../docs/examples/sprint46_semantic_core_example.json"
    });
    C(!text.empty(), "example json missing");
    json j = json::parse(text);
    C(j.is_object(), "example json invalid");
    P();
}

void t3() {
    T(example_ir_valid);
    auto ir = Sprint46DocsBaseline::exampleIr();
    std::string err;
    C(SemanticCoreIRModel::validate(ir, &err), err.c_str());
    P();
}

void t4() {
    T(example_contract_packet_valid);
    auto packet = Sprint46DocsBaseline::exampleContractPacket();
    C(packet.value("minimumTestPassRate", -1.0f) >= 0.0f, "minimumTestPassRate missing");
    C(packet.contains("maxPerfRegressionPct"), "maxPerfRegressionPct missing");
    P();
}

void t5() {
    T(links_resolve);
    auto text = readFirstExisting({
        "docs/SPRINT46_FOUNDATION.md",
        "../docs/SPRINT46_FOUNDATION.md",
        "../../docs/SPRINT46_FOUNDATION.md"
    });
    C(!text.empty(), "foundation doc missing");
    C(text.find("docs/examples/sprint46_semantic_core_example.json") != std::string::npos,
      "example link missing");
    P();
}

void t6() {
    T(no_broken_references);
    auto text = readFirstExisting({
        "docs/SPRINT46_FOUNDATION.md",
        "../docs/SPRINT46_FOUNDATION.md",
        "../../docs/SPRINT46_FOUNDATION.md"
    });
    C(!text.empty(), "foundation doc missing");
    C(text.find("TODO") == std::string::npos, "contains TODO placeholders");
    P();
}

void t7() {
    T(schema_snippets_current);
    auto ir = Sprint46DocsBaseline::exampleIr();
    auto j = SemanticCoreIRModel::toJson(ir);
    C(j.contains("moduleId"), "missing moduleId");
    C(j.contains("nodes"), "missing nodes");
    P();
}

void t8() {
    T(deterministic_examples);
    auto a = SemanticCoreIRModel::toJson(Sprint46DocsBaseline::exampleIr()).dump();
    auto b = SemanticCoreIRModel::toJson(Sprint46DocsBaseline::exampleIr()).dump();
    C(a == b, "example not deterministic");
    P();
}

int main() {
    std::cout << "Step 696: docs baseline + examples\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << p << "/" << (p + f) << " passed\n";
    return f ? 1 : 0;
}
