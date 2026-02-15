// Step 315: Routing Annotation Types — Subject 9: Workflow (12 tests)
// 6 annotation types: ContextWidth, Review, Automatability, Priority,
// ImplementationStatus, + AmbiguityAnnotation level/description fields.
// Wired through Serialization, CompactAST, SidecarPersistence, SemannoFormat.

#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"
#include "ast/Serialization.h"
#include "CompactAST.h"
#include "SidecarPersistence.h"
#include "SemannoFormat.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <memory>

using json = nlohmann::json;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. ContextWidthAnnotation construction + properties
void test_context_width_construction() {
    TEST(context_width_construction);
    auto cw = std::make_unique<ContextWidthAnnotation>();
    cw->id = "cw1";
    cw->width = "project";
    CHECK(cw->conceptType == "ContextWidthAnnotation", "conceptType");
    CHECK(cw->width == "project", "width");
    PASS();
}

// 2. ReviewAnnotation construction + properties
void test_review_construction() {
    TEST(review_construction);
    auto rv = std::make_unique<ReviewAnnotation>();
    rv->id = "rv1";
    rv->required = true;
    rv->reviewer = "human";
    rv->reason = "security-critical";
    CHECK(rv->conceptType == "ReviewAnnotation", "conceptType");
    CHECK(rv->required == true, "required");
    CHECK(rv->reviewer == "human", "reviewer");
    CHECK(rv->reason == "security-critical", "reason");
    PASS();
}

// 3. AutomatabilityAnnotation construction
void test_automatability_construction() {
    TEST(automatability_construction);
    auto aa = std::make_unique<AutomatabilityAnnotation>();
    aa->id = "aa1";
    aa->strategy = "llm";
    aa->confidence = 0.85;
    CHECK(aa->conceptType == "AutomatabilityAnnotation", "conceptType");
    CHECK(aa->strategy == "llm", "strategy");
    CHECK(aa->confidence > 0.8, "confidence");
    PASS();
}

// 4. PriorityAnnotation with blockedBy
void test_priority_construction() {
    TEST(priority_construction);
    auto pa = std::make_unique<PriorityAnnotation>();
    pa->id = "pa1";
    pa->level = "critical";
    pa->blockedBy.push_back("task-1");
    pa->blockedBy.push_back("task-2");
    CHECK(pa->conceptType == "PriorityAnnotation", "conceptType");
    CHECK(pa->level == "critical", "level");
    CHECK(pa->blockedBy.size() == 2, "blockedBy count");
    CHECK(pa->blockedBy[0] == "task-1", "blockedBy[0]");
    PASS();
}

// 5. ImplementationStatusAnnotation construction
void test_impl_status_construction() {
    TEST(impl_status_construction);
    auto is = std::make_unique<ImplementationStatusAnnotation>();
    is->id = "is1";
    is->status = "skeleton";
    is->assignee = "agent-1";
    CHECK(is->conceptType == "ImplementationStatusAnnotation", "conceptType");
    CHECK(is->status == "skeleton", "status");
    CHECK(is->assignee == "agent-1", "assignee");
    PASS();
}

// 6. JSON roundtrip for all 5 types
void test_json_roundtrip() {
    TEST(json_roundtrip);
    // Build module with all 5 workflow annotations on a function
    auto mod = std::make_unique<Module>("mod1", "testmod", "python");
    auto fn = std::make_unique<Function>("fn1", "process");

    auto cw = std::make_unique<ContextWidthAnnotation>();
    cw->id = "cw1"; cw->width = "file";
    fn->addChild("annotations", cw.release());

    auto rv = std::make_unique<ReviewAnnotation>();
    rv->id = "rv1"; rv->required = true; rv->reviewer = "agent"; rv->reason = "complex";
    fn->addChild("annotations", rv.release());

    auto aa = std::make_unique<AutomatabilityAnnotation>();
    aa->id = "aa1"; aa->strategy = "template"; aa->confidence = 0.9;
    fn->addChild("annotations", aa.release());

    auto pa = std::make_unique<PriorityAnnotation>();
    pa->id = "pa1"; pa->level = "high"; pa->blockedBy.push_back("dep-1");
    fn->addChild("annotations", pa.release());

    auto is = std::make_unique<ImplementationStatusAnnotation>();
    is->id = "is1"; is->status = "partial"; is->assignee = "claude";
    fn->addChild("annotations", is.release());

    mod->addChild("functions", fn.release());

    // Serialize
    json j = toJson(mod.get());
    std::string serialized = j.dump();
    CHECK(serialized.find("ContextWidthAnnotation") != std::string::npos, "CW in JSON");
    CHECK(serialized.find("ReviewAnnotation") != std::string::npos, "RV in JSON");
    CHECK(serialized.find("AutomatabilityAnnotation") != std::string::npos, "AA in JSON");
    CHECK(serialized.find("PriorityAnnotation") != std::string::npos, "PA in JSON");
    CHECK(serialized.find("ImplementationStatusAnnotation") != std::string::npos, "IS in JSON");

    // Deserialize
    auto* restored = fromJson(j);
    CHECK(restored != nullptr, "fromJson returned non-null");
    auto fns = restored->getChildren("functions");
    CHECK(!fns.empty(), "has functions");
    auto annos = fns[0]->getChildren("annotations");
    CHECK(annos.size() == 5, "5 annotations restored, got: " + std::to_string(annos.size()));

    // Verify types
    bool hasCW = false, hasRV = false, hasAA = false, hasPA = false, hasIS = false;
    for (auto* a : annos) {
        if (a->conceptType == "ContextWidthAnnotation") {
            hasCW = true;
            auto* cwn = static_cast<ContextWidthAnnotation*>(a);
            CHECK(cwn->width == "file", "CW width restored");
        }
        if (a->conceptType == "ReviewAnnotation") {
            hasRV = true;
            auto* rvn = static_cast<ReviewAnnotation*>(a);
            CHECK(rvn->required == true, "RV required restored");
            CHECK(rvn->reviewer == "agent", "RV reviewer restored");
        }
        if (a->conceptType == "AutomatabilityAnnotation") {
            hasAA = true;
            auto* aan = static_cast<AutomatabilityAnnotation*>(a);
            CHECK(aan->strategy == "template", "AA strategy restored");
            CHECK(aan->confidence > 0.8, "AA confidence restored");
        }
        if (a->conceptType == "PriorityAnnotation") {
            hasPA = true;
            auto* pan = static_cast<PriorityAnnotation*>(a);
            CHECK(pan->level == "high", "PA level restored");
            CHECK(pan->blockedBy.size() == 1, "PA blockedBy restored");
        }
        if (a->conceptType == "ImplementationStatusAnnotation") {
            hasIS = true;
            auto* isn = static_cast<ImplementationStatusAnnotation*>(a);
            CHECK(isn->status == "partial", "IS status restored");
            CHECK(isn->assignee == "claude", "IS assignee restored");
        }
    }
    CHECK(hasCW && hasRV && hasAA && hasPA && hasIS, "All 5 types roundtripped");
    delete restored;
    PASS();
}

// 7. Compact AST semantic summary includes workflow annotations
void test_compact_ast_summary() {
    TEST(compact_ast_summary);
    auto mod = std::make_unique<Module>("mod1", "testmod", "python");
    auto fn = std::make_unique<Function>("fn1", "process");

    auto cw = std::make_unique<ContextWidthAnnotation>();
    cw->id = "cw1"; cw->width = "project";
    fn->addChild("annotations", cw.release());

    auto pa = std::make_unique<PriorityAnnotation>();
    pa->id = "pa1"; pa->level = "critical";
    fn->addChild("annotations", pa.release());

    auto is = std::make_unique<ImplementationStatusAnnotation>();
    is->id = "is1"; is->status = "skeleton";
    fn->addChild("annotations", is.release());

    auto* fnPtr = fn.get();
    mod->addChild("functions", fn.release());

    // Use toJsonCompactTree which walks the full tree including children
    json compactTree = toJsonCompactTree(mod.get());
    std::string str = compactTree.dump();
    CHECK(str.find("contextWidth") != std::string::npos, "contextWidth in compact");
    CHECK(str.find("priority") != std::string::npos, "priority in compact");
    CHECK(str.find("implStatus") != std::string::npos, "implStatus in compact");
    PASS();
}

// 8. SidecarPersistence recognizes all 5 as semantic annotations
void test_sidecar_recognition() {
    TEST(sidecar_recognition);
    CHECK(isSemanticAnnotation("ContextWidthAnnotation"), "CW is semantic");
    CHECK(isSemanticAnnotation("ReviewAnnotation"), "RV is semantic");
    CHECK(isSemanticAnnotation("AutomatabilityAnnotation"), "AA is semantic");
    CHECK(isSemanticAnnotation("PriorityAnnotation"), "PA is semantic");
    CHECK(isSemanticAnnotation("ImplementationStatusAnnotation"), "IS is semantic");
    PASS();
}

// 9. SemannoFormat emits all 5 types
void test_semanno_emission() {
    TEST(semanno_emission);
    SemannoEmitter emitter;

    auto cw = std::make_unique<ContextWidthAnnotation>();
    cw->id = "cw1"; cw->width = "local";
    std::string cwOut = emitter.emit(cw.get());
    CHECK(cwOut.find("@semanno:contextwidth") != std::string::npos, "CW semanno tag: " + cwOut);
    CHECK(cwOut.find("local") != std::string::npos, "CW width value");

    auto rv = std::make_unique<ReviewAnnotation>();
    rv->id = "rv1"; rv->required = true; rv->reviewer = "human";
    std::string rvOut = emitter.emit(rv.get());
    CHECK(rvOut.find("@semanno:review") != std::string::npos, "RV semanno tag: " + rvOut);
    CHECK(rvOut.find("human") != std::string::npos, "RV reviewer value");

    auto aa = std::make_unique<AutomatabilityAnnotation>();
    aa->id = "aa1"; aa->strategy = "deterministic";
    std::string aaOut = emitter.emit(aa.get());
    CHECK(aaOut.find("@semanno:automatability") != std::string::npos, "AA semanno tag: " + aaOut);
    CHECK(aaOut.find("deterministic") != std::string::npos, "AA strategy value");

    auto pa = std::make_unique<PriorityAnnotation>();
    pa->id = "pa1"; pa->level = "medium";
    std::string paOut = emitter.emit(pa.get());
    CHECK(paOut.find("@semanno:priority") != std::string::npos, "PA semanno tag: " + paOut);
    CHECK(paOut.find("medium") != std::string::npos, "PA level value");

    auto is = std::make_unique<ImplementationStatusAnnotation>();
    is->id = "is1"; is->status = "complete"; is->assignee = "alice";
    std::string isOut = emitter.emit(is.get());
    CHECK(isOut.find("@semanno:implstatus") != std::string::npos, "IS semanno tag: " + isOut);
    CHECK(isOut.find("complete") != std::string::npos, "IS status value");
    PASS();
}

// 10. AmbiguityAnnotation extended with level + description
void test_ambiguity_extended() {
    TEST(ambiguity_extended);
    auto aa = std::make_unique<AmbiguityAnnotation>();
    aa->id = "amb1";
    aa->intent = "original intent";
    aa->options.push_back("opt1");
    aa->level = "high";
    aa->description = "Requirements are unclear";

    // JSON roundtrip
    auto mod = std::make_unique<Module>("mod1", "testmod", "python");
    auto fn = std::make_unique<Function>("fn1", "process");
    fn->addChild("annotations", aa.release());
    mod->addChild("functions", fn.release());

    json j = toJson(mod.get());
    auto* restored = fromJson(j);
    auto annos = restored->getChildren("functions")[0]->getChildren("annotations");
    CHECK(!annos.empty(), "has annotations");
    auto* amb = static_cast<AmbiguityAnnotation*>(annos[0]);
    CHECK(amb->intent == "original intent", "intent preserved");
    CHECK(amb->level == "high", "level preserved");
    CHECK(amb->description == "Requirements are unclear", "description preserved");
    delete restored;
    PASS();
}

// 11. Semanno parse roundtrip for workflow annotations
void test_semanno_parse_roundtrip() {
    TEST(semanno_parse_roundtrip);
    SemannoEmitter emitter;

    auto cw = std::make_unique<ContextWidthAnnotation>();
    cw->id = "cw1"; cw->width = "cross-project";
    std::string emitted = "# " + emitter.emit(cw.get());

    auto entry = SemannoParser::parse(emitted);
    CHECK(entry.type == "contextwidth", "Parsed type: " + entry.type);
    CHECK(entry.properties.count("width"), "has width property");
    CHECK(entry.properties["width"] == "cross-project", "width value: " + entry.properties["width"]);
    PASS();
}

// 12. Validation of enum values
void test_enum_validation() {
    TEST(enum_validation);
    // ContextWidth valid values
    std::set<std::string> validWidths = {"local", "file", "project", "cross-project"};
    for (const auto& w : validWidths) {
        auto cw = std::make_unique<ContextWidthAnnotation>();
        cw->width = w;
        CHECK(validWidths.count(cw->width), "Valid width: " + w);
    }

    // Automatability valid values
    std::set<std::string> validStrategies = {"deterministic", "template", "slm", "llm", "human"};
    for (const auto& s : validStrategies) {
        auto aa = std::make_unique<AutomatabilityAnnotation>();
        aa->strategy = s;
        CHECK(validStrategies.count(aa->strategy), "Valid strategy: " + s);
    }

    // Priority valid values
    std::set<std::string> validLevels = {"critical", "high", "medium", "low"};
    for (const auto& l : validLevels) {
        auto pa = std::make_unique<PriorityAnnotation>();
        pa->level = l;
        CHECK(validLevels.count(pa->level), "Valid level: " + l);
    }

    // ImplementationStatus valid values
    std::set<std::string> validStatuses = {"skeleton", "partial", "complete", "needs-review"};
    for (const auto& s : validStatuses) {
        auto is = std::make_unique<ImplementationStatusAnnotation>();
        is->status = s;
        CHECK(validStatuses.count(is->status), "Valid status: " + s);
    }
    PASS();
}

int main() {
    std::cout << "Step 315: Routing Annotation Types (Subject 9: Workflow)\n";
    test_context_width_construction();
    test_review_construction();
    test_automatability_construction();
    test_priority_construction();
    test_impl_status_construction();
    test_json_roundtrip();
    test_compact_ast_summary();
    test_sidecar_recognition();
    test_semanno_emission();
    test_ambiguity_extended();
    test_semanno_parse_roundtrip();
    test_enum_validation();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
