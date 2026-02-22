// Step 690: Language support tiers + gate mapping tests (10 tests)

#include "LanguageSupportTier.h"

#include <iostream>

using json = nlohmann::json;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_tier_parse() {
    TEST(tier_parse);
    CHECK(LanguageSupportTierModel::tierFromString("experimental") == SupportTier::Experimental, "exp parse");
    CHECK(LanguageSupportTierModel::tierFromString("beta") == SupportTier::Beta, "beta parse");
    CHECK(LanguageSupportTierModel::tierFromString("stable") == SupportTier::Stable, "stable parse");
    PASS();
}

void test_gate_mapping() {
    TEST(gate_mapping);
    auto e = LanguageSupportTierModel::requiredGates(SupportTier::Experimental);
    auto b = LanguageSupportTierModel::requiredGates(SupportTier::Beta);
    auto s = LanguageSupportTierModel::requiredGates(SupportTier::Stable);
    CHECK(e.parseAndProject, "exp missing parse");
    CHECK(b.executableEquivalence && b.staticChecks, "beta missing gates");
    CHECK(s.securityGates && s.performanceGates && s.migrationReportQuality, "stable missing gates");
    PASS();
}

void test_stable_implies_beta_gates() {
    TEST(stable_implies_beta_gates);
    auto b = LanguageSupportTierModel::requiredGates(SupportTier::Beta);
    auto s = LanguageSupportTierModel::requiredGates(SupportTier::Stable);
    CHECK(LanguageSupportTierModel::gateSuperset(s, b), "stable should supersede beta");
    PASS();
}

void test_invalid_tier_rejected() {
    TEST(invalid_tier_rejected);
    LanguageTierStatus out;
    std::string err;
    json j = {{"language", "rust"}, {"tier", "nonsense"}};
    CHECK(!LanguageSupportTierModel::fromJson(j, &out, &err), "expected failure");
    CHECK(err == "tier_unknown", "wrong error");
    PASS();
}

void test_default_tier_experimental() {
    TEST(default_tier_experimental);
    LanguageTierStatus s;
    CHECK(s.tier == SupportTier::Experimental, "default tier mismatch");
    PASS();
}

void test_serialization_roundtrip() {
    TEST(serialization_roundtrip);
    LanguageTierStatus in;
    in.language = "rust";
    in.tier = SupportTier::Stable;
    in.gates = LanguageSupportTierModel::requiredGates(in.tier);
    in.warnings = {"none"};
    json j = LanguageSupportTierModel::toJson(in);
    LanguageTierStatus out;
    std::string err;
    CHECK(LanguageSupportTierModel::fromJson(j, &out, &err), err.c_str());
    CHECK(out.language == in.language, "language mismatch");
    CHECK(out.tier == in.tier, "tier mismatch");
    PASS();
}

void test_upgrade_path_validity() {
    TEST(upgrade_path_validity);
    CHECK(LanguageSupportTierModel::isUpgradePathValid(SupportTier::Experimental, SupportTier::Beta), "exp->beta invalid");
    CHECK(LanguageSupportTierModel::isUpgradePathValid(SupportTier::Beta, SupportTier::Stable), "beta->stable invalid");
    CHECK(!LanguageSupportTierModel::isUpgradePathValid(SupportTier::Stable, SupportTier::Experimental), "stable->exp should be invalid");
    PASS();
}

void test_downgrade_warnings() {
    TEST(downgrade_warnings);
    auto w = LanguageSupportTierModel::downgradeWarnings(SupportTier::Stable, SupportTier::Experimental);
    CHECK(!w.empty(), "expected warnings");
    CHECK(w[0] == "tier_downgrade_detected", "missing base warning");
    PASS();
}

void test_deterministic_ordering() {
    TEST(deterministic_ordering);
    LanguageTierStatus a{"python", SupportTier::Beta, {}, {}};
    LanguageTierStatus b{"cpp", SupportTier::Experimental, {}, {}};
    LanguageTierStatus c{"rust", SupportTier::Stable, {}, {}};
    auto sorted = LanguageSupportTierModel::sortByLanguage({a, b, c});
    CHECK(sorted[0].language == "cpp", "ordering 1");
    CHECK(sorted[1].language == "python", "ordering 2");
    CHECK(sorted[2].language == "rust", "ordering 3");
    PASS();
}

void test_docs_export() {
    TEST(docs_export);
    LanguageTierStatus a{"python", SupportTier::Beta, {}, {}};
    LanguageTierStatus b{"cpp", SupportTier::Experimental, {}, {}};
    auto lines = LanguageSupportTierModel::exportDocs({a, b});
    CHECK(lines.find("python:beta") != lines.end(), "python line missing");
    CHECK(lines.find("cpp:experimental") != lines.end(), "cpp line missing");
    PASS();
}

int main() {
    std::cout << "Step 690: Language support tiers + gates tests\n";
    test_tier_parse();                // 1
    test_gate_mapping();              // 2
    test_stable_implies_beta_gates(); // 3
    test_invalid_tier_rejected();     // 4
    test_default_tier_experimental(); // 5
    test_serialization_roundtrip();   // 6
    test_upgrade_path_validity();     // 7
    test_downgrade_warnings();        // 8
    test_deterministic_ordering();    // 9
    test_docs_export();               // 10

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
