#pragma once

// Step 505: MCP Tool Documentation
// Structured MCP tool docs with category index and exportable resource formats.

#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct MCPToolErrorCode {
    std::string code;
    std::string meaning;
};

struct MCPToolDoc {
    std::string name;
    std::string category;      // AST, Diagnostics, Workflow, Routing, Security, etc.
    std::string description;
    std::string inputSchema;
    std::string inputExample;
    std::string outputSchema;
    std::string outputFields;
    std::vector<MCPToolErrorCode> errorCodes;

    bool isComplete() const {
        return !name.empty() && !category.empty() && !description.empty() &&
               !inputSchema.empty() && !inputExample.empty() &&
               !outputSchema.empty() && !outputFields.empty() &&
               !errorCodes.empty();
    }
};

struct MCPDocumentationResource {
    std::vector<MCPToolDoc> tools;
    std::map<std::string, std::vector<std::string>> categories;
    std::vector<std::string> notes;
};

class MCPToolDocumentation {
public:
    static MCPDocumentationResource buildDefault() {
        MCPDocumentationResource out;
        out.tools = {
            {"whetstone_parse_ast", "AST",
             "Parse source code into a normalized AST with semantic node identities.",
             R"({"type":"object","properties":{"source":{"type":"string"},"language":{"type":"string"}},"required":["source","language"]})",
             R"({"source":"fn add(a,b){return a+b;}","language":"javascript"})",
             R"({"type":"object","properties":{"ast":{"type":"object"},"nodeCount":{"type":"integer"}}})",
             "ast: parsed tree, nodeCount: number of nodes",
             {{"-32602", "Invalid params"}, {"-32001", "Parse failure"}}},

            {"whetstone_run_diagnostics", "Diagnostics",
             "Run static diagnostics and return normalized findings with severity.",
             R"({"type":"object","properties":{"source":{"type":"string"},"language":{"type":"string"}},"required":["source","language"]})",
             R"({"source":"int x = gets(buf);","language":"c"})",
             R"({"type":"object","properties":{"findings":{"type":"array"},"summary":{"type":"object"}}})",
             "findings: diagnostic list, summary: by-severity counts",
             {{"-32602", "Invalid params"}, {"-32003", "Diagnostics engine failure"}}},

            {"whetstone_create_workflow", "Workflow",
             "Create a workflow from skeleton annotations with dependency ordering.",
             R"({"type":"object","properties":{"projectName":{"type":"string"}},"required":["projectName"]})",
             R"({"projectName":"demo"})",
             R"({"type":"object","properties":{"workflow":{"type":"object"},"itemCount":{"type":"integer"}}})",
             "workflow: serialized workflow state, itemCount: total items",
             {{"-32602", "Invalid params"}, {"-32000", "No active project"}}},

            {"whetstone_route_work_items", "Routing",
             "Route ready workflow items to deterministic/template/slm/llm/human workers.",
             R"({"type":"object","properties":{"maxItems":{"type":"integer"}}})",
             R"({"maxItems":25})",
             R"({"type":"object","properties":{"routed":{"type":"array"},"counts":{"type":"object"}}})",
             "routed: per-item routing decisions, counts: worker-type distribution",
             {{"-32602", "Invalid params"}, {"-32010", "Routing state unavailable"}}},

            {"whetstone_security_audit", "Security",
             "Run security audit with CWE mappings and risk ratings.",
             R"({"type":"object","properties":{"source":{"type":"string"},"language":{"type":"string"}},"required":["source","language"]})",
             R"({"source":"char buf[8]; gets(buf);","language":"c"})",
             R"({"type":"object","properties":{"findings":{"type":"array"},"overallRisk":{"type":"integer"}}})",
             "findings: vulnerability details, overallRisk: max severity level",
             {{"-32602", "Invalid params"}, {"-32020", "Security analysis failed"}}}
        };

        indexCategories(out);
        out.notes.push_back("Default MCP tool documentation generated");
        out.notes.push_back("Includes AST, Diagnostics, Workflow, Routing, Security categories");
        return out;
    }

    static bool validateCompleteness(const MCPDocumentationResource& docs,
                                     std::string* error = nullptr) {
        if (docs.tools.empty()) {
            if (error) *error = "No tools documented";
            return false;
        }

        for (const auto& t : docs.tools) {
            if (!t.isComplete()) {
                if (error) *error = "Incomplete tool documentation: " + t.name;
                return false;
            }
        }
        return true;
    }

    static std::string toMarkdown(const MCPDocumentationResource& docs) {
        std::ostringstream o;
        o << "# MCP Tool Documentation\n\n";
        for (const auto& t : docs.tools) {
            o << "## " << t.name << "\n";
            o << "- Category: " << t.category << "\n";
            o << "- Description: " << t.description << "\n";
            o << "- Input schema: `" << t.inputSchema << "`\n";
            o << "- Input example: `" << t.inputExample << "`\n";
            o << "- Output schema: `" << t.outputSchema << "`\n";
            o << "- Output fields: " << t.outputFields << "\n";
            o << "- Errors:\n";
            for (const auto& e : t.errorCodes) {
                o << "  - " << e.code << ": " << e.meaning << "\n";
            }
            o << "\n";
        }
        return o.str();
    }

    static std::string toJson(const MCPDocumentationResource& docs) {
        std::ostringstream o;
        o << "{\n  \"tools\": [\n";
        for (size_t i = 0; i < docs.tools.size(); ++i) {
            const auto& t = docs.tools[i];
            o << "    {\n"
              << "      \"name\": \"" << escape(t.name) << "\",\n"
              << "      \"category\": \"" << escape(t.category) << "\",\n"
              << "      \"description\": \"" << escape(t.description) << "\"\n"
              << "    }";
            if (i + 1 < docs.tools.size()) o << ",";
            o << "\n";
        }
        o << "  ]\n}\n";
        return o.str();
    }

    static bool writeResource(const MCPDocumentationResource& docs,
                              const std::string& directory,
                              std::string* markdownPath = nullptr,
                              std::string* jsonPath = nullptr) {
        namespace fs = std::filesystem;
        fs::path root = directory.empty() ? fs::path(".") : fs::path(directory);
        std::error_code ec;
        fs::create_directories(root, ec);
        if (ec) return false;

        fs::path md = root / "mcp_tool_docs.md";
        fs::path js = root / "mcp_tool_docs.json";

        {
            std::ofstream f(md);
            if (!f) return false;
            f << toMarkdown(docs);
        }
        {
            std::ofstream f(js);
            if (!f) return false;
            f << toJson(docs);
        }

        if (markdownPath) *markdownPath = md.string();
        if (jsonPath) *jsonPath = js.string();
        return true;
    }

private:
    static void indexCategories(MCPDocumentationResource& docs) {
        docs.categories.clear();
        for (const auto& t : docs.tools) docs.categories[t.category].push_back(t.name);
    }

    static std::string escape(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '\"') out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else out.push_back(c);
        }
        return out;
    }
};
