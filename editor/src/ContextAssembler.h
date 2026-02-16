#pragma once
// Step 328: ContextAssembler — Context Window Assembly + Budget
//
// Builds the context window for each work item based on its @ContextWidth,
// respecting token budgets. Priority ordering under truncation:
// target node > annotations > diagnostics > siblings > buffer > project.

#include "WorkerRegistry.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

// --- HeadlessBufferInfo: lightweight buffer info for context assembly ---
// (Avoids depending on HeadlessEditorState directly)

struct BufferInfo {
    std::string path;
    std::string content;
    json compactAst;     // pre-computed compact AST summary
    json importGraph;    // import/dependency graph
};

// --- Token estimation ---

inline int estimateTokens(const json& j) {
    std::string s = j.dump();
    return static_cast<int>(s.size()) / 4;
}

inline int estimateTokens(const std::string& s) {
    return static_cast<int>(s.size()) / 4;
}

// --- ContextAssembler ---

class ContextAssembler {
public:
    // Main entry point: assemble context at the specified width within budget
    WorkerContext assembleContext(const WorkItem& item,
                                  const std::map<std::string, BufferInfo>& buffers,
                                  const std::string& contextWidth,
                                  int budget = 0) const {
        WorkerContext ctx;
        int tokensUsed = 0;
        int effectiveBudget = (budget > 0) ? budget : 999999; // 0 = unlimited

        // Always include target node AST
        auto bufIt = buffers.find(item.bufferId);
        if (bufIt != buffers.end()) {
            ctx.nodeAst = findNodeInAst(bufIt->second.compactAst, item.nodeId);
        } else {
            // Fallback: minimal node info
            ctx.nodeAst = json{{"id", item.nodeId}, {"type", item.nodeType},
                               {"name", item.nodeName}};
        }
        tokensUsed += estimateTokens(ctx.nodeAst);

        // Always include annotations (high priority)
        // Annotations are embedded in nodeAst for now

        // Rejection feedback is first-class context for re-routed attempts.
        if (!item.rejectionFeedback.empty()) {
            ctx.feedbackFromRejection = item.rejectionFeedback;
        } else if (!item.result.reasoning.empty() &&
                   item.result.reasoning.find("feedback") != std::string::npos) {
            // Backward-compatible fallback for older serialized items.
            ctx.feedbackFromRejection = item.result.reasoning;
        }

        // Width-specific assembly
        if (contextWidth == "local") {
            assembleLocal(ctx, item, buffers, tokensUsed, effectiveBudget);
        } else if (contextWidth == "file") {
            assembleFile(ctx, item, buffers, tokensUsed, effectiveBudget);
        } else if (contextWidth == "project") {
            assembleProject(ctx, item, buffers, tokensUsed, effectiveBudget);
        } else if (contextWidth == "cross-project") {
            // cross-project = project + external refs
            assembleProject(ctx, item, buffers, tokensUsed, effectiveBudget);
            // External dependency annotations would come from sidecar files
            // (placeholder for now)
        }

        return ctx;
    }

private:
    // Find a node by ID in a compact AST (array of nodes)
    json findNodeInAst(const json& compactAst, const std::string& nodeId) const {
        if (compactAst.is_array()) {
            for (const auto& node : compactAst) {
                if (node.contains("id") && node["id"] == nodeId) {
                    return node;
                }
            }
        }
        // If compact AST is an object (single node or full tree)
        if (compactAst.contains("id") && compactAst["id"] == nodeId) {
            return compactAst;
        }
        // Return what we have as fallback
        return json{{"id", nodeId}, {"type", "unknown"}};
    }

    // Local: target node only + immediate siblings info
    void assembleLocal(WorkerContext& ctx, const WorkItem& item,
                       const std::map<std::string, BufferInfo>& buffers,
                       int& tokensUsed, int budget) const {
        // Local context is minimal — just the node AST (already included)
        // Add sibling summary if budget allows
        auto bufIt = buffers.find(item.bufferId);
        if (bufIt != buffers.end() && bufIt->second.compactAst.is_array()) {
            json siblings = json::array();
            for (const auto& node : bufIt->second.compactAst) {
                if (node.contains("id") && node["id"] != item.nodeId) {
                    json sibling = {{"id", node.value("id", "")},
                                    {"type", node.value("type", "")},
                                    {"name", node.value("name", "")}};
                    int cost = estimateTokens(sibling);
                    if (tokensUsed + cost <= budget) {
                        siblings.push_back(sibling);
                        tokensUsed += cost;
                    }
                }
            }
            if (!siblings.empty()) {
                ctx.nodeAst["siblings"] = siblings;
            }
        }
    }

    // File: target node + buffer content
    void assembleFile(WorkerContext& ctx, const WorkItem& item,
                      const std::map<std::string, BufferInfo>& buffers,
                      int& tokensUsed, int budget) const {
        // Include local context first
        assembleLocal(ctx, item, buffers, tokensUsed, budget);

        // Add buffer content if within budget
        auto bufIt = buffers.find(item.bufferId);
        if (bufIt != buffers.end()) {
            int contentTokens = estimateTokens(bufIt->second.content);
            if (tokensUsed + contentTokens <= budget) {
                ctx.bufferContent = bufIt->second.content;
                tokensUsed += contentTokens;
            } else {
                // Truncate content to fit remaining budget
                int remaining = budget - tokensUsed;
                if (remaining > 0) {
                    int chars = remaining * 4;
                    ctx.bufferContent = bufIt->second.content.substr(
                        0, std::min((int)bufIt->second.content.size(), chars));
                    tokensUsed += remaining;
                }
            }
        }
    }

    // Project: file context + compact summaries of all buffers
    void assembleProject(WorkerContext& ctx, const WorkItem& item,
                         const std::map<std::string, BufferInfo>& buffers,
                         int& tokensUsed, int budget) const {
        // Include file context first
        assembleFile(ctx, item, buffers, tokensUsed, budget);

        // Add compact AST summaries for other buffers
        json summaries = json::object();
        for (const auto& [path, buf] : buffers) {
            if (path == item.bufferId) continue; // skip own buffer
            int cost = estimateTokens(buf.compactAst);
            if (tokensUsed + cost <= budget) {
                summaries[path] = buf.compactAst;
                tokensUsed += cost;
            }
        }
        if (!summaries.empty()) {
            ctx.projectSummary = summaries;
        }

        // Add import graph for the target buffer
        auto bufIt = buffers.find(item.bufferId);
        if (bufIt != buffers.end() && !bufIt->second.importGraph.is_null()) {
            int cost = estimateTokens(bufIt->second.importGraph);
            if (tokensUsed + cost <= budget) {
                if (ctx.projectSummary.is_null()) ctx.projectSummary = json::object();
                ctx.projectSummary["importGraph"] = bufIt->second.importGraph;
                tokensUsed += cost;
            }
        }
    }
};
