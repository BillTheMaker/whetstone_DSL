// Step 246: Standalone MCP server entry point.
//
// Wires HeadlessEditorState (Step 245) + MCPBridge (Step 212) into a
// standalone `whetstone_mcp` binary that any MCP client can launch
// over stdio.  No SDL, no ImGui, no OpenGL.
//
// Usage: whetstone_mcp [--workspace <dir>] [--language <lang>] [--verbose]

#include "HeadlessEditorState.h"
#include "MCPBridge.h"
#include "MCPProjectConfig.h"
#include "AgentPermissionPolicy.h"
#include "ast/Serialization.h"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>

// -----------------------------------------------------------------------
//  CLI helpers (same pattern as pipeline_main.cpp)
// -----------------------------------------------------------------------

static std::string getArg(int argc, char** argv, const std::string& key) {
    for (int i = 1; i < argc - 1; ++i) {
        if (argv[i] == key) return argv[i + 1];
    }
    return "";
}

static bool hasFlag(int argc, char** argv, const std::string& key) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == key) return true;
    }
    return false;
}

// -----------------------------------------------------------------------
//  Signal handling (same pattern as orchestrator_main.cpp)
// -----------------------------------------------------------------------

static volatile sig_atomic_t g_running = 1;

static void signalHandler(int /*sig*/) {
    g_running = 0;
    exit(0);
}

// -----------------------------------------------------------------------
//  Resource reader — routes whetstone:// URIs to headless state
// -----------------------------------------------------------------------

static json readResource(HeadlessEditorState& state,
                          const std::string& uri) {
    if (uri == "whetstone://ast") {
        Module* ast = state.activeAST();
        if (ast) return toJson(ast);
        return {{"error", "No active AST"}};
    }
    if (uri == "whetstone://diagnostics") {
        return state.buildDiagnosticsJson();
    }
    if (uri == "whetstone://libraries") {
        // Return available functions from primitives registry
        state.library.primitives.setRoot(state.activeAST());
        state.library.primitives.setLanguage(state.defaultLanguage);
        auto funcs = state.library.primitives.getAvailableFunctions();
        json libs = json::array();
        for (const auto& p : funcs) {
            libs.push_back(json{{"name", p.name}, {"kind", p.kind},
                                {"source", p.source}});
        }
        return libs;
    }
    if (uri == "whetstone://annotations") {
        Module* ast = state.activeAST();
        if (!ast) return json::array();
        json annotations = json::array();
        for (auto* child : ast->allChildren()) {
            if (isAnnotationNode(child)) {
                annotations.push_back(toJson(child));
            }
        }
        return annotations;
    }
    if (uri == "whetstone://settings") {
        return {
            {"workspace", state.workspaceRoot},
            {"language", state.defaultLanguage},
            {"verbose", state.verbose},
            {"bufferCount", (int)state.bufferStates.size()},
            {"mode", "headless-mcp"}
        };
    }
    return {{"error", "Unknown resource: " + uri}};
}

// -----------------------------------------------------------------------
//  main
// -----------------------------------------------------------------------

int main(int argc, char** argv) {
    // Parse CLI args
    std::string workspace = getArg(argc, argv, "--workspace");
    std::string language  = getArg(argc, argv, "--language");
    bool verbose          = hasFlag(argc, argv, "--verbose");

    if (hasFlag(argc, argv, "--help") || hasFlag(argc, argv, "-h")) {
        std::cerr << "Usage: whetstone_mcp [--workspace <dir>] "
                     "[--language <lang>] [--verbose]\n";
        return 0;
    }

    // Signal handling
    signal(SIGINT,  signalHandler);
    signal(SIGTERM, signalHandler);

    // Create headless state
    HeadlessEditorState state;
    state.verbose = verbose;
    MCPProjectConfigLoadResult configLoad;
    if (!workspace.empty()) {
        configLoad = MCPProjectConfig::loadFromWorkspace(workspace);
        if (!configLoad.error.empty()) {
            std::cerr << "[whetstone-mcp] Config load warning: " << configLoad.error << "\n";
        }
    } else {
        configLoad = MCPProjectConfig::discoverFromCwd();
        if (!configLoad.error.empty()) {
            std::cerr << "[whetstone-mcp] Config discovery warning: " << configLoad.error << "\n";
        }
    }

    if (configLoad.found && !configLoad.config.workspace.empty()) {
        state.workspaceRoot = configLoad.config.workspace;
    } else if (!workspace.empty()) {
        state.workspaceRoot = workspace;
    } else if (configLoad.found) {
        state.workspaceRoot = configLoad.sourceWorkspaceRoot;
    }

    if (!language.empty()) {
        state.defaultLanguage = language;
    } else if (configLoad.found && !configLoad.config.defaultLanguage.empty()) {
        state.defaultLanguage = configLoad.config.defaultLanguage;
    }

    // Create an empty default buffer so getAST etc. work immediately
    state.openBuffer("(scratch)", "", state.defaultLanguage);

    AgentRole role = AgentRole::Refactor;
    if (configLoad.found && !configLoad.config.agentRole.empty()) {
        role = AgentPermissionPolicy::roleFromString(configLoad.config.agentRole);
    }
    state.setAgentRole("mcp-session", role);

    // Create bridge and wire handlers
    MCPBridge bridge;

    bridge.setRequestHandler([&state](const json& request) -> json {
        return state.processAgentRequest(request, "mcp-session");
    });

    bridge.setResourceReader([&state](const std::string& uri) -> json {
        return readResource(state, uri);
    });

    // Log startup info to stderr (stdout is the MCP transport)
    std::cerr << "[whetstone-mcp] Starting MCP server v0.1.0\n";
    if (!state.workspaceRoot.empty())
        std::cerr << "[whetstone-mcp] Workspace: " << state.workspaceRoot << "\n";
    if (configLoad.found && !configLoad.config.mcpWorkspaceAlias.empty())
        std::cerr << "[whetstone-mcp] Workspace alias: "
                  << configLoad.config.mcpWorkspaceAlias << "\n";
    std::cerr << "[whetstone-mcp] Language: " << state.defaultLanguage << "\n";
    if (verbose)
        std::cerr << "[whetstone-mcp] Verbose mode enabled\n";
    std::cerr << "[whetstone-mcp] Listening on stdio...\n";

    // Run the MCP stdio transport loop
    bridge.runStdio();

    return 0;
}
