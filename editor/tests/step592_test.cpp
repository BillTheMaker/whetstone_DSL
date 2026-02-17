// Step 592: Documentation + Operator Playbooks (12 tests)

#include "DocumentationOperatorPlaybooks.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_add_document_success() {
    TEST(add_document_success);
    DocumentationOperatorPlaybooks p;
    std::string error;
    CHECK(p.addDocument("doc-1", "Deployment Guide", &error), "add doc should succeed");
    CHECK(p.allDocuments().size() == 1, "doc count mismatch");
    PASS();
}

void test_add_document_rejects_missing_id() {
    TEST(add_document_rejects_missing_id);
    DocumentationOperatorPlaybooks p;
    std::string error;
    CHECK(!p.addDocument("", "Deployment Guide", &error), "add doc should fail");
    CHECK(error == "doc_id_missing", "wrong error");
    PASS();
}

void test_add_document_rejects_duplicate() {
    TEST(add_document_rejects_duplicate);
    DocumentationOperatorPlaybooks p;
    std::string error;
    CHECK(p.addDocument("doc-1", "Deployment Guide", &error), "first add failed");
    CHECK(!p.addDocument("doc-1", "Duplicate", &error), "duplicate should fail");
    CHECK(error == "doc_duplicate", "wrong error");
    PASS();
}

void test_add_section_success() {
    TEST(add_section_success);
    DocumentationOperatorPlaybooks p;
    std::string error;
    CHECK(p.addDocument("doc-1", "Deployment Guide", &error), "add doc failed");
    CHECK(p.addSection("doc-1", "Install", &error), "add section failed");
    CHECK(p.allDocuments()[0].sections.size() == 1, "section count mismatch");
    PASS();
}

void test_add_section_rejects_missing_doc() {
    TEST(add_section_rejects_missing_doc);
    DocumentationOperatorPlaybooks p;
    std::string error;
    CHECK(!p.addSection("missing", "Install", &error), "add section should fail");
    CHECK(error == "doc_missing", "wrong error");
    PASS();
}

void test_add_section_rejects_duplicate() {
    TEST(add_section_rejects_duplicate);
    DocumentationOperatorPlaybooks p;
    std::string error;
    CHECK(p.addDocument("doc-1", "Deployment Guide", &error), "add doc failed");
    CHECK(p.addSection("doc-1", "Install", &error), "add section failed");
    CHECK(!p.addSection("doc-1", "Install", &error), "duplicate section should fail");
    CHECK(error == "section_duplicate", "wrong error");
    PASS();
}

void test_add_operator_check_success() {
    TEST(add_operator_check_success);
    DocumentationOperatorPlaybooks p;
    std::string error;
    CHECK(p.addDocument("doc-1", "Deployment Guide", &error), "add doc failed");
    CHECK(p.addOperatorCheck("doc-1", "Verify logs clean", &error), "add check failed");
    CHECK(p.allDocuments()[0].operatorChecks.size() == 1, "check count mismatch");
    PASS();
}

void test_add_operator_check_rejects_duplicate() {
    TEST(add_operator_check_rejects_duplicate);
    DocumentationOperatorPlaybooks p;
    std::string error;
    CHECK(p.addDocument("doc-1", "Deployment Guide", &error), "add doc failed");
    CHECK(p.addOperatorCheck("doc-1", "Verify logs clean", &error), "add check failed");
    CHECK(!p.addOperatorCheck("doc-1", "Verify logs clean", &error), "duplicate check should fail");
    CHECK(error == "check_duplicate", "wrong error");
    PASS();
}

void test_is_deployment_ready_requires_sections_and_checks() {
    TEST(is_deployment_ready_requires_sections_and_checks);
    DocumentationOperatorPlaybooks p;
    std::string error;
    CHECK(p.addDocument("doc-1", "Deployment Guide", &error), "add doc failed");
    CHECK(!p.isDeploymentReady("doc-1"), "should not be ready yet");
    CHECK(p.addSection("doc-1", "Install", &error), "add section failed");
    CHECK(!p.isDeploymentReady("doc-1"), "should still not be ready");
    CHECK(p.addOperatorCheck("doc-1", "Verify logs clean", &error), "add check failed");
    CHECK(p.isDeploymentReady("doc-1"), "should be ready");
    PASS();
}

void test_coverage_score_combines_sections_and_checks() {
    TEST(coverage_score_combines_sections_and_checks);
    DocumentationOperatorPlaybooks p;
    std::string error;
    CHECK(p.addDocument("doc-1", "Deployment Guide", &error), "add doc failed");
    CHECK(p.addSection("doc-1", "Install", &error), "add section failed");
    CHECK(p.addSection("doc-1", "Troubleshooting", &error), "add section failed");
    CHECK(p.addOperatorCheck("doc-1", "Verify logs clean", &error), "add check failed");
    CHECK(p.coverageScore("doc-1") == 40, "coverage score mismatch");
    PASS();
}

void test_all_documents_sorted_by_doc_id() {
    TEST(all_documents_sorted_by_doc_id);
    DocumentationOperatorPlaybooks p;
    std::string error;
    CHECK(p.addDocument("doc-b", "B", &error), "add doc-b failed");
    CHECK(p.addDocument("doc-a", "A", &error), "add doc-a failed");
    auto docs = p.allDocuments();
    CHECK(docs[0].docId == "doc-a", "sorted order mismatch");
    PASS();
}

void test_unknown_doc_readiness_and_score_default() {
    TEST(unknown_doc_readiness_and_score_default);
    DocumentationOperatorPlaybooks p;
    CHECK(!p.isDeploymentReady("missing"), "missing doc should not be ready");
    CHECK(p.coverageScore("missing") == 0, "missing doc score should be zero");
    PASS();
}

int main() {
    std::cout << "Step 592: Documentation + Operator Playbooks\n";

    test_add_document_success();                       // 1
    test_add_document_rejects_missing_id();           // 2
    test_add_document_rejects_duplicate();            // 3
    test_add_section_success();                       // 4
    test_add_section_rejects_missing_doc();           // 5
    test_add_section_rejects_duplicate();             // 6
    test_add_operator_check_success();                // 7
    test_add_operator_check_rejects_duplicate();      // 8
    test_is_deployment_ready_requires_sections_and_checks(); // 9
    test_coverage_score_combines_sections_and_checks();// 10
    test_all_documents_sorted_by_doc_id();            // 11
    test_unknown_doc_readiness_and_score_default();   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
