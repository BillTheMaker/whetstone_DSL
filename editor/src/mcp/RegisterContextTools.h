// Step 677: MCP wiring for context assembly tools
//
// Registers:
//   whetstone_assemble_context
//
// This file is #included inside the MCPServer class body.

#include "../WorkspaceFileIndex.h"
#include "../ContextSliceAssembler.h"
#include "../TokenBudgetEnforcer.h"

#include <filesystem>

    void registerContextTools() {
        tools_.push_back({"whetstone_assemble_context",
            "Assemble minimal cross-file context slices for requested files/symbols "
            "and enforce a token budget.",
            {{"type", "object"}, {"properties", {
                {"files", {{"type", "array"},
                    {"description", "List of slice requests."},
                    {"items", {{"type", "object"}, {"properties", {
                        {"path", {{"type", "string"},
                            {"description", "File path relative to workspace."}}},
                        {"symbol", {{"type", "string"},
                            {"description", "Symbol name to extract (optional)."}}},
                        {"line_hint", {{"type", "integer"},
                            {"description", "Line number hint (optional)."}}},
                        {"head_lines", {{"type", "integer"},
                            {"description", "Head fallback line count (default 40)."}}}
                    }}, {"required", json::array({"path"})}}}}},
                {"max_tokens", {{"type", "integer"},
                    {"description", "Token budget (default 8000)."}}}
            }}, {"required", json::array({"files"})}}
        });
        toolHandlers_["whetstone_assemble_context"] =
            [this](const json& args) {
                return runAssembleContext(args);
            };
    }

    json runAssembleContext(const json& args) {
        if (!args.contains("files") || !args["files"].is_array()) {
            return {{"success", false}, {"error", "files_missing_or_invalid"}};
        }

        const int maxTokens = args.value("max_tokens", 8000);
        std::vector<SliceRequest> requests;
        bool needsSymbolIndex = false;
        std::filesystem::path indexRoot = std::filesystem::current_path();
        for (const auto& entry : args["files"]) {
            if (!entry.is_object() || !entry.contains("path") || !entry["path"].is_string()) {
                return {{"success", false}, {"error", "file_entry_missing_path"}};
            }

            SliceRequest req;
            std::filesystem::path path = std::filesystem::path(entry["path"].get<std::string>());
            if (path.is_relative()) path = std::filesystem::current_path() / path;
            req.filePath = path.lexically_normal().generic_string();
            req.symbolName = entry.value("symbol", "");
            req.lineHint = entry.value("line_hint", 0);
            req.headLines = entry.value("head_lines", 40);
            if (!req.symbolName.empty()) {
                needsSymbolIndex = true;
                std::error_code ec;
                if (std::filesystem::exists(path, ec)) {
                    indexRoot = path.parent_path();
                }
            }
            requests.push_back(req);
        }

        WorkspaceFileIndex index;
        if (needsSymbolIndex) {
            index = WorkspaceFileIndex::build(indexRoot.generic_string());
        }

        auto slices = ContextSliceAssembler::assemble(requests, index);
        auto [included, report] = TokenBudgetEnforcer::enforce(slices, maxTokens);

        json outSlices = json::array();
        for (const auto& s : included) {
            outSlices.push_back({
                {"file", s.filePath},
                {"symbol", s.symbolName},
                {"line_start", s.lineStart},
                {"line_end", s.lineEnd},
                {"content", s.content}
            });
        }

        return {
            {"success", true},
            {"slices", outSlices},
            {"budget_report", {
                {"total_slices", report.totalSlices},
                {"included_slices", report.includedSlices},
                {"dropped_slices", report.droppedSlices},
                {"estimated_tokens_used", report.estimatedTokensUsed},
                {"budget_tokens", report.budgetTokens},
                {"budget_exceeded", report.budgetExceeded},
                {"dropped_files", report.droppedFiles}
            }}
        };
    }
