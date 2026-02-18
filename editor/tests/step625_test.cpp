// Step 625: Tool call visualization (12 tests)

#include "AgentChatPanelModel.h"
#include "AgentToolCallVisualization.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_build_sets_tool_name() {
    TEST(build_sets_tool_name);
    auto view = AgentToolCallVisualization::build("whetstone_mutate", json::object(), json::object());
    CHECK(view.toolName == "whetstone_mutate", "tool name mismatch");
    PASS();
}

void test_build_serializes_input_json() {
    TEST(build_serializes_input_json);
    auto view = AgentToolCallVisualization::build("whetstone_mutate",
                                                  {{"type", "setProperty"}},
                                                  json::object());
    CHECK(view.inputJson.find("setProperty") != std::string::npos, "input json missing expected token");
    PASS();
}

void test_build_serializes_output_json() {
    TEST(build_serializes_output_json);
    auto view = AgentToolCallVisualization::build("whetstone_mutate",
                                                  json::object(),
                                                  {{"ok", true}});
    CHECK(view.outputJson.find("\"ok\": true") != std::string::npos, "output json missing expected token");
    PASS();
}

void test_mutate_summary_includes_property_path() {
    TEST(mutate_summary_includes_property_path);
    auto view = AgentToolCallVisualization::build("whetstone_mutate",
                                                  {{"type", "setProperty"},
                                                   {"nodeId", "fn1"},
                                                   {"property", "name"},
                                                   {"value", "newName"}},
                                                  json::object());
    CHECK(view.summary.find("fn1.name") != std::string::npos, "summary missing path");
    CHECK(view.summary.find("newName") != std::string::npos, "summary missing value");
    PASS();
}

void test_generate_code_summary_truncates_long_spec() {
    TEST(generate_code_summary_truncates_long_spec);
    std::string longSpec(80, 'x');
    auto view = AgentToolCallVisualization::build("whetstone_generate_code",
                                                  {{"spec", longSpec}},
                                                  json::object());
    CHECK(view.summary.find("...") != std::string::npos, "summary should be truncated");
    PASS();
}

void test_pipeline_summary_includes_languages() {
    TEST(pipeline_summary_includes_languages);
    auto view = AgentToolCallVisualization::build("whetstone_run_pipeline",
                                                  {{"sourceLanguage", "python"},
                                                   {"targetLanguage", "cpp"}},
                                                  json::object());
    CHECK(view.summary.find("python -> cpp") != std::string::npos, "language summary mismatch");
    PASS();
}

void test_unknown_tool_uses_call_summary() {
    TEST(unknown_tool_uses_call_summary);
    auto view = AgentToolCallVisualization::build("whetstone_unknown",
                                                  json::object(),
                                                  json::object());
    CHECK(view.summary == "call", "unexpected summary");
    PASS();
}

void test_inline_label_uses_collapsed_chevron_by_default() {
    TEST(inline_label_uses_collapsed_chevron_by_default);
    auto view = AgentToolCallVisualization::build("whetstone_mutate", json::object(), json::object());
    std::string label = AgentToolCallVisualization::inlineLabel(view);
    CHECK(label.find(">") != std::string::npos, "collapsed chevron missing");
    PASS();
}

void test_toggle_expanded_updates_label_chevron() {
    TEST(toggle_expanded_updates_label_chevron);
    auto view = AgentToolCallVisualization::build("whetstone_mutate", json::object(), json::object());
    AgentToolCallVisualization::toggleExpanded(&view);
    std::string label = AgentToolCallVisualization::inlineLabel(view);
    CHECK(label.find("v") != std::string::npos, "expanded chevron missing");
    PASS();
}

void test_chat_add_tool_call_appends_message() {
    TEST(chat_add_tool_call_appends_message);
    AgentChatState state;
    AgentChatPanelModel::addToolCall(&state,
                                     "whetstone_mutate",
                                     {{"type", "setProperty"}, {"nodeId", "fn1"}},
                                     {{"ok", true}},
                                     "11:00");
    CHECK(state.messages.size() == 1, "message should be added");
    CHECK(state.messages[0].role == AgentChatRole::Tool, "message role should be tool");
    PASS();
}

void test_chat_add_tool_call_appends_tool_record() {
    TEST(chat_add_tool_call_appends_tool_record);
    AgentChatState state;
    AgentChatPanelModel::addToolCall(&state,
                                     "whetstone_run_pipeline",
                                     {{"sourceLanguage", "python"}, {"targetLanguage", "rust"}},
                                     {{"generatedCode", "fn main() {}"}},
                                     "11:01");
    CHECK(state.toolCalls.size() == 1, "tool call record should be added");
    CHECK(state.toolCalls[0].toolName == "whetstone_run_pipeline", "tool call name mismatch");
    PASS();
}

void test_chat_tool_message_contains_inline_tool_label() {
    TEST(chat_tool_message_contains_inline_tool_label);
    AgentChatState state;
    AgentChatPanelModel::addToolCall(&state,
                                     "whetstone_mutate",
                                     {{"type", "setProperty"}, {"nodeId", "fn1"}, {"property", "name"}, {"value", "x"}},
                                     {{"ok", true}},
                                     "11:02");
    CHECK(state.messages[0].content.find("[TOOL: whetstone_mutate]") != std::string::npos,
          "tool inline label missing");
    PASS();
}

int main() {
    std::cout << "Step 625: Tool call visualization\n";

    test_build_sets_tool_name();                              // 1
    test_build_serializes_input_json();                       // 2
    test_build_serializes_output_json();                      // 3
    test_mutate_summary_includes_property_path();             // 4
    test_generate_code_summary_truncates_long_spec();         // 5
    test_pipeline_summary_includes_languages();               // 6
    test_unknown_tool_uses_call_summary();                    // 7
    test_inline_label_uses_collapsed_chevron_by_default();    // 8
    test_toggle_expanded_updates_label_chevron();             // 9
    test_chat_add_tool_call_appends_message();                // 10
    test_chat_add_tool_call_appends_tool_record();            // 11
    test_chat_tool_message_contains_inline_tool_label();      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
