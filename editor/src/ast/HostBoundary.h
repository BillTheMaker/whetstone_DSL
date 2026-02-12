#pragma once
// Step 288: Host Boundary AST Nodes
//
// Explicit nodes for host-environment interactions:
// HostCall, ScheduleTask, ModuleLoad

#include "ASTNode.h"
#include "Expression.h"
#include "Statement.h"
#include <string>

// HostCall — capability-namespaced host function call
class HostCall : public Expression {
public:
    std::string capability;  // e.g. "io.fs", "io.net"
    std::string name;        // e.g. "readFile", "listen"

    HostCall() { conceptType = "HostCall"; }
    HostCall(const std::string& cap, const std::string& n)
        : capability(cap), name(n) { conceptType = "HostCall"; }
};

// ScheduleTask — dispatches work to a queue
class ScheduleTask : public Statement {
public:
    std::string queue;  // "microtask" | "macrotask" | "thread"

    ScheduleTask() { conceptType = "ScheduleTask"; }
    // Body statements via children role "body"
};

// ModuleLoad — static or dynamic module import
class ModuleLoad : public Statement {
public:
    std::string moduleName;
    std::string mode;  // "static" | "dynamic"

    ModuleLoad() { conceptType = "ModuleLoad"; }
};
