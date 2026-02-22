// Step 693: adapter API contracts (10 tests)

#include "IRToLanguageAdapter.h"
#include "LanguageToIRAdapter.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

class FakeLower final : public LanguageToIRAdapter {
public:
    std::string language() const override { return "rust"; }
    std::string version() const override { return "v1"; }
    AdapterResult lower(const std::string& source, SemanticCoreIR* out) const override {
        AdapterResult r;
        if (!out) {
            r.success = false;
            r.error = "out_missing";
            return r;
        }
        out->moduleId = "mod";
        out->moduleName = "demo";
        out->nodes.push_back({"f1", IRNodeKind::Function, "run", "rust", {"algorithmic"}, json::object(), json::object()});
        r.success = true;
        r.confidence = source.empty() ? 0.2f : 0.9f;
        r.reviewRequired = source.find("unsafe") != std::string::npos;
        if (source.find("asm!") != std::string::npos) {
            r.unsupportedFeatures.push_back("inline_asm");
            r.success = false;
        }
        r.sourceLocationMap["f1"] = 1;
        return r;
    }
};

class FakeRaise final : public IRToLanguageAdapter {
public:
    std::string targetLanguage() const override { return "cpp"; }
    std::string version() const override { return "v1"; }
    AdapterResult raise(const SemanticCoreIR& ir, std::string* outCode) const override {
        AdapterResult r;
        if (!outCode) {
            r.success = false;
            r.error = "out_code_missing";
            return r;
        }
        *outCode = "int main(){return " + std::to_string((int)ir.nodes.size()) + ";}";
        r.success = true;
        r.confidence = 0.85f;
        return r;
    }
};

class FakeLowerV2 final : public LanguageToIRAdapter {
public:
    std::string language() const override { return "rust"; }
    std::string version() const override { return "v2"; }
    AdapterResult lower(const std::string&, SemanticCoreIR* out) const override {
        AdapterResult r;
        if (out) out->moduleId = "mod2";
        r.success = true;
        r.confidence = 0.7f;
        return r;
    }
};

void t1() {
    TEST(interface_conformance);
    FakeLower lower;
    FakeRaise raise;
    CHECK(lower.language() == "rust", "lower interface broken");
    CHECK(raise.targetLanguage() == "cpp", "raise interface broken");
    PASS();
}

void t2() {
    TEST(unsupported_feature_signaling);
    FakeLower lower;
    SemanticCoreIR ir;
    auto out = lower.lower("asm!()", &ir);
    CHECK(!out.success, "expected unsupported fail");
    CHECK(!out.unsupportedFeatures.empty(), "unsupported features missing");
    PASS();
}

void t3() {
    TEST(partial_lowering_packets);
    FakeLower lower;
    SemanticCoreIR ir;
    auto out = lower.lower("unsafe fn x(){}", &ir);
    CHECK(out.success, "expected success");
    CHECK(out.reviewRequired, "review flag expected");
    PASS();
}

void t4() {
    TEST(confidence_propagation);
    FakeLower lower;
    SemanticCoreIR ir;
    auto out = lower.lower("fn run(){}", &ir);
    CHECK(out.confidence > 0.5f, "confidence too low");
    PASS();
}

void t5() {
    TEST(source_location_mapping);
    FakeLower lower;
    SemanticCoreIR ir;
    auto out = lower.lower("fn run(){}", &ir);
    CHECK(out.sourceLocationMap.find("f1") != out.sourceLocationMap.end(), "location map missing");
    PASS();
}

void t6() {
    TEST(annotation_carry_through);
    FakeLower lower;
    SemanticCoreIR ir;
    auto out = lower.lower("fn run(){}", &ir);
    CHECK(out.success, "lower failed");
    CHECK(!ir.nodes.empty(), "nodes missing");
    CHECK(ir.nodes[0].intentTags[0] == "algorithmic", "annotation/tag missing");
    PASS();
}

void t7() {
    TEST(adapter_version_pinning);
    FakeLower lower;
    CHECK(lower.version() == "v1", "version pin mismatch");
    PASS();
}

void t8() {
    TEST(registry_lookup);
    FakeLower lower;
    LanguageToIRAdapterRegistry reg;
    reg.registerAdapter(&lower);
    CHECK(reg.select("rust") != nullptr, "lookup failed");
    PASS();
}

void t9() {
    TEST(registry_fallback_behavior);
    FakeLower v1;
    FakeLowerV2 v2;
    LanguageToIRAdapterRegistry reg;
    reg.registerAdapter(&v1);
    reg.registerAdapter(&v2);
    const auto* any = reg.select("rust");
    const auto* pinned = reg.select("rust", "v2");
    CHECK(any != nullptr && any->version() == "v1", "fallback should select first");
    CHECK(pinned != nullptr && pinned->version() == "v2", "version pinning failed");
    PASS();
}

void t10() {
    TEST(deterministic_adapter_selection);
    FakeLower v1;
    LanguageToIRAdapterRegistry reg;
    reg.registerAdapter(&v1);
    const auto* a = reg.select("rust");
    const auto* b = reg.select("rust");
    CHECK(a == b, "selection not deterministic");
    PASS();
}

int main() {
    std::cout << "Step 693: adapter API contracts\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8(); t9(); t10();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
