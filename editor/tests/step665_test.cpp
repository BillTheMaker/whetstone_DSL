// Step 665: whetstone_generate_dispatch_table MCP tool (12 tests)
// Tests JobDispatchTableGenerator directly — same logic the MCP handler calls.

#include "JobDispatchTableGenerator.h"
#include <iostream>

static int p = 0, f = 0;
#define T(n)    { std::cout << "  " << #n << "... "; }
#define P()     { std::cout << "PASS\n"; ++p; }
#define F(m)    { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m)  if (!(c)) { F(m); return; }

static std::vector<JobDispatchEntrySpec> hivemindEntries() {
    return {
        {"agent_swarm",  {"Claude_Agent", "Agent_Host"}, "AgentPayload",   "handleAgentSwarm"},
        {"cuda_task",    {"Cuda_Compute"},               "CudaPayload",    "handleCudaTask"},
        {"compile_job",  {"Compilation"},                "CompilePayload", "handleCompileJob"},
        {"shell_script", {},                             "ShellPayload",   "handleShellScript"}
    };
}

void t1() {
    T(single_entry_produces_valid_header);
    auto out = JobDispatchTableGenerator::generate({{"cuda_task", {}, "CudaPayload", "handleCuda"}});
    C(out.success, "expected success");
    C(!out.headerCode.empty(), "expected non-empty header");
    P();
}

void t2() {
    T(multiple_entries_all_appear_in_output);
    auto out = JobDispatchTableGenerator::generate(hivemindEntries());
    C(out.success, "expected success");
    C(out.headerCode.find("agent_swarm") != std::string::npos, "agent_swarm missing");
    C(out.headerCode.find("cuda_task") != std::string::npos, "cuda_task missing");
    C(out.headerCode.find("compile_job") != std::string::npos, "compile_job missing");
    C(out.headerCode.find("shell_script") != std::string::npos, "shell_script missing");
    P();
}

void t3() {
    T(empty_entries_returns_error);
    auto out = JobDispatchTableGenerator::generate({});
    C(!out.success, "expected failure on empty");
    C(!out.errors.empty(), "expected error message");
    C(out.errors[0] == "entries_required", "expected entries_required error");
    P();
}

void t4() {
    T(header_contains_pragma_once);
    auto out = JobDispatchTableGenerator::generate(hivemindEntries());
    C(out.success, "expected success");
    C(out.headerCode.find("#pragma once") != std::string::npos, "missing #pragma once");
    P();
}

void t5() {
    T(header_contains_make_dispatch_table);
    auto out = JobDispatchTableGenerator::generate(hivemindEntries());
    C(out.success, "expected success");
    C(out.headerCode.find("makeDispatchTable") != std::string::npos, "missing makeDispatchTable");
    P();
}

void t6() {
    T(required_caps_appear_as_comments);
    auto out = JobDispatchTableGenerator::generate(hivemindEntries());
    C(out.success, "expected success");
    C(out.headerCode.find("Claude_Agent") != std::string::npos, "caps comment missing");
    P();
}

void t7() {
    T(executor_appears_as_table_value);
    auto out = JobDispatchTableGenerator::generate({{"my_job", {}, "", "myExecutorFn"}});
    C(out.success, "expected success");
    C(out.headerCode.find("myExecutorFn") != std::string::npos, "executor missing from output");
    P();
}

void t8() {
    T(job_type_appears_as_string_key);
    auto out = JobDispatchTableGenerator::generate({{"my_unique_job_type", {}, "", "fn"}});
    C(out.success, "expected success");
    C(out.headerCode.find("\"my_unique_job_type\"") != std::string::npos, "job_type key missing");
    P();
}

void t9() {
    T(output_contains_braces);
    auto out = JobDispatchTableGenerator::generate(hivemindEntries());
    C(out.success, "expected success");
    C(out.headerCode.find("{") != std::string::npos, "missing opening brace");
    C(out.headerCode.find("}") != std::string::npos, "missing closing brace");
    P();
}

void t10() {
    T(header_contains_executor_fn_type);
    auto out = JobDispatchTableGenerator::generate(hivemindEntries());
    C(out.success, "expected success");
    C(out.headerCode.find("ExecutorFn") != std::string::npos, "missing ExecutorFn type");
    P();
}

void t11() {
    T(header_contains_unordered_map);
    auto out = JobDispatchTableGenerator::generate(hivemindEntries());
    C(out.success, "expected success");
    C(out.headerCode.find("unordered_map") != std::string::npos, "missing unordered_map");
    P();
}

void t12() {
    T(no_errors_on_success);
    auto out = JobDispatchTableGenerator::generate(hivemindEntries());
    C(out.success, "expected success");
    C(out.errors.empty(), "expected no errors on success");
    P();
}

int main() {
    std::cout << "Step 665: whetstone_generate_dispatch_table\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << p << "/" << (p + f) << " passed\n";
    return f ? 1 : 0;
}
