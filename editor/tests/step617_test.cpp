// Step 617: MCP initialize instructions field (12 tests)

#include "MCPServer.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static json initialize(MCPServer& mcp) {
    json req = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "initialize"},
        {"params", {{"protocolVersion", "2024-11-05"}}}
    };
    return mcp.handleRequest(req);
}

static std::string readPrompt() {
    std::ifstream in("tools/claude/system_prompt.txt");
    if (!in.is_open()) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void test_initialize_returns_result_object() {
    TEST(initialize_returns_result_object);
    MCPServer mcp;
    json resp = initialize(mcp);
    CHECK(resp.contains("result"), "missing result");
    PASS();
}

void test_initialize_protocol_version_unchanged() {
    TEST(initialize_protocol_version_unchanged);
    MCPServer mcp;
    json resp = initialize(mcp);
    CHECK(resp["result"].value("protocolVersion", "") == "2024-11-05", "wrong protocol version");
    PASS();
}

void test_initialize_includes_instructions_field() {
    TEST(initialize_includes_instructions_field);
    MCPServer mcp;
    json resp = initialize(mcp);
    CHECK(resp["result"].contains("instructions"), "instructions field missing");
    CHECK(resp["result"]["instructions"].is_string(), "instructions should be string");
    PASS();
}

void test_instructions_not_empty() {
    TEST(instructions_not_empty);
    MCPServer mcp;
    json resp = initialize(mcp);
    std::string instructions = resp["result"].value("instructions", "");
    CHECK(!instructions.empty(), "instructions should be non-empty");
    PASS();
}

void test_instructions_include_available_tools_header() {
    TEST(instructions_include_available_tools_header);
    MCPServer mcp;
    json resp = initialize(mcp);
    std::string instructions = resp["result"].value("instructions", "");
    CHECK(instructions.find("Available tools:") != std::string::npos, "missing tools header");
    PASS();
}

void test_instructions_include_core_tool_name() {
    TEST(instructions_include_core_tool_name);
    MCPServer mcp;
    json resp = initialize(mcp);
    std::string instructions = resp["result"].value("instructions", "");
    CHECK(instructions.find("whetstone_get_ast") != std::string::npos, "missing whetstone_get_ast");
    PASS();
}

void test_instructions_include_guidance_line() {
    TEST(instructions_include_guidance_line);
    MCPServer mcp;
    json resp = initialize(mcp);
    std::string instructions = resp["result"].value("instructions", "");
    CHECK(instructions.find("You must follow user instructions carefully") != std::string::npos,
          "missing guidance line");
    PASS();
}

void test_instructions_match_system_prompt_file_when_available() {
    TEST(instructions_match_system_prompt_file_when_available);
    MCPServer mcp;
    json resp = initialize(mcp);
    std::string expected = readPrompt();
    std::string instructions = resp["result"].value("instructions", "");
    CHECK(!expected.empty(), "expected prompt file should be readable");
    CHECK(instructions == expected, "initialize instructions should match prompt file");
    PASS();
}

void test_server_info_still_present() {
    TEST(server_info_still_present);
    MCPServer mcp;
    json resp = initialize(mcp);
    CHECK(resp["result"].contains("serverInfo"), "missing serverInfo");
    CHECK(resp["result"]["serverInfo"].value("name", "") == "whetstone-mcp", "wrong server name");
    PASS();
}

void test_tools_list_still_available_after_initialize() {
    TEST(tools_list_still_available_after_initialize);
    MCPServer mcp;
    (void)initialize(mcp);
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    json resp = mcp.handleRequest(req);
    CHECK(resp.contains("result"), "missing result");
    CHECK(resp["result"].contains("tools"), "missing tools");
    CHECK(!resp["result"]["tools"].empty(), "tools should not be empty");
    PASS();
}

void test_notifications_initialized_still_no_response() {
    TEST(notifications_initialized_still_no_response);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"method", "notifications/initialized"}};
    json resp = mcp.handleRequest(req);
    CHECK(resp.is_null(), "notification should return null response");
    PASS();
}

void test_instructions_do_not_use_fallback_message() {
    TEST(instructions_do_not_use_fallback_message);
    MCPServer mcp;
    json resp = initialize(mcp);
    std::string instructions = resp["result"].value("instructions", "");
    CHECK(instructions.find("System prompt unavailable") == std::string::npos,
          "fallback message should not appear");
    PASS();
}

int main() {
    std::cout << "Step 617: MCP initialize instructions field\n";

    test_initialize_returns_result_object();                    // 1
    test_initialize_protocol_version_unchanged();               // 2
    test_initialize_includes_instructions_field();              // 3
    test_instructions_not_empty();                              // 4
    test_instructions_include_available_tools_header();         // 5
    test_instructions_include_core_tool_name();                 // 6
    test_instructions_include_guidance_line();                  // 7
    test_instructions_match_system_prompt_file_when_available();// 8
    test_server_info_still_present();                           // 9
    test_tools_list_still_available_after_initialize();         // 10
    test_notifications_initialized_still_no_response();         // 11
    test_instructions_do_not_use_fallback_message();            // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
