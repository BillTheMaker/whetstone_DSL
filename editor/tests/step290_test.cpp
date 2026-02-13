// Step 290: SemannoFormat.h — Comment Format Standard (12 tests)
#include "SemannoFormat.h"
#include <iostream>
#include <cassert>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. Subject 1 memory annotation roundtrip
void test_memory_annotation_roundtrip() {
    TEST(memory_annotation_roundtrip);
    auto anno = std::make_unique<DeallocateAnnotation>();
    anno->strategy = "Explicit";
    anno->deallocateLocation = "scope_end";
    std::string emitted = SemannoEmitter::emit(anno.get());
    CHECK(emitted.find("@semanno:deallocate") != std::string::npos, "missing tag");
    CHECK(emitted.find("strategy=\"Explicit\"") != std::string::npos, "missing strategy");

    auto entry = SemannoParser::parse(emitted);
    CHECK(entry.type == "deallocate", "wrong type: " + entry.type);
    CHECK(entry.properties["strategy"] == "Explicit", "wrong strategy");
    CHECK(entry.properties["deallocateLocation"] == "scope_end", "wrong location");
    PASS();
}

// 2. Subject 2 type system annotation roundtrip
void test_type_system_roundtrip() {
    TEST(type_system_roundtrip);
    auto anno = std::make_unique<BitWidthAnnotation>();
    anno->width = 32;
    std::string emitted = SemannoEmitter::emit(anno.get());
    CHECK(emitted == "@semanno:bitwidth(width=32)", "wrong emit: " + emitted);

    auto entry = SemannoParser::parse(emitted);
    CHECK(entry.type == "bitwidth", "wrong type");
    CHECK(entry.properties["width"] == "32", "wrong width");
    PASS();
}

// 3. Subject 3 concurrency annotation roundtrip
void test_concurrency_roundtrip() {
    TEST(concurrency_roundtrip);
    auto anno = std::make_unique<AtomicAnnotation>();
    anno->consistency = "seq_cst";
    std::string emitted = SemannoEmitter::emit(anno.get());
    CHECK(emitted.find("@semanno:atomic") != std::string::npos, "missing tag");
    CHECK(emitted.find("consistency=\"seq_cst\"") != std::string::npos, "missing consistency");

    auto entry = SemannoParser::parse(emitted);
    CHECK(entry.type == "atomic", "wrong type");
    CHECK(entry.properties["consistency"] == "seq_cst", "wrong consistency");
    PASS();
}

// 4. Subject 4 scope annotation roundtrip
void test_scope_roundtrip() {
    TEST(scope_roundtrip);
    auto anno = std::make_unique<CaptureAnnotation>();
    anno->strategy = "move";
    std::string emitted = SemannoEmitter::emit(anno.get());
    CHECK(emitted == "@semanno:capture(strategy=\"move\")", "wrong emit: " + emitted);

    auto entry = SemannoParser::parse(emitted);
    CHECK(entry.type == "capture", "wrong type");
    CHECK(entry.properties["strategy"] == "move", "wrong strategy");
    PASS();
}

// 5. Subject 5 shim annotation roundtrip
void test_shim_roundtrip() {
    TEST(shim_roundtrip);
    auto anno = std::make_unique<IntrinsicAnnotation>();
    anno->instruction = "_mm256_add_ps";
    anno->arch = "x86_64";
    std::string emitted = SemannoEmitter::emit(anno.get());
    CHECK(emitted.find("@semanno:intrinsic") != std::string::npos, "missing tag");

    auto entry = SemannoParser::parse(emitted);
    CHECK(entry.type == "intrinsic", "wrong type");
    CHECK(entry.properties["instruction"] == "_mm256_add_ps", "wrong instruction");
    CHECK(entry.properties["arch"] == "x86_64", "wrong arch");
    PASS();
}

// 6. Subject 6 optimization annotation roundtrip
void test_optimization_roundtrip() {
    TEST(optimization_roundtrip);
    auto anno = std::make_unique<LoopAnnotation>();
    anno->hint = "vectorize";
    anno->factor = 4;
    std::string emitted = SemannoEmitter::emit(anno.get());
    CHECK(emitted.find("@semanno:loop") != std::string::npos, "missing tag");
    CHECK(emitted.find("hint=\"vectorize\"") != std::string::npos, "missing hint");
    CHECK(emitted.find("factor=4") != std::string::npos, "missing factor");

    auto entry = SemannoParser::parse(emitted);
    CHECK(entry.type == "loop", "wrong type");
    CHECK(entry.properties["hint"] == "vectorize", "wrong hint");
    CHECK(entry.properties["factor"] == "4", "wrong factor");
    PASS();
}

// 7. Subject 7 meta-programming roundtrip
void test_meta_roundtrip() {
    TEST(meta_roundtrip);
    auto anno = std::make_unique<MetaAnnotation>();
    anno->state = "quoted";
    anno->phase = "compile";
    std::string emitted = SemannoEmitter::emit(anno.get());

    auto entry = SemannoParser::parse(emitted);
    CHECK(entry.type == "meta", "wrong type");
    CHECK(entry.properties["state"] == "quoted", "wrong state");
    CHECK(entry.properties["phase"] == "compile", "wrong phase");
    PASS();
}

// 8. Subject 8 policy annotation roundtrip
void test_policy_roundtrip() {
    TEST(policy_roundtrip);
    auto anno = std::make_unique<PolicyAnnotation>();
    anno->strictness = "high";
    anno->perf = "critical";
    anno->style = "idiomatic";
    anno->binaryStable = true;
    std::string emitted = SemannoEmitter::emit(anno.get());

    auto entry = SemannoParser::parse(emitted);
    CHECK(entry.type == "policy", "wrong type");
    CHECK(entry.properties["strictness"] == "high", "wrong strictness");
    CHECK(entry.properties["perf"] == "critical", "wrong perf");
    CHECK(entry.properties["binaryStable"] == "true", "wrong binaryStable");
    PASS();
}

// 9. Marker annotations (no properties)
void test_marker_annotations() {
    TEST(marker_annotations);
    auto pure = std::make_unique<PureAnnotation>();
    std::string e1 = SemannoEmitter::emit(pure.get());
    CHECK(e1 == "@semanno:pure", "wrong pure: " + e1);

    auto tailcall = std::make_unique<TailCallAnnotation>();
    std::string e2 = SemannoEmitter::emit(tailcall.get());
    CHECK(e2 == "@semanno:tailcall", "wrong tailcall: " + e2);

    auto pack = std::make_unique<PackAnnotation>();
    std::string e3 = SemannoEmitter::emit(pack.get());
    CHECK(e3 == "@semanno:pack", "wrong pack: " + e3);

    auto barrier = std::make_unique<MemoryBarrierAnnotation>();
    std::string e4 = SemannoEmitter::emit(barrier.get());
    CHECK(e4 == "@semanno:memorybarrier", "wrong barrier: " + e4);
    PASS();
}

// 10. Vector property roundtrip (semicolon-separated)
void test_vector_properties() {
    TEST(vector_properties);
    auto anno = std::make_unique<AmbiguityAnnotation>();
    anno->intent = "choose logging";
    anno->options = {"syslog", "journald", "file"};
    std::string emitted = SemannoEmitter::emit(anno.get());
    CHECK(emitted.find("@semanno:ambiguity") != std::string::npos, "missing tag");
    CHECK(emitted.find("options=\"syslog;journald;file\"") != std::string::npos, "wrong options: " + emitted);

    auto entry = SemannoParser::parse(emitted);
    CHECK(entry.type == "ambiguity", "wrong type");
    CHECK(entry.properties["intent"] == "choose logging", "wrong intent");
    // Vector values are stored as semicolon-separated string
    CHECK(entry.properties["options"] == "syslog;journald;file", "wrong options: " + entry.properties["options"]);
    PASS();
}

// 11. Comment prefix parsing (various languages)
void test_comment_prefix_parsing() {
    TEST(comment_prefix_parsing);
    // Python-style
    auto e1 = SemannoParser::parse("# @semanno:pure");
    CHECK(e1.type == "pure", "failed python comment");

    // C-style
    auto e2 = SemannoParser::parse("// @semanno:bitwidth(width=64)");
    CHECK(e2.type == "bitwidth", "failed C comment");
    CHECK(e2.properties["width"] == "64", "wrong width");

    // Elisp-style
    auto e3 = SemannoParser::parse(";; @semanno:capture(strategy=\"ref\")");
    CHECK(e3.type == "capture", "failed elisp comment");
    CHECK(e3.properties["strategy"] == "ref", "wrong strategy");

    // isSemannoComment
    CHECK(SemannoParser::isSemannoComment("// @semanno:pure"), "should detect");
    CHECK(!SemannoParser::isSemannoComment("// just a comment"), "false positive");
    PASS();
}

// 12. Value escaping (quotes and backslashes)
void test_value_escaping() {
    TEST(value_escaping);
    auto anno = std::make_unique<RawAnnotation>();
    anno->language = "cpp";
    anno->code = "printf(\"hello\\n\")";
    std::string emitted = SemannoEmitter::emit(anno.get());
    CHECK(emitted.find("@semanno:raw") != std::string::npos, "missing tag");

    auto entry = SemannoParser::parse(emitted);
    CHECK(entry.type == "raw", "wrong type");
    CHECK(entry.properties["language"] == "cpp", "wrong language");
    CHECK(entry.properties["code"] == "printf(\"hello\\n\")", "wrong code: " + entry.properties["code"]);
    PASS();
}

int main() {
    std::cout << "=== Step 290: SemannoFormat Tests ===\n";
    test_memory_annotation_roundtrip();
    test_type_system_roundtrip();
    test_concurrency_roundtrip();
    test_scope_roundtrip();
    test_shim_roundtrip();
    test_optimization_roundtrip();
    test_meta_roundtrip();
    test_policy_roundtrip();
    test_marker_annotations();
    test_vector_properties();
    test_comment_prefix_parsing();
    test_value_escaping();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
