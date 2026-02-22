// Step 689: SemanticCoreIR schema tests (12 tests)

#include "SemanticCoreIR.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static SemanticCoreIR sampleIr() {
    SemanticCoreIR ir;
    ir.moduleId = "mod-1";
    ir.moduleName = "Sample";
    ir.contracts = {{"preconditions", {"input != null"}},
                    {"postconditions", {"returns sorted output"}},
                    {"errorBehavior", {"raises invalid_input"}}};
    ir.extras = {{"futureField", "kept"}};

    IRNode mod{"n-mod", IRNodeKind::Module, "Sample", "rust", {"stateful"}, json::object(), json::object()};
    IRNode fn{"n-fn", IRNodeKind::Function, "sort_items", "rust", {"algorithmic", "pure"},
              json{{"reviewRequired", false}}, json{{"line", 10}}};
    IRNode own{"n-own", IRNodeKind::OwnershipRegion, "region-A", "rust", {"memory"},
               json{{"policy", "borrow_checked"}}, json::object()};
    IRNode eff{"n-eff", IRNodeKind::Effect, "io", "rust", {"io"},
               json{{"kind", "filesystem"}}, json::object()};

    ir.nodes = {mod, fn, own, eff};
    ir.edges = {
        {"n-mod", "n-fn", "contains"},
        {"n-fn", "n-own", "owns"},
        {"n-fn", "n-eff", "has_effect"}
    };
    return ir;
}

void test_schema_constructable() {
    TEST(schema_constructable);
    SemanticCoreIR ir;
    ir.moduleId = "m";
    CHECK(ir.moduleId == "m", "construct failed");
    PASS();
}

void test_node_identity_stability() {
    TEST(node_identity_stability);
    auto ir = sampleIr();
    const IRNode* n = SemanticCoreIRModel::findNode(ir, "n-fn");
    CHECK(n != nullptr, "missing node");
    CHECK(n->id == "n-fn", "identity changed");
    PASS();
}

void test_roundtrip_json() {
    TEST(roundtrip_json);
    auto ir = sampleIr();
    json j = SemanticCoreIRModel::toJson(ir);
    SemanticCoreIR ir2;
    std::string err;
    CHECK(SemanticCoreIRModel::fromJson(j, &ir2, &err), err.c_str());
    CHECK(ir2.moduleId == ir.moduleId, "module id mismatch");
    CHECK(ir2.nodes.size() == ir.nodes.size(), "node count mismatch");
    CHECK(ir2.edges.size() == ir.edges.size(), "edge count mismatch");
    PASS();
}

void test_function_graph_shape() {
    TEST(function_graph_shape);
    auto ir = sampleIr();
    auto edges = SemanticCoreIRModel::outgoing(ir, "n-fn");
    CHECK(edges.size() == 2, "expected 2 outgoing edges from function");
    PASS();
}

void test_ownership_edge_validity() {
    TEST(ownership_edge_validity);
    auto ir = sampleIr();
    std::string err;
    CHECK(SemanticCoreIRModel::validate(ir, &err), err.c_str());
    PASS();
}

void test_effect_tags_preserved() {
    TEST(effect_tags_preserved);
    auto ir = sampleIr();
    const IRNode* eff = SemanticCoreIRModel::findNode(ir, "n-eff");
    CHECK(eff != nullptr, "effect node missing");
    CHECK(!eff->intentTags.empty(), "intent tags missing");
    CHECK(eff->intentTags[0] == "io", "effect tag not preserved");
    PASS();
}

void test_empty_module_valid() {
    TEST(empty_module_valid);
    SemanticCoreIR ir;
    ir.moduleId = "m-empty";
    std::string err;
    CHECK(SemanticCoreIRModel::validate(ir, &err), err.c_str());
    PASS();
}

void test_invalid_edge_rejected() {
    TEST(invalid_edge_rejected);
    auto ir = sampleIr();
    ir.edges.push_back({"n-fn", "n-missing", "calls"});
    std::string err;
    CHECK(!SemanticCoreIRModel::validate(ir, &err), "expected validation failure");
    CHECK(err == "edge_endpoint_missing", "wrong error");
    PASS();
}

void test_deterministic_serialization() {
    TEST(deterministic_serialization);
    auto ir = sampleIr();
    std::string a = SemanticCoreIRModel::toJson(ir).dump();
    std::string b = SemanticCoreIRModel::toJson(ir).dump();
    CHECK(a == b, "serialization is not deterministic");
    PASS();
}

void test_language_metadata_preserved() {
    TEST(language_metadata_preserved);
    auto ir = sampleIr();
    json j = SemanticCoreIRModel::toJson(ir);
    SemanticCoreIR out;
    std::string err;
    CHECK(SemanticCoreIRModel::fromJson(j, &out, &err), err.c_str());
    const IRNode* fn = SemanticCoreIRModel::findNode(out, "n-fn");
    CHECK(fn != nullptr, "function missing");
    CHECK(fn->language == "rust", "language metadata mismatch");
    PASS();
}

void test_annotation_attaches_to_ir_nodes() {
    TEST(annotation_attaches_to_ir_nodes);
    auto ir = sampleIr();
    const IRNode* fn = SemanticCoreIRModel::findNode(ir, "n-fn");
    CHECK(fn != nullptr, "fn missing");
    CHECK(fn->annotations.contains("reviewRequired"), "annotation missing");
    PASS();
}

void test_unknown_tag_passthrough() {
    TEST(unknown_tag_passthrough);
    auto ir = sampleIr();
    ir.nodes[1].intentTags.push_back("x-unknown-tag");
    json j = SemanticCoreIRModel::toJson(ir);
    SemanticCoreIR out;
    std::string err;
    CHECK(SemanticCoreIRModel::fromJson(j, &out, &err), err.c_str());
    const IRNode* fn = SemanticCoreIRModel::findNode(out, "n-fn");
    CHECK(fn != nullptr, "fn missing");
    bool found = false;
    for (const auto& t : fn->intentTags) if (t == "x-unknown-tag") found = true;
    CHECK(found, "unknown tag not preserved");
    PASS();
}

int main() {
    std::cout << "Step 689: SemanticCoreIR schema tests\n";

    test_schema_constructable();          // 1
    test_node_identity_stability();       // 2
    test_roundtrip_json();                // 3
    test_function_graph_shape();          // 4
    test_ownership_edge_validity();       // 5
    test_effect_tags_preserved();         // 6
    test_empty_module_valid();            // 7
    test_invalid_edge_rejected();         // 8
    test_deterministic_serialization();   // 9
    test_language_metadata_preserved();   // 10
    test_annotation_attaches_to_ir_nodes(); // 11
    test_unknown_tag_passthrough();       // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
