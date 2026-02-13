// Step 291: ProjectionGenerator — Subject 2-4 Visitors (12 tests)
#include "ast/ProjectionGenerator.h"
#include "ast/PythonGenerator.h"
#include "ast/CppGenerator.h"
#include "SemannoFormat.h"
#include <iostream>
#include <cassert>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. BitWidth dispatch returns non-empty
void test_bitwidth_dispatch() {
    TEST(bitwidth_dispatch);
    PythonGenerator gen;
    auto anno = std::make_unique<BitWidthAnnotation>();
    anno->width = 32;
    std::string result = gen.generate(anno.get());
    CHECK(!result.empty(), "empty result");
    CHECK(result.find("@semanno:bitwidth") != std::string::npos, "missing semanno tag");
    PASS();
}

// 2. Endian dispatch
void test_endian_dispatch() {
    TEST(endian_dispatch);
    CppGenerator gen;
    auto anno = std::make_unique<EndianAnnotation>();
    anno->order = "big";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:endian") != std::string::npos, "missing semanno");
    PASS();
}

// 3. Layout dispatch
void test_layout_dispatch() {
    TEST(layout_dispatch);
    PythonGenerator gen;
    auto anno = std::make_unique<LayoutAnnotation>();
    anno->mode = "packed";
    anno->alignment = 8;
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:layout") != std::string::npos, "missing semanno");
    CHECK(result.find("packed") != std::string::npos, "missing mode");
    PASS();
}

// 4. Nullability dispatch
void test_nullability_dispatch() {
    TEST(nullability_dispatch);
    CppGenerator gen;
    auto anno = std::make_unique<NullabilityAnnotation>();
    anno->nullable = true;
    anno->strategy = "strict";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:nullability") != std::string::npos, "missing semanno");
    PASS();
}

// 5. Variance dispatch
void test_variance_dispatch() {
    TEST(variance_dispatch);
    PythonGenerator gen;
    auto anno = std::make_unique<VarianceAnnotation>();
    anno->variance = "covariant";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("covariant") != std::string::npos, "missing variance");
    PASS();
}

// 6. AtomicAnnotation dispatch (Subject 3)
void test_atomic_dispatch() {
    TEST(atomic_dispatch);
    CppGenerator gen;
    auto anno = std::make_unique<AtomicAnnotation>();
    anno->consistency = "relaxed";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:atomic") != std::string::npos, "missing semanno");
    CHECK(result.find("relaxed") != std::string::npos, "missing consistency");
    PASS();
}

// 7. SyncAnnotation dispatch
void test_sync_dispatch() {
    TEST(sync_dispatch);
    PythonGenerator gen;
    auto anno = std::make_unique<SyncAnnotation>();
    anno->primitive = "monitor";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:sync") != std::string::npos, "missing semanno");
    PASS();
}

// 8. ExecAnnotation dispatch
void test_exec_dispatch() {
    TEST(exec_dispatch);
    CppGenerator gen;
    auto anno = std::make_unique<ExecAnnotation>();
    anno->mode = "async";
    anno->runtimeHint = "tokio";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:exec") != std::string::npos, "missing semanno");
    CHECK(result.find("async") != std::string::npos, "missing mode");
    PASS();
}

// 9. CaptureAnnotation dispatch (Subject 4)
void test_capture_dispatch() {
    TEST(capture_dispatch);
    PythonGenerator gen;
    auto anno = std::make_unique<CaptureAnnotation>();
    anno->strategy = "value";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:capture") != std::string::npos, "missing semanno");
    PASS();
}

// 10. VisibilityAnnotation dispatch
void test_visibility_dispatch() {
    TEST(visibility_dispatch);
    CppGenerator gen;
    auto anno = std::make_unique<VisibilityAnnotation>();
    anno->level = "private";
    std::string result = gen.generate(anno.get());
    CHECK(result.find("@semanno:visibility") != std::string::npos, "missing semanno");
    PASS();
}

// 11. No "Unknown concept" for Subject 2-4
void test_no_unknown_subject2_4() {
    TEST(no_unknown_subject2_4);
    PythonGenerator gen;

    std::vector<std::unique_ptr<ASTNode>> nodes;
    auto bw = std::make_unique<BitWidthAnnotation>(); bw->width = 16;
    auto end = std::make_unique<EndianAnnotation>(); end->order = "little";
    auto id = std::make_unique<IdentityAnnotation>(); id->mode = "nominal";
    auto mut = std::make_unique<MutAnnotation>(); mut->depth = "deep";
    auto ts = std::make_unique<TypeStateAnnotation>(); ts->state = "reified";
    auto tm = std::make_unique<ThreadModelAnnotation>(); tm->model = "green";
    auto mb = std::make_unique<MemoryBarrierAnnotation>();
    auto blk = std::make_unique<BlockingAnnotation>(); blk->kind = "io";
    auto par = std::make_unique<ParallelAnnotation>(); par->kind = "data";
    auto trp = std::make_unique<TrapAnnotation>(); trp->signal = "SIGFPE";
    auto exc = std::make_unique<ExceptionAnnotation>(); exc->style = "checked";
    auto pan = std::make_unique<PanicAnnotation>(); pan->behavior = "abort";
    auto bnd = std::make_unique<BindingAnnotation>(); bnd->time = "static";
    auto lkp = std::make_unique<LookupAnnotation>(); lkp->mode = "lexical";
    auto ns = std::make_unique<NamespaceAnnotation>(); ns->style = "qualified";
    auto scp = std::make_unique<ScopeAnnotation>(); scp->kind = "local";

    const ASTNode* allNodes[] = {
        bw.get(), end.get(), id.get(), mut.get(), ts.get(),
        tm.get(), mb.get(), blk.get(), par.get(), trp.get(),
        exc.get(), pan.get(), bnd.get(), lkp.get(), ns.get(), scp.get()
    };
    for (auto* n : allNodes) {
        std::string result = gen.generate(n);
        CHECK(result.find("Unknown") == std::string::npos,
              "Unknown concept for " + n->conceptType + ": " + result);
    }
    PASS();
}

// 12. Python vs C++ comment prefix
void test_python_cpp_prefix() {
    TEST(python_cpp_prefix);
    PythonGenerator pyGen;
    CppGenerator cppGen;

    auto anno = std::make_unique<AtomicAnnotation>();
    anno->consistency = "acquire";

    std::string pyResult = pyGen.generate(anno.get());
    std::string cppResult = cppGen.generate(anno.get());

    CHECK(pyResult.substr(0, 2) == "# ", "python prefix wrong: " + pyResult.substr(0, 5));
    CHECK(cppResult.substr(0, 3) == "// ", "cpp prefix wrong: " + cppResult.substr(0, 5));
    PASS();
}

int main() {
    std::cout << "=== Step 291: Subject 2-4 Visitor Tests ===\n";
    test_bitwidth_dispatch();
    test_endian_dispatch();
    test_layout_dispatch();
    test_nullability_dispatch();
    test_variance_dispatch();
    test_atomic_dispatch();
    test_sync_dispatch();
    test_exec_dispatch();
    test_capture_dispatch();
    test_visibility_dispatch();
    test_no_unknown_subject2_4();
    test_python_cpp_prefix();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
