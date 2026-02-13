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
            "Get the current AST of the active buffer. Set compact=true for "
            "a token-efficient flat list of {id, type, name, line, children}. "
            "Full mode returns complete tree with properties and spans.",
            {{"type", "object"}, {"properties", {
                {"compact", {{"type", "boolean"}, {"description", "Compact mode: flat list with minimal fields (default false)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_ast"] = [this](const json& args) {
            return callWhetstone("getAST", args);
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

        // whetstone_get_ast_subtree
        tools_.push_back({"whetstone_get_ast_subtree",
            "Get only the subtree rooted at a specific node ID. Returns full "
            "node detail for just that subtree, saving tokens vs full AST.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"}, {"description", "Root node ID for the subtree"}}}
            }}, {"required", {"nodeId"}}}
        });
        toolHandlers_["whetstone_get_ast_subtree"] = [this](const json& args) {
            return callWhetstone("getASTSubtree", args);
        };

        // whetstone_get_ast_diff
        tools_.push_back({"whetstone_get_ast_diff",
            "Get only the AST nodes that changed since a given version. "
            "Use the version number from a previous getAST or mutation response.",
            {{"type", "object"}, {"properties", {
                {"sinceVersion", {{"type", "integer"}, {"description", "Version number to diff against (from previous response)"}}}
            }}, {"required", {"sinceVersion"}}}
        });
        toolHandlers_["whetstone_get_ast_diff"] = [this](const json& args) {
            return callWhetstone("getASTDiff", args);
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
        // --- Step 270: Semantic annotation prompts ---
        prompts_.push_back({"annotate_intent",
            "Annotate unannotated functions with intent summaries and categories. "
            "Uses whetstone_get_unannotated_nodes and whetstone_set_semantic_annotation.",
            {}
        });

        prompts_.push_back({"annotate_complexity",
            "Annotate functions with complexity estimates (time complexity, "
            "cognitive complexity, lines of logic).",
            {}
        });

        prompts_.push_back({"annotate_risk",
            "Assess modification risk for each function based on callers, "
            "complexity, and side effects. Uses call hierarchy for dependent counts.",
            {}
        });

        prompts_.push_back({"annotate_contracts",
            "Document preconditions, postconditions, return shapes, and "
            "side effects for each function.",
            {}
        });

        prompts_.push_back({"annotate_full",
            "Run a complete annotation pass: intent, complexity, risk, "
            "and contracts for all unannotated functions. Save the annotated "
            "AST sidecar when done.",
            {}
        });

        // --- Original prompts ---
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
        // --- Step 270: Semantic annotation prompt messages ---
        if (name == "annotate_intent") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "For each unannotated function, add an intent annotation:\n"
                    "1. Call whetstone_get_unannotated_nodes to find targets\n"
                    "2. Use whetstone_get_ast (compact:true) to understand each function\n"
                    "3. Call whetstone_set_semantic_annotation with type='intent', providing:\n"
                    "   - summary: 1-sentence description of what and why\n"
                    "   - category: one of 'validation', 'transformation', 'io',\n"
                    "     'coordination', 'computation', 'initialization'\n"
                    "4. Verify with whetstone_get_semantic_annotations"
                }}
            }};
        }
        if (name == "annotate_complexity") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "For each function, estimate complexity:\n"
                    "1. Call whetstone_get_unannotated_nodes with type='complexity'\n"
                    "2. Use whetstone_get_ast (compact:true) to read each function\n"
                    "3. Call whetstone_set_semantic_annotation with type='complexity':\n"
                    "   - timeComplexity: 'O(1)', 'O(n)', 'O(n^2)', etc.\n"
                    "   - cognitiveComplexity: 1-10 scale\n"
                    "   - linesOfLogic: count of logic statements"
                }}
            }};
        }
        if (name == "annotate_risk") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "For each function, assess modification risk:\n"
                    "1. Call whetstone_get_unannotated_nodes with type='risk'\n"
                    "2. Use whetstone_get_call_hierarchy to count callers/dependents\n"
                    "3. Consider complexity and side effects\n"
                    "4. Call whetstone_set_semantic_annotation with type='risk':\n"
                    "   - level: 'low', 'medium', 'high', 'critical'\n"
                    "   - reason: why this risk level\n"
                    "   - dependentCount: number of callers/consumers"
                }}
            }};
        }
        if (name == "annotate_contracts") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "For each function, document data contracts:\n"
                    "1. Call whetstone_get_unannotated_nodes with type='contract'\n"
                    "2. Use whetstone_get_ast (compact:true) to read each function\n"
                    "3. Call whetstone_set_semantic_annotation with type='contract':\n"
                    "   - preconditions: input requirements\n"
                    "   - postconditions: output guarantees\n"
                    "   - returnShape: human-readable type/shape\n"
                    "   - sideEffects: 'none', 'io', 'mutation', 'network'"
                }}
            }};
        }
        if (name == "annotate_full") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "Run a complete semantic annotation pass:\n"
                    "1. Call whetstone_get_unannotated_nodes to find all targets\n"
                    "2. For each function, add intent, complexity, risk, and contract\n"
                    "   annotations using whetstone_set_semantic_annotation\n"
                    "3. Use whetstone_get_call_hierarchy for caller counts\n"
                    "4. Save the annotated AST with whetstone_save_annotated_ast\n"
                    "5. Verify with whetstone_get_semantic_annotations"
                }}
            }};
        }
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
    //  Step 247: Register file operation tools
    // ---------------------------------------------------------------
    void registerFileTools() {
        // whetstone_file_read
        tools_.push_back({"whetstone_file_read",
            "Read a file from the workspace. Returns file content with "
            "optional line range filtering. Path is relative to workspace root.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"}, {"description", "File path (relative to workspace)"}}},
                {"startLine", {{"type", "integer"}, {"description", "Start line (1-based, optional)"}}},
                {"endLine", {{"type", "integer"}, {"description", "End line (1-based, optional)"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_file_read"] = [this](const json& args) {
            return callWhetstone("fileRead", args);
        };

        // whetstone_file_write
        tools_.push_back({"whetstone_file_write",
            "Write content to a file in the workspace. Creates parent "
            "directories if needed. Requires Refactor or Generator role.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"}, {"description", "File path (relative to workspace)"}}},
                {"content", {{"type", "string"}, {"description", "Content to write"}}}
            }}, {"required", {"path", "content"}}}
        });
        toolHandlers_["whetstone_file_write"] = [this](const json& args) {
            return callWhetstone("fileWrite", args);
        };

        // whetstone_file_create
        tools_.push_back({"whetstone_file_create",
            "Create a new file with optional language template. "
            "Requires Refactor or Generator role.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"}, {"description", "File path (relative to workspace)"}}},
                {"language", {{"type", "string"}, {"description", "Language (python, cpp, rust, go, java, javascript, typescript)"}}},
                {"template", {{"type", "string"}, {"description", "Template name (module, empty) or empty for blank file"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_file_create"] = [this](const json& args) {
            return callWhetstone("fileCreate", args);
        };

        // whetstone_file_diff
        tools_.push_back({"whetstone_file_diff",
            "Get a unified diff between the active buffer content and the "
            "file on disk. Optionally specify a path, defaults to active buffer.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"}, {"description", "File path (optional, defaults to active buffer)"}}}
            }}}
        });
        toolHandlers_["whetstone_file_diff"] = [this](const json& args) {
            return callWhetstone("fileDiff", args);
        };

        // whetstone_workspace_list
        tools_.push_back({"whetstone_workspace_list",
            "List files in the workspace matching an optional glob pattern. "
            "Respects .gitignore. Returns path, size, and isDir for each entry.",
            {{"type", "object"}, {"properties", {
                {"glob", {{"type", "string"}, {"description", "Glob pattern (default: *). E.g. *.py, *.cpp"}}}
            }}}
        });
        toolHandlers_["whetstone_workspace_list"] = [this](const json& args) {
            return callWhetstone("workspaceList", args);
        };
    }

    // ---------------------------------------------------------------
    //  Step 250: Register diagnostic tools
    // ---------------------------------------------------------------
    void registerDiagnosticTools() {
        tools_.push_back({"whetstone_get_diagnostics",
            "Get structured diagnostics for the active buffer. Combines "
            "parse errors, annotation validation, and strategy violations "
            "into one stream with error codes, nodeIds, and fix suggestions. "
            "Filter by severity (error/warning/info/hint) or source "
            "(parser/annotation/strategy).",
            {{"type", "object"}, {"properties", {
                {"severity", {{"type", "string"},
                    {"enum", {"error", "warning", "info", "hint"}},
                    {"description", "Maximum severity level to include"}}},
                {"source", {{"type", "string"},
                    {"enum", {"parser", "annotation", "strategy"}},
                    {"description", "Filter by diagnostic source"}}}
            }}}
        });
        toolHandlers_["whetstone_get_diagnostics"] = [this](const json& args) {
            return callWhetstone("getDiagnostics", args);
        };

        // whetstone_get_diagnostics_delta
        tools_.push_back({"whetstone_get_diagnostics_delta",
            "Get only the diagnostics that changed since a given version. "
            "Returns added and removed diagnostics for efficient "
            "mutate-then-check loops. Use the version from a previous "
            "getDiagnostics or getDiagnosticsDelta response.",
            {{"type", "object"}, {"properties", {
                {"sinceVersion", {{"type", "integer"},
                    {"description",
                     "Version number from a previous diagnostics response"}}}
            }}, {"required", {"sinceVersion"}}}
        });
        toolHandlers_["whetstone_get_diagnostics_delta"] =
            [this](const json& args) {
                return callWhetstone("getDiagnosticsDelta", args);
            };

        // whetstone_get_quick_fixes
        tools_.push_back({"whetstone_get_quick_fixes",
            "Get all applicable quick-fix actions for a node or the entire "
            "AST. Each fix is a concrete mutation object the agent can review "
            "and apply with whetstone_apply_quick_fix.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"},
                    {"description",
                     "Node ID to get fixes for (omit for all fixes)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_quick_fixes"] = [this](const json& args) {
            return callWhetstone("getQuickFixes", args);
        };

        // whetstone_apply_quick_fix
        tools_.push_back({"whetstone_apply_quick_fix",
            "Apply a quick-fix action for a specific diagnostic. Takes the "
            "diagnostic code and nodeId, finds the fix, and applies it as a "
            "mutation. Reports whether the diagnostic was cleared.",
            {{"type", "object"}, {"properties", {
                {"diagCode", {{"type", "string"},
                    {"description", "Diagnostic error code (e.g. E0200)"}}},
                {"nodeId", {{"type", "string"},
                    {"description", "Target node ID"}}}
            }}, {"required", {"diagCode", "nodeId"}}}
        });
        toolHandlers_["whetstone_apply_quick_fix"] = [this](const json& args) {
            return callWhetstone("applyQuickFix", args);
        };

        // whetstone_get_project_diagnostics
        tools_.push_back({"whetstone_get_project_diagnostics",
            "Get diagnostics across all open files in one call. Returns "
            "diagnostics grouped by file path in compact format. Includes "
            "cross-file errors (undefined imports). Filter by severity "
            "or file glob pattern.",
            {{"type", "object"}, {"properties", {
                {"severity", {{"type", "string"},
                    {"enum", {"error", "warning", "info", "hint"}},
                    {"description",
                     "Maximum severity level to include"}}},
                {"fileGlob", {{"type", "string"},
                    {"description",
                     "File pattern filter (e.g. *.py, utils.py)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_project_diagnostics"] =
            [this](const json& args) {
                return callWhetstone("getProjectDiagnostics", args);
            };
    }

    // ---------------------------------------------------------------
    //  Register all tools
    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
    //  Batch query tool
    // ---------------------------------------------------------------
    void registerBatchTools() {
        // whetstone_batch_query
        tools_.push_back({"whetstone_batch_query",
            "Execute multiple queries in a single round-trip. "
            "Pass an array of {method, params} objects. Each sub-query "
            "runs independently; errors in one don't affect others.",
            {{"type", "object"},
             {"properties", {
                 {"queries", {{"type", "array"},
                     {"items", {{"type", "object"},
                         {"properties", {
                             {"method", {{"type", "string"}}},
                             {"params", {{"type", "object"}}}
                         }},
                         {"required", json::array({"method"})}
                     }}
                 }}
             }},
             {"required", json::array({"queries"})}}
        });
        toolHandlers_["whetstone_batch_query"] = [this](const json& args) {
            return callWhetstone("batchQuery", args);
        };
    }

    // ---------------------------------------------------------------
    //  Project management tools
    // ---------------------------------------------------------------
    void registerProjectTools() {
        // whetstone_open_file
        tools_.push_back({"whetstone_open_file",
            "Open a file as a buffer for AST analysis. Reads from disk "
            "if content not provided. Language auto-detected from extension.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description", "File path (relative to workspace or absolute)"}}},
                {"content", {{"type", "string"},
                    {"description", "File content (optional, reads from disk if omitted)"}}},
                {"language", {{"type", "string"},
                    {"description", "Language (optional, auto-detected from extension)"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_open_file"] = [this](const json& args) {
            return callWhetstone("openFile", args);
        };

        // whetstone_close_file
        tools_.push_back({"whetstone_close_file",
            "Close an open buffer, removing it from the project.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description", "Path of the buffer to close"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_close_file"] = [this](const json& args) {
            return callWhetstone("closeFile", args);
        };

        // whetstone_list_buffers
        tools_.push_back({"whetstone_list_buffers",
            "List all open buffers with language, modified status, and "
            "which buffer is currently active.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_list_buffers"] = [this](const json& args) {
            return callWhetstone("listBuffers", args);
        };

        // whetstone_set_active_buffer
        tools_.push_back({"whetstone_set_active_buffer",
            "Switch the active buffer. Subsequent getAST, getDiagnostics, "
            "etc. will operate on this buffer.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description", "Path of the buffer to activate"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_set_active_buffer"] = [this](const json& args) {
            return callWhetstone("setActiveBuffer", args);
        };

        // whetstone_index_workspace
        tools_.push_back({"whetstone_index_workspace",
            "Scan the workspace directory and index all file paths. "
            "Returns file and directory counts. Does not open files.",
            {{"type", "object"}, {"properties", {
                {"root", {{"type", "string"},
                    {"description", "Workspace root (optional, uses --workspace default)"}}}
            }}}
        });
        toolHandlers_["whetstone_index_workspace"] = [this](const json& args) {
            return callWhetstone("indexWorkspace", args);
        };

        // whetstone_search_project
        tools_.push_back({"whetstone_search_project",
            "Find all references to a symbol across all open files. "
            "Search by name or nodeId. Returns file, line, col, nodeId, "
            "kind (definition/call/reference/parameter), and context.",
            {{"type", "object"}, {"properties", {
                {"name", {{"type", "string"},
                    {"description",
                     "Symbol name to search for"}}},
                {"nodeId", {{"type", "string"},
                    {"description",
                     "Node ID to resolve name from (alternative to name)"}}}
            }}}
        });
        toolHandlers_["whetstone_search_project"] =
            [this](const json& args) {
                return callWhetstone("searchProject", args);
            };

        // whetstone_rename_symbol
        tools_.push_back({"whetstone_rename_symbol",
            "Rename a symbol across all open files. Updates function "
            "definitions, calls, variable declarations, references, and "
            "parameters. Set preview=true to see changes without applying.",
            {{"type", "object"}, {"properties", {
                {"oldName", {{"type", "string"},
                    {"description", "Current symbol name"}}},
                {"newName", {{"type", "string"},
                    {"description", "New symbol name"}}},
                {"preview", {{"type", "boolean"},
                    {"description",
                     "Preview changes without applying (default false)"}}}
            }}, {"required", {"oldName", "newName"}}}
        });
        toolHandlers_["whetstone_rename_symbol"] =
            [this](const json& args) {
                return callWhetstone("renameSymbol", args);
            };
    }

    // ---------------------------------------------------------------
    //  Save and undo/redo tools
    // ---------------------------------------------------------------
    void registerSaveUndoTools() {
        // whetstone_save_buffer
        tools_.push_back({"whetstone_save_buffer",
            "Save a buffer to disk. Writes the current editBuf (code "
            "regenerated from AST) to the file path. Clears the "
            "modified flag. Defaults to active buffer if no path given.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description",
                     "Buffer path to save (optional, defaults to active)"}}}
            }}}
        });
        toolHandlers_["whetstone_save_buffer"] =
            [this](const json& args) {
                return callWhetstone("saveBuffer", args);
            };

        // whetstone_save_all_buffers
        tools_.push_back({"whetstone_save_all_buffers",
            "Save all modified buffers to disk. Skips unmodified "
            "buffers. Returns count of saved and skipped files.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_save_all_buffers"] =
            [this](const json& args) {
                return callWhetstone("saveAllBuffers", args);
            };

        // whetstone_undo
        tools_.push_back({"whetstone_undo",
            "Undo the last mutation on the active buffer. Restores "
            "the previous AST and regenerated code from the snapshot "
            "journal. Returns the new undo/redo depths.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_undo"] =
            [this](const json& args) {
                return callWhetstone("undo", args);
            };

        // whetstone_redo
        tools_.push_back({"whetstone_redo",
            "Redo the last undone mutation on the active buffer. "
            "Returns the new undo/redo depths.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_redo"] =
            [this](const json& args) {
                return callWhetstone("redo", args);
            };
    }

    // ---------------------------------------------------------------
    //  Step 267: Sidecar persistence tools
    // ---------------------------------------------------------------
    void registerSidecarTools() {
        // whetstone_save_annotated_ast
        tools_.push_back({"whetstone_save_annotated_ast",
            "Save the current buffer's AST (with all semantic annotations) "
            "to a .whetstone/ sidecar file. Annotations persist alongside "
            "the codebase without polluting source code.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description", "Buffer path to save annotations for"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_save_annotated_ast"] =
            [this](const json& args) {
                return callWhetstone("saveAnnotatedAST", args);
            };

        // whetstone_load_annotated_ast
        tools_.push_back({"whetstone_load_annotated_ast",
            "Load semantic annotations from a .whetstone/ sidecar file "
            "and merge them into the current buffer's AST. Matches "
            "annotations to live nodes by node ID.",
            {{"type", "object"}, {"properties", {
                {"path", {{"type", "string"},
                    {"description", "Buffer path to load annotations for"}}}
            }}, {"required", {"path"}}}
        });
        toolHandlers_["whetstone_load_annotated_ast"] =
            [this](const json& args) {
                return callWhetstone("loadAnnotatedAST", args);
            };

        // whetstone_list_annotated_files
        tools_.push_back({"whetstone_list_annotated_files",
            "List all files that have saved .whetstone/ sidecar annotation "
            "files in the workspace.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_list_annotated_files"] =
            [this](const json& args) {
                return callWhetstone("listAnnotatedFiles", args);
            };
    }

    // ---------------------------------------------------------------
    //  Step 269: Semantic annotation management tools
    // ---------------------------------------------------------------
    void registerSemanticAnnotationTools() {
        // whetstone_set_semantic_annotation
        tools_.push_back({"whetstone_set_semantic_annotation",
            "Set or update a semantic annotation on an AST node. If an "
            "annotation of the same type already exists, it is replaced. "
            "Types: intent, complexity, risk, contract, tags.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"},
                    {"description", "Target node ID"}}},
                {"type", {{"type", "string"},
                    {"enum", {"intent", "complexity", "risk", "contract", "tags"}},
                    {"description", "Annotation type"}}},
                {"fields", {{"type", "object"},
                    {"description", "Annotation-specific fields"}}}
            }}, {"required", {"nodeId", "type", "fields"}}}
        });
        toolHandlers_["whetstone_set_semantic_annotation"] =
            [this](const json& args) {
                return callWhetstone("setSemanticAnnotation", args);
            };

        // whetstone_get_semantic_annotations
        tools_.push_back({"whetstone_get_semantic_annotations",
            "Get semantic annotations. If nodeId provided, returns "
            "annotations for that node. Otherwise returns all annotated "
            "nodes with their annotations.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"},
                    {"description",
                     "Node ID (optional, omit for all annotated nodes)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_semantic_annotations"] =
            [this](const json& args) {
                return callWhetstone("getSemanticAnnotations", args);
            };

        // whetstone_remove_semantic_annotation
        tools_.push_back({"whetstone_remove_semantic_annotation",
            "Remove a semantic annotation of a specific type from a node.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"},
                    {"description", "Target node ID"}}},
                {"type", {{"type", "string"},
                    {"enum", {"intent", "complexity", "risk", "contract", "tags"}},
                    {"description", "Annotation type to remove"}}}
            }}, {"required", {"nodeId", "type"}}}
        });
        toolHandlers_["whetstone_remove_semantic_annotation"] =
            [this](const json& args) {
                return callWhetstone("removeSemanticAnnotation", args);
            };

        // whetstone_get_unannotated_nodes
        tools_.push_back({"whetstone_get_unannotated_nodes",
            "Get Function/Variable nodes that lack semantic annotations. "
            "Optionally filter by annotation type to find nodes missing "
            "a specific annotation.",
            {{"type", "object"}, {"properties", {
                {"type", {{"type", "string"},
                    {"enum", {"intent", "complexity", "risk", "contract", "tags"}},
                    {"description",
                     "Filter: nodes missing this annotation type (optional)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_unannotated_nodes"] =
            [this](const json& args) {
                return callWhetstone("getUnannotatedNodes", args);
            };
    }

    // ---------------------------------------------------------------
    //  Step 286: Environment layer tools
    // ---------------------------------------------------------------
    void registerEnvironmentTools() {
        // whetstone_set_environment
        tools_.push_back({"whetstone_set_environment",
            "Set the environment spec on the active module. Defines "
            "scheduler, memory model, capabilities, constraints, "
            "exception model, and FFI style. Replaces any existing env.",
            {{"type", "object"}, {"properties", {
                {"envId", {{"type", "string"},
                    {"description", "Environment identifier (e.g. posix_process, browser, jvm)"}}},
                {"scheduler", {{"type", "string"},
                    {"enum", {"event_loop", "threads", "fibers", "coroutines", "single_thread"}},
                    {"description", "Scheduling model"}}},
                {"memory", {{"type", "string"},
                    {"enum", {"manual", "raii", "refcount", "tracing_gc", "region"}},
                    {"description", "Memory management model"}}},
                {"capabilities", {{"type", "array"}, {"items", {{"type", "string"}}},
                    {"description", "Available capabilities (io.fs, io.net, threads, etc.)"}}},
                {"constraints", {{"type", "array"}, {"items", {{"type", "string"}}},
                    {"description", "Environment constraints (no_jit, no_threads, etc.)"}}},
                {"exceptions", {{"type", "string"},
                    {"enum", {"unwind", "checked", "typed", "untyped"}},
                    {"description", "Exception handling model"}}},
                {"ffi", {{"type", "string"},
                    {"enum", {"c_abi", "dynamic_linking", "none"}},
                    {"description", "Foreign function interface style"}}}
            }}, {"required", {"envId"}}}
        });
        toolHandlers_["whetstone_set_environment"] =
            [this](const json& args) {
                return callWhetstone("setEnvironment", args);
            };

        // whetstone_get_environment
        tools_.push_back({"whetstone_get_environment",
            "Get the current environment spec from the active module. "
            "Returns hasEnvironment=false if none is set.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_get_environment"] =
            [this](const json& args) {
                return callWhetstone("getEnvironment", args);
            };

        // whetstone_validate_environment
        tools_.push_back({"whetstone_validate_environment",
            "Validate the active module against its environment spec. "
            "Checks capability requirements (E0501) and annotation "
            "compatibility (E0502-E0505). Returns diagnostics array.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_validate_environment"] =
            [this](const json& args) {
                return callWhetstone("validateEnvironment", args);
            };

        // whetstone_get_lowering_hints
        tools_.push_back({"whetstone_get_lowering_hints",
            "Get environment-aware lowering hints for a node. Returns "
            "code generation patterns based on scheduler, memory, and "
            "exception models (e.g. callback, std_async, try_catch).",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"},
                    {"description",
                     "Node ID (optional, defaults to module root)"}}}
            }}}
        });
        toolHandlers_["whetstone_get_lowering_hints"] =
            [this](const json& args) {
                return callWhetstone("getLoweringHints", args);
            };
    }

    void registerTrainingDataTools() {
        // whetstone_export_training_data
        tools_.push_back({"whetstone_export_training_data",
            "Export annotated code as training data for LLM fine-tuning. "
            "Supports HuggingFace (instruction/input/output JSONL) and "
            "PairsJSONL (raw_code/annotated_code) formats. Includes statistics.",
            {{"type", "object"}, {"properties", {
                {"format", {{"type", "string"},
                    {"description",
                     "Export format: 'huggingface' or 'pairs' (default: 'huggingface')"}}},
                {"languages", {{"type", "array"},
                    {"description",
                     "Languages to include (default: all available)"},
                    {"items", {{"type", "string"}}}}}
            }}}
        });
        toolHandlers_["whetstone_export_training_data"] =
            [this](const json& args) {
                return callWhetstone("exportTrainingData", args);
            };

        // whetstone_generate_examples
        tools_.push_back({"whetstone_generate_examples",
            "Generate annotated code examples with Semanno comments. "
            "Takes raw source code and language, returns annotated version "
            "with inferred annotations from all 8 subjects.",
            {{"type", "object"}, {"properties", {
                {"source", {{"type", "string"},
                    {"description", "Source code to annotate"}}},
                {"language", {{"type", "string"},
                    {"description",
                     "Programming language (python, cpp, rust, etc.)"}}}
            }}, {"required", json::array({"source", "language"})}}
        });
        toolHandlers_["whetstone_generate_examples"] =
            [this](const json& args) {
                return callWhetstone("generateExamples", args);
            };
    }

    void registerWhetstoneTools() {
        registerASTTools();
        registerAnnotationTools();
        registerFileTools();
        registerDiagnosticTools();
        registerBatchTools();
        registerProjectTools();
        registerSaveUndoTools();
        registerSidecarTools();
        registerSemanticAnnotationTools();
        registerEnvironmentTools();
        registerTrainingDataTools();
    }
};
