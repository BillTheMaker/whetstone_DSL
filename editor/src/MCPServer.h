#pragma once
// Step 207-211: MCP Server for Whetstone
//
// Implements the Model Context Protocol (MCP) over stdio (JSON-RPC 2.0).
// Exposes Whetstone agent API as MCP tools, resources, and prompts so
// LLMs (Claude, Codex, etc.) can interact with the editor natively.
//
// Architecture: MCPServer holds tool/resource/prompt registrations and
// dispatches incoming JSON-RPC requests. A bridge function translates
// MCP tool calls to Whetstone JSON-RPC requests (via a callback).

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// -----------------------------------------------------------------------
//  MCP Tool definition
// -----------------------------------------------------------------------
struct MCPTool {
    std::string name;
    std::string description;
    json inputSchema;  // JSON Schema for tool input
};

// -----------------------------------------------------------------------
//  MCP Resource definition
// -----------------------------------------------------------------------
struct MCPResource {
    std::string uri;
    std::string name;
    std::string description;
    std::string mimeType;
};

// -----------------------------------------------------------------------
//  MCP Prompt definition
// -----------------------------------------------------------------------
struct MCPPromptArgument {
    std::string name;
    std::string description;
    bool required = false;
};

struct MCPPrompt {
    std::string name;
    std::string description;
    std::vector<MCPPromptArgument> arguments;
};

struct MCPPromptMessage {
    std::string role;  // "user" or "assistant"
    json content;      // {type: "text", text: "..."}
};

// -----------------------------------------------------------------------
//  MCP Server
// -----------------------------------------------------------------------
class MCPServer {
public:
    // Callback type: given a Whetstone JSON-RPC request, returns a response.
    using RpcCallback = std::function<json(const json& request)>;

    MCPServer() {
        registerWhetstoneTools();
        registerWhetstoneResources();
        registerWhetstonePrompts();
    }

    void setRpcCallback(RpcCallback cb) { rpcCallback_ = std::move(cb); }

    // ---------------------------------------------------------------
    //  Handle a single MCP JSON-RPC request
    // ---------------------------------------------------------------
    json handleRequest(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);

        std::string method = request.value("method", "");

        if (method == "initialize") return handleInitialize(request);
        if (method == "notifications/initialized") return json(nullptr); // no response
        if (method == "ping") return handlePing(request);
        if (method == "tools/list") return handleToolsList(request);
        if (method == "tools/call") return handleToolsCall(request);
        if (method == "resources/list") return handleResourcesList(request);
        if (method == "resources/read") return handleResourcesRead(request);
        if (method == "prompts/list") return handlePromptsList(request);
        if (method == "prompts/get") return handlePromptsGet(request);

        response["error"] = {{"code", -32601}, {"message", "Method not found: " + method}};
        return response;
    }

    // ---------------------------------------------------------------
    //  Accessors for testing
    // ---------------------------------------------------------------
    const std::vector<MCPTool>& getTools() const { return tools_; }
    const std::vector<MCPResource>& getResources() const { return resources_; }
    const std::vector<MCPPrompt>& getPrompts() const { return prompts_; }
    bool isInitialized() const { return initialized_; }

    // Resource read handler registration
    using ResourceReader = std::function<json(const std::string& uri)>;
    void setResourceReader(ResourceReader reader) { resourceReader_ = std::move(reader); }

private:
    std::vector<MCPTool> tools_;
    std::vector<MCPResource> resources_;
    std::vector<MCPPrompt> prompts_;
    std::map<std::string, std::function<json(const json&)>> toolHandlers_;
    RpcCallback rpcCallback_;
    ResourceReader resourceReader_;
    bool initialized_ = false;

    // ---------------------------------------------------------------
    //  Protocol handlers
    // ---------------------------------------------------------------

    json handleInitialize(const json& request) {
        initialized_ = true;
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);
        response["result"] = {
            {"protocolVersion", "2024-11-05"},
            {"capabilities", {
                {"tools", json::object()},
                {"resources", json::object()},
                {"prompts", json::object()}
            }},
            {"serverInfo", {
                {"name", "whetstone-mcp"},
                {"version", "0.1.0"}
            }}
        };
        return response;
    }

    json handlePing(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);
        response["result"] = json::object();
        return response;
    }

    json handleToolsList(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);
        json toolArr = json::array();
        for (const auto& t : tools_) {
            toolArr.push_back({
                {"name", t.name},
                {"description", t.description},
                {"inputSchema", t.inputSchema}
            });
        }
        response["result"] = {{"tools", toolArr}};
        return response;
    }

    json handleToolsCall(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);

        auto params = request.contains("params") ? request["params"] : json::object();
        std::string toolName = params.value("name", "");
        json args = params.contains("arguments") ? params["arguments"] : json::object();

        auto it = toolHandlers_.find(toolName);
        if (it == toolHandlers_.end()) {
            response["result"] = {
                {"content", json::array({{{"type", "text"}, {"text", "Unknown tool: " + toolName}}})},
                {"isError", true}
            };
            return response;
        }

        try {
            json result = it->second(args);
            std::string text = result.dump(2);
            response["result"] = {
                {"content", json::array({{{"type", "text"}, {"text", text}}})},
                {"isError", false}
            };
        } catch (const std::exception& e) {
            response["result"] = {
                {"content", json::array({{{"type", "text"}, {"text", std::string("Error: ") + e.what()}}})},
                {"isError", true}
            };
        }
        return response;
    }

    json handleResourcesList(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);
        json arr = json::array();
        for (const auto& r : resources_) {
            arr.push_back({
                {"uri", r.uri},
                {"name", r.name},
                {"description", r.description},
                {"mimeType", r.mimeType}
            });
        }
        response["result"] = {{"resources", arr}};
        return response;
    }

    json handleResourcesRead(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);

        auto params = request.contains("params") ? request["params"] : json::object();
        std::string uri = params.value("uri", "");

        if (resourceReader_) {
            json content = resourceReader_(uri);
            response["result"] = {
                {"contents", json::array({{
                    {"uri", uri},
                    {"mimeType", "application/json"},
                    {"text", content.dump()}
                }})}
            };
        } else {
            response["error"] = {{"code", -32002}, {"message", "No resource reader configured"}};
        }
        return response;
    }

    json handlePromptsList(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);
        json arr = json::array();
        for (const auto& p : prompts_) {
            json argArr = json::array();
            for (const auto& a : p.arguments) {
                argArr.push_back({
                    {"name", a.name},
                    {"description", a.description},
                    {"required", a.required}
                });
            }
            arr.push_back({
                {"name", p.name},
                {"description", p.description},
                {"arguments", argArr}
            });
        }
        response["result"] = {{"prompts", arr}};
        return response;
    }

    json handlePromptsGet(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);

        auto params = request.contains("params") ? request["params"] : json::object();
        std::string name = params.value("name", "");
        json args = params.contains("arguments") ? params["arguments"] : json::object();

        auto messages = generatePromptMessages(name, args);
        if (messages.empty()) {
            response["error"] = {{"code", -32602}, {"message", "Unknown prompt: " + name}};
            return response;
        }

        json msgArr = json::array();
        for (const auto& m : messages) {
            msgArr.push_back({{"role", m.role}, {"content", m.content}});
        }
        response["result"] = {{"messages", msgArr}};
        return response;
    }

    // ---------------------------------------------------------------
    //  Bridge: call Whetstone JSON-RPC via callback
    // ---------------------------------------------------------------
    json callWhetstone(const std::string& method, const json& params = json::object()) {
        if (!rpcCallback_) return {{"error", "No RPC callback configured"}};
        json request = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", method},
            {"params", params}
        };
        json resp = rpcCallback_(request);
        if (resp.contains("result")) return resp["result"];
        if (resp.contains("error")) return {{"error", resp["error"]["message"]}};
        return resp;
    }

    // ---------------------------------------------------------------
    //  Step 208: Register AST query and mutation tools
    // ---------------------------------------------------------------
    void registerASTTools() {
        // whetstone_get_ast
        tools_.push_back({"whetstone_get_ast",
            "Get the current AST (Abstract Syntax Tree) of the active buffer as JSON. "
            "Returns the full tree structure with all nodes, annotations, and metadata.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_get_ast"] = [this](const json&) {
            return callWhetstone("getAST");
        };

        // whetstone_mutate
        tools_.push_back({"whetstone_mutate",
            "Apply a single mutation to the AST. Supports setProperty (rename/change), "
            "updateNode (bulk property update), deleteNode, and insertNode operations.",
            {{"type", "object"}, {"properties", {
                {"type", {{"type", "string"}, {"enum", {"setProperty", "updateNode", "deleteNode", "insertNode"}}, {"description", "Mutation type"}}},
                {"nodeId", {{"type", "string"}, {"description", "Target node ID (for setProperty, updateNode, deleteNode)"}}},
                {"property", {{"type", "string"}, {"description", "Property name (for setProperty)"}}},
                {"value", {{"type", "string"}, {"description", "New value (for setProperty)"}}},
                {"parentId", {{"type", "string"}, {"description", "Parent node ID (for insertNode)"}}},
                {"role", {{"type", "string"}, {"description", "Child role (for insertNode)"}}},
                {"node", {{"type", "object"}, {"description", "Node to insert (for insertNode)"}}}
            }}, {"required", {"type"}}}
        });
        toolHandlers_["whetstone_mutate"] = [this](const json& args) {
            return callWhetstone("applyMutation", args);
        };

        // whetstone_batch_mutate
        tools_.push_back({"whetstone_batch_mutate",
            "Apply multiple mutations atomically. All mutations succeed or all are rolled back. "
            "Each mutation has: type (setProperty|deleteNode|insertNode), nodeId, property, value, etc.",
            {{"type", "object"}, {"properties", {
                {"mutations", {{"type", "array"}, {"items", {{"type", "object"}}}, {"description", "Array of mutation objects"}}}
            }}, {"required", {"mutations"}}}
        });
        toolHandlers_["whetstone_batch_mutate"] = [this](const json& args) {
            return callWhetstone("applyBatch", args);
        };

        // whetstone_get_scope
        tools_.push_back({"whetstone_get_scope",
            "Get all symbols (variables, functions, parameters) visible at a given AST node. "
            "Walks up the scope chain to collect all accessible identifiers.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"}, {"description", "Node ID to query scope from"}}}
            }}, {"required", {"nodeId"}}}
        });
        toolHandlers_["whetstone_get_scope"] = [this](const json& args) {
            return callWhetstone("getInScopeSymbols", args);
        };

        // whetstone_get_call_hierarchy
        tools_.push_back({"whetstone_get_call_hierarchy",
            "Get the call hierarchy for a function: which functions call it (callers) "
            "and which functions it calls (callees).",
            {{"type", "object"}, {"properties", {
                {"functionId", {{"type", "string"}, {"description", "Function node ID"}}}
            }}, {"required", {"functionId"}}}
        });
        toolHandlers_["whetstone_get_call_hierarchy"] = [this](const json& args) {
            return callWhetstone("getCallHierarchy", args);
        };
    }

    // ---------------------------------------------------------------
    //  Step 209: Register annotation and generation tools
    // ---------------------------------------------------------------
    void registerAnnotationTools() {
        // whetstone_suggest_annotations
        tools_.push_back({"whetstone_suggest_annotations",
            "Get memory annotation suggestions for a code region. Returns suggestions "
            "with confidence scores and diagnostics. Specify nodeId or line/col.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"}, {"description", "Node ID to suggest annotations for"}}},
                {"line", {{"type", "integer"}, {"description", "Line number (0-based, alternative to nodeId)"}}},
                {"col", {{"type", "integer"}, {"description", "Column number (0-based, alternative to nodeId)"}}}
            }}}
        });
        toolHandlers_["whetstone_suggest_annotations"] = [this](const json& args) {
            return callWhetstone("getAnnotationSuggestions", args);
        };

        // whetstone_apply_annotation
        tools_.push_back({"whetstone_apply_annotation",
            "Apply a memory annotation suggestion to the AST. Pass the suggestion "
            "object from whetstone_suggest_annotations.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"}, {"description", "Target node ID"}}},
                {"annotationType", {{"type", "string"}, {"description", "e.g., ReclaimAnnotation, OwnerAnnotation"}}},
                {"strategy", {{"type", "string"}, {"description", "e.g., Tracing, Single, RAII"}}},
                {"reason", {{"type", "string"}, {"description", "Why this annotation"}}},
                {"confidence", {{"type", "number"}, {"description", "Confidence score 0-1"}}}
            }}, {"required", {"nodeId", "annotationType", "strategy"}}}
        });
        toolHandlers_["whetstone_apply_annotation"] = [this](const json& args) {
            return callWhetstone("applyAnnotationSuggestion", args);
        };

        // whetstone_generate_code
        tools_.push_back({"whetstone_generate_code",
            "Generate code from a natural language specification. Uses available "
            "library primitives when preferImports is true (default).",
            {{"type", "object"}, {"properties", {
                {"spec", {{"type", "string"}, {"description", "Natural language description of code to generate"}}},
                {"preferImports", {{"type", "boolean"}, {"description", "Prefer imported library symbols (default true)"}}}
            }}, {"required", {"spec"}}}
        });
        toolHandlers_["whetstone_generate_code"] = [this](const json& args) {
            return callWhetstone("generateCode", args);
        };

        // whetstone_run_pipeline
        tools_.push_back({"whetstone_run_pipeline",
            "Run the full Whetstone pipeline: parse source code, infer annotations, "
            "validate, optimize, and generate target language code.",
            {{"type", "object"}, {"properties", {
                {"source", {{"type", "string"}, {"description", "Source code to process"}}},
                {"sourceLanguage", {{"type", "string"}, {"description", "Source language (python, cpp, rust, go, java, javascript, typescript, elisp)"}}},
                {"targetLanguage", {{"type", "string"}, {"description", "Target language for code generation"}}}
            }}, {"required", {"source", "sourceLanguage", "targetLanguage"}}}
        });
        toolHandlers_["whetstone_run_pipeline"] = [this](const json& args) {
            return callWhetstone("runPipeline", args);
        };

        // whetstone_project_language
        tools_.push_back({"whetstone_project_language",
            "Project the current AST to a different target language. Adapts memory "
            "annotations appropriately and generates code in the target language.",
            {{"type", "object"}, {"properties", {
                {"targetLanguage", {{"type", "string"}, {"description", "Target language to project to"}}}
            }}, {"required", {"targetLanguage"}}}
        });
        toolHandlers_["whetstone_project_language"] = [this](const json& args) {
            return callWhetstone("projectLanguage", args);
        };
    }

    // ---------------------------------------------------------------
    //  Step 210: Register resources
    // ---------------------------------------------------------------
    void registerWhetstoneResources() {
        resources_.push_back({"whetstone://ast", "Current AST",
            "The current Abstract Syntax Tree as JSON", "application/json"});
        resources_.push_back({"whetstone://diagnostics", "Diagnostics",
            "Current diagnostics (errors, warnings) from LSP and Whetstone", "application/json"});
        resources_.push_back({"whetstone://libraries", "Imported Libraries",
            "Currently imported libraries with available symbols", "application/json"});
        resources_.push_back({"whetstone://annotations", "Annotations",
            "All memory annotations in the current module", "application/json"});
        resources_.push_back({"whetstone://settings", "Editor Settings",
            "Current editor settings (read-only)", "application/json"});
    }

    // ---------------------------------------------------------------
    //  Step 211: Register prompts
    // ---------------------------------------------------------------
    void registerWhetstonePrompts() {
        prompts_.push_back({"annotate_module",
            "Analyze the current module and suggest memory annotations for all "
            "unannotated functions with confidence scores.",
            {{"scope", "Which functions to annotate (all, unannotated, or a specific function name)", false}}
        });

        prompts_.push_back({"cross_language_projection",
            "Project the current code to a different target language, adapting "
            "annotations appropriately.",
            {{"targetLanguage", "Target language (cpp, rust, go, java, javascript, python, elisp)", true}}
        });

        prompts_.push_back({"security_audit",
            "Check all dependencies for known vulnerabilities and suggest "
            "safe alternatives or upgrades.",
            {}
        });

        prompts_.push_back({"refactor_memory",
            "Analyze memory strategy annotations and suggest improvements "
            "for safety and performance.",
            {}
        });
    }

    std::vector<MCPPromptMessage> generatePromptMessages(const std::string& name,
                                                          const json& args) {
        if (name == "annotate_module") {
            std::string scope = args.value("scope", "all");
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "Analyze the current module's AST and suggest memory annotations for " +
                    scope + " functions. For each function:\n"
                    "1. Use whetstone_get_ast to read the current AST\n"
                    "2. Use whetstone_suggest_annotations for each unannotated function\n"
                    "3. Present suggestions with confidence scores\n"
                    "4. Apply approved annotations with whetstone_apply_annotation\n"
                    "5. Verify with whetstone_get_ast that annotations were applied correctly"
                }}
            }};
        }
        if (name == "cross_language_projection") {
            std::string lang = args.value("targetLanguage", "cpp");
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "Project the current code to " + lang + ":\n"
                    "1. Use whetstone_get_ast to read the current AST\n"
                    "2. Use whetstone_project_language with targetLanguage=\"" + lang + "\"\n"
                    "3. Review the generated code and annotation adaptations\n"
                    "4. Report any annotation issues or incompatibilities"
                }}
            }};
        }
        if (name == "security_audit") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "Perform a security audit of the project's dependencies:\n"
                    "1. Use whetstone_get_ast to identify imported libraries\n"
                    "2. Check each dependency for known vulnerabilities\n"
                    "3. Suggest safe alternatives or version upgrades\n"
                    "4. Report severity levels and recommended actions"
                }}
            }};
        }
        if (name == "refactor_memory") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "Analyze and improve memory strategy annotations:\n"
                    "1. Use whetstone_get_ast to read the current AST\n"
                    "2. Use whetstone_suggest_annotations for each function\n"
                    "3. Identify annotation conflicts or suboptimal strategies\n"
                    "4. Suggest improvements for safety and performance\n"
                    "5. Apply approved changes with whetstone_apply_annotation"
                }}
            }};
        }
        return {};
    }

    // ---------------------------------------------------------------
    //  Register all tools
    // ---------------------------------------------------------------
    void registerWhetstoneTools() {
        registerASTTools();
        registerAnnotationTools();
    }
};
