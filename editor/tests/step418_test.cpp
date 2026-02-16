// Step 418: Workflow Prompt Templates Tests (12 tests)

#include "MCPWorkflowPrompts.h"

#include <filesystem>
#include <iostream>
#include <map>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_templates_include_required_names() {
    TEST(templates_include_required_names);
    CHECK(MCPWorkflowPrompts::hasTemplate("architect_project"), "missing architect_project");
    CHECK(MCPWorkflowPrompts::hasTemplate("modernize_module"), "missing modernize_module");
    CHECK(MCPWorkflowPrompts::hasTemplate("port_to_language"), "missing port_to_language");
    CHECK(MCPWorkflowPrompts::hasTemplate("add_feature"), "missing add_feature");
    CHECK(MCPWorkflowPrompts::hasTemplate("review_pending"), "missing review_pending");
    PASS();
}

void test_architect_project_tools_present() {
    TEST(architect_project_tools_present);
    auto t = MCPWorkflowPrompts::getTemplate("architect_project");
    CHECK(t.userPrompt.find("whetstone_infer_annotations") != std::string::npos, "missing infer tool");
    CHECK(t.userPrompt.find("whetstone_create_skeleton") != std::string::npos, "missing skeleton tool");
    CHECK(t.userPrompt.find("whetstone_create_workflow") != std::string::npos, "missing workflow tool");
    PASS();
}

void test_modernize_module_tools_present() {
    TEST(modernize_module_tools_present);
    auto t = MCPWorkflowPrompts::getTemplate("modernize_module");
    CHECK(t.userPrompt.find("whetstone_orchestrate_run_deterministic") != std::string::npos,
          "missing deterministic run tool");
    CHECK(t.userPrompt.find("whetstone_get_blockers") != std::string::npos, "missing blockers tool");
    CHECK(t.userPrompt.find("whetstone_submit_result") != std::string::npos, "missing submit tool");
    PASS();
}

void test_port_template_mentions_source_target() {
    TEST(port_template_mentions_source_target);
    auto rendered = MCPWorkflowPrompts::renderPrompt(
        "port_to_language",
        {{"source_language", "python"}, {"target_language", "rust"}});
    CHECK(rendered.find("python") != std::string::npos, "missing source language replacement");
    CHECK(rendered.find("rust") != std::string::npos, "missing target language replacement");
    PASS();
}

void test_add_feature_template_progress_tools_present() {
    TEST(add_feature_template_progress_tools_present);
    auto t = MCPWorkflowPrompts::getTemplate("add_feature");
    CHECK(t.userPrompt.find("whetstone_create_workflow") != std::string::npos, "missing workflow");
    CHECK(t.userPrompt.find("whetstone_get_progress") != std::string::npos, "missing progress");
    PASS();
}

void test_review_pending_template_review_tools_present() {
    TEST(review_pending_template_review_tools_present);
    auto t = MCPWorkflowPrompts::getTemplate("review_pending");
    CHECK(t.userPrompt.find("whetstone_get_review_queue") != std::string::npos, "missing review queue");
    CHECK(t.userPrompt.find("whetstone_get_review_context") != std::string::npos, "missing review context");
    CHECK(t.userPrompt.find("whetstone_approve_item") != std::string::npos, "missing approve");
    CHECK(t.userPrompt.find("whetstone_reject_item") != std::string::npos, "missing reject");
    PASS();
}

void test_render_prompt_substitutes_project_name() {
    TEST(render_prompt_substitutes_project_name);
    std::string rendered = MCPWorkflowPrompts::renderPrompt(
        "architect_project", {{"project_name", "AlphaRepo"}});
    CHECK(rendered.find("AlphaRepo") != std::string::npos, "project_name not substituted");
    PASS();
}

void test_render_prompt_keeps_unprovided_placeholders() {
    TEST(render_prompt_keeps_unprovided_placeholders);
    std::string rendered = MCPWorkflowPrompts::renderPrompt("add_feature", {});
    CHECK(rendered.find("{{feature_name}}") != std::string::npos,
          "unprovided placeholder should remain explicit");
    PASS();
}

void test_validate_rejects_empty_tools() {
    TEST(validate_rejects_empty_tools);
    MCPWorkflowPromptTemplate t;
    t.name = "bad";
    t.userPrompt = "Protocol:\n1. do a thing";
    std::string err;
    CHECK(!MCPWorkflowPrompts::validate(t, err), "expected validation failure");
    CHECK(!err.empty(), "expected error message");
    PASS();
}

void test_manifest_json_contains_all_templates() {
    TEST(manifest_json_contains_all_templates);
    json m = MCPWorkflowPrompts::manifestJson();
    CHECK(m.contains("templates"), "missing templates field");
    CHECK(m["templates"].is_array(), "templates should be array");
    CHECK(m["templates"].size() == 5, "expected five templates");
    PASS();
}

void test_write_template_files_to_directory() {
    TEST(write_template_files_to_directory);
    auto outDir = std::filesystem::temp_directory_path() / "whetstone_step418_prompts";
    CHECK(MCPWorkflowPrompts::writeTemplateFiles(outDir.string()), "writeTemplateFiles failed");
    CHECK(std::filesystem::exists(outDir / "architect_project.prompt"), "missing architect prompt file");
    CHECK(std::filesystem::exists(outDir / "review_pending.prompt"), "missing review prompt file");
    PASS();
}

void test_all_templates_validate_successfully() {
    TEST(all_templates_validate_successfully);
    for (const auto& t : MCPWorkflowPrompts::templates()) {
        std::string err;
        CHECK(MCPWorkflowPrompts::validate(t, err), "template validation failed");
    }
    PASS();
}

int main() {
    std::cout << "Step 418: Workflow Prompt Templates Tests\n";

    test_templates_include_required_names();             // 1
    test_architect_project_tools_present();              // 2
    test_modernize_module_tools_present();               // 3
    test_port_template_mentions_source_target();         // 4
    test_add_feature_template_progress_tools_present();  // 5
    test_review_pending_template_review_tools_present(); // 6
    test_render_prompt_substitutes_project_name();       // 7
    test_render_prompt_keeps_unprovided_placeholders();  // 8
    test_validate_rejects_empty_tools();                 // 9
    test_manifest_json_contains_all_templates();         // 10
    test_write_template_files_to_directory();            // 11
    test_all_templates_validate_successfully();          // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
