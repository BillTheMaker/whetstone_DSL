#pragma once
// Step 284-286: Environment Layer
//
// EnvironmentSpec — first-class environment model attached to modules.
// Capability vocabulary, validation, and environment-aware diagnostics.

#include "ast/ASTNode.h"
#include "ast/Annotation.h"
#include "ast/Serialization.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <set>

using json = nlohmann::json;

// --- EnvironmentSpec AST Node ---
class EnvironmentSpec : public ASTNode {
public:
    std::string envId;          // "posix_process", "browser", "jvm", etc.
    std::string envVersion;
    std::vector<std::string> capabilities;
    std::vector<std::string> constraints;
    std::string scheduler;      // "event_loop"|"threads"|"fibers"|"coroutines"|"single_thread"
    std::string memory;         // "manual"|"raii"|"refcount"|"tracing_gc"|"region"
    json bindingTimes;          // per-feature: "compile"|"link"|"load"|"runtime"
    std::string exceptions;     // "unwind"|"checked"|"typed"|"untyped"
    std::string ffi;            // "c_abi"|"dynamic_linking"|"none"

    EnvironmentSpec() {
        conceptType = "EnvironmentSpec";
        bindingTimes = json::object();
    }
};

// --- CapabilityRequirement Annotation ---
class CapabilityRequirement : public Annotation {
public:
    std::string capability;
    bool required = true;
    CapabilityRequirement() { conceptType = "CapabilityRequirement"; }
};

// --- Capability Vocabulary ---
inline const std::set<std::string>& knownCapabilities() {
    static const std::set<std::string> caps = {
        "io.fs", "io.net", "io.stdinout",
        "clock.time", "timers",
        "event_loop", "threads", "process", "signals",
        "modules", "dynamic_loading", "reflection",
        "gc", "jit",
        "ui.dom", "editor.emacs", "host.imgui"
    };
    return caps;
}

inline bool isKnownCapability(const std::string& cap) {
    return knownCapabilities().count(cap) > 0;
}

// --- Capability Validation ---
struct CapabilityDiagnostic {
    std::string severity;  // "error" | "warning"
    std::string message;
    std::string nodeId;
    std::string capability;
};

inline std::vector<CapabilityDiagnostic> validateCapabilities(
    const ASTNode* root, const EnvironmentSpec* env)
{
    std::vector<CapabilityDiagnostic> diags;
    if (!root || !env) return diags;

    std::set<std::string> envCaps(env->capabilities.begin(), env->capabilities.end());

    // Collect all CapabilityRequirements from the tree
    std::vector<const ASTNode*> toVisit = {root};
    while (!toVisit.empty()) {
        const ASTNode* node = toVisit.back();
        toVisit.pop_back();
        for (const auto* child : node->allChildren()) {
            if (child->conceptType == "CapabilityRequirement") {
                auto* cr = static_cast<const CapabilityRequirement*>(child);
                if (cr->required && envCaps.find(cr->capability) == envCaps.end()) {
                    diags.push_back({
                        "error",
                        "E0501: Unmet capability requirement: " + cr->capability,
                        node->id,
                        cr->capability
                    });
                }
            }
            toVisit.push_back(child);
        }
    }
    return diags;
}

// --- Environment-Incompatible Annotation Diagnostics (Step 286) ---
inline std::vector<CapabilityDiagnostic> validateEnvAnnotations(
    const ASTNode* root, const EnvironmentSpec* env)
{
    std::vector<CapabilityDiagnostic> diags;
    if (!root || !env) return diags;

    std::vector<const ASTNode*> toVisit = {root};
    while (!toVisit.empty()) {
        const ASTNode* node = toVisit.back();
        toVisit.pop_back();
        for (const auto* child : node->allChildren()) {
            // ParallelAnnotation(data) in single_thread env
            if (child->conceptType == "ParallelAnnotation" &&
                env->scheduler == "single_thread") {
                auto* pa = static_cast<const ParallelAnnotation*>(child);
                diags.push_back({
                    "error",
                    "E0502: @Parallel(" + pa->kind + ") incompatible with single_thread scheduler",
                    node->id, ""
                });
            }
            // ThreadModelAnnotation(os) in single_thread env
            if (child->conceptType == "ThreadModelAnnotation" &&
                env->scheduler == "single_thread") {
                diags.push_back({
                    "error",
                    "E0503: @ThreadModel incompatible with single_thread scheduler",
                    node->id, ""
                });
            }
            // AtomicAnnotation in single_thread env (warning, not error)
            if (child->conceptType == "AtomicAnnotation" &&
                env->scheduler == "single_thread") {
                diags.push_back({
                    "warning",
                    "E0504: @Atomic unnecessary in single_thread environment",
                    node->id, ""
                });
            }
            // ExecAnnotation(async) requires event_loop, threads, fibers, or coroutines
            if (child->conceptType == "ExecAnnotation") {
                auto* ea = static_cast<const ExecAnnotation*>(child);
                if (ea->mode == "async" && env->scheduler == "single_thread") {
                    diags.push_back({
                        "error",
                        "E0505: @Exec(async) incompatible with single_thread scheduler",
                        node->id, ""
                    });
                }
            }
            toVisit.push_back(child);
        }
    }
    return diags;
}

// --- Environment-Aware Lowering Hints (Step 287) ---
struct LoweringHint {
    std::string pattern;     // e.g. "callback", "std_async", "try_catch"
    std::string description;
};

inline std::vector<LoweringHint> getLoweringHints(
    const ASTNode* node, const EnvironmentSpec* env)
{
    std::vector<LoweringHint> hints;
    if (!node || !env) return hints;

    for (const auto* child : node->getChildren("annotations")) {
        if (child->conceptType == "ExecAnnotation") {
            auto* ea = static_cast<const ExecAnnotation*>(child);
            if (ea->mode == "async") {
                if (env->scheduler == "event_loop")
                    hints.push_back({"callback", "async → callback-based pattern"});
                else if (env->scheduler == "threads")
                    hints.push_back({"std_async", "async → std::async/std::future"});
                else if (env->scheduler == "coroutines")
                    hints.push_back({"coroutine", "async → co_await coroutine"});
            }
        }
    }

    // Memory model hints
    if (env->memory == "tracing_gc")
        hints.push_back({"suppress_dealloc", "GC env → suppress explicit delete"});
    else if (env->memory == "manual")
        hints.push_back({"explicit_delete", "manual env → inject explicit delete"});
    else if (env->memory == "raii")
        hints.push_back({"raii_cleanup", "RAII env → destructor-based cleanup"});

    // Exception model hints
    if (env->exceptions == "unwind")
        hints.push_back({"try_catch", "unwind → try/catch blocks"});
    else if (env->exceptions == "typed")
        hints.push_back({"expected", "typed → std::expected return types"});
    else if (env->exceptions == "checked")
        hints.push_back({"checked_throw", "checked → explicit throw declarations"});

    return hints;
}

// --- EnvironmentSpec JSON serialization helpers ---
inline json envSpecToJson(const EnvironmentSpec* env) {
    if (!env) return json();
    json j;
    j["envId"] = env->envId;
    if (!env->envVersion.empty()) j["envVersion"] = env->envVersion;
    if (!env->capabilities.empty()) j["capabilities"] = env->capabilities;
    if (!env->constraints.empty()) j["constraints"] = env->constraints;
    if (!env->scheduler.empty()) j["scheduler"] = env->scheduler;
    if (!env->memory.empty()) j["memory"] = env->memory;
    if (!env->bindingTimes.empty()) j["bindingTimes"] = env->bindingTimes;
    if (!env->exceptions.empty()) j["exceptions"] = env->exceptions;
    if (!env->ffi.empty()) j["ffi"] = env->ffi;
    return j;
}

inline void envSpecFromJson(EnvironmentSpec* env, const json& j) {
    if (!env) return;
    if (j.contains("envId")) env->envId = j["envId"].get<std::string>();
    if (j.contains("envVersion")) env->envVersion = j["envVersion"].get<std::string>();
    if (j.contains("capabilities") && j["capabilities"].is_array()) {
        env->capabilities.clear();
        for (const auto& c : j["capabilities"])
            if (c.is_string()) env->capabilities.push_back(c.get<std::string>());
    }
    if (j.contains("constraints") && j["constraints"].is_array()) {
        env->constraints.clear();
        for (const auto& c : j["constraints"])
            if (c.is_string()) env->constraints.push_back(c.get<std::string>());
    }
    if (j.contains("scheduler")) env->scheduler = j["scheduler"].get<std::string>();
    if (j.contains("memory")) env->memory = j["memory"].get<std::string>();
    if (j.contains("bindingTimes")) env->bindingTimes = j["bindingTimes"];
    if (j.contains("exceptions")) env->exceptions = j["exceptions"].get<std::string>();
    if (j.contains("ffi")) env->ffi = j["ffi"].get<std::string>();
}
