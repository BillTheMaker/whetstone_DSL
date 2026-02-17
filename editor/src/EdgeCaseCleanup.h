#pragma once

// Step 506: Edge Case Cleanup
// Guard rails for common failure modes with explicit error semantics.

#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

struct EdgeCaseResult {
    bool ok = true;
    std::string code;      // e.g. E_EMPTY_FILE, E_BINARY_FILE, ...
    std::string message;
};

class EdgeCaseCleanup {
public:
    static EdgeCaseResult inspectFileContent(const std::string& content,
                                             std::size_t maxBytes = 1024 * 1024) {
        if (content.empty()) return fail("E_EMPTY_FILE", "File is empty");
        if (isBinary(content)) return fail("E_BINARY_FILE", "Binary file detected");
        if (content.size() > maxBytes) return fail("E_FILE_TOO_LARGE", "File exceeds size threshold");
        return ok("File content accepted");
    }

    static EdgeCaseResult validateAnnotations(const std::vector<std::string>& annotations) {
        for (const auto& a : annotations) {
            if (a.empty() || a[0] != '@') {
                return fail("E_MALFORMED_ANNOTATION", "Annotation must begin with '@'");
            }
            if (a.find('(') != std::string::npos && a.find(')') == std::string::npos) {
                return fail("E_MALFORMED_ANNOTATION", "Unbalanced annotation parentheses");
            }
        }
        return ok("Annotations valid");
    }

    static EdgeCaseResult detectCircularDependencies(
        const std::map<std::string, std::vector<std::string>>& graph) {
        std::set<std::string> visited, stack;
        for (const auto& kv : graph) {
            if (dfsCycle(kv.first, graph, visited, stack))
                return fail("E_CIRCULAR_DEPENDENCY", "Circular dependency detected");
        }
        return ok("No circular dependencies");
    }

    static EdgeCaseResult checkConcurrentModification(int expectedVersion, int currentVersion) {
        if (expectedVersion != currentVersion) {
            return fail("E_CONCURRENT_MODIFICATION",
                        "Workflow state changed concurrently; reload required");
        }
        return ok("No concurrent modification conflict");
    }

    static EdgeCaseResult saveInterruptedWorkflow(const std::string& path,
                                                  const std::string& workflowJson) {
        namespace fs = std::filesystem;
        fs::path p(path);
        std::error_code ec;
        fs::create_directories(p.parent_path(), ec);
        if (ec) return fail("E_WORKFLOW_SAVE_FAILED", "Unable to create workflow directory");

        std::ofstream out(path);
        if (!out) return fail("E_WORKFLOW_SAVE_FAILED", "Unable to open workflow file for write");
        out << workflowJson;
        return ok("Workflow saved");
    }

    static EdgeCaseResult resumeInterruptedWorkflow(const std::string& path,
                                                    std::string* workflowJson = nullptr) {
        std::ifstream in(path);
        if (!in) return fail("E_WORKFLOW_RESUME_FAILED", "Workflow state file missing");
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        if (content.empty()) return fail("E_WORKFLOW_RESUME_FAILED", "Workflow state file is empty");
        if (workflowJson) *workflowJson = content;
        return ok("Workflow resumed");
    }

    static std::string normalizeLanguage(const std::string& language) {
        if (language == "python" || language == "cpp" || language == "c" ||
            language == "rust" || language == "go" || language == "java" ||
            language == "javascript" || language == "typescript") {
            return language;
        }
        return "plain_text";
    }

    static EdgeCaseResult validateMcpRequest(const std::string& requestJson) {
        if (requestJson.empty()) {
            return fail("E_MCP_MALFORMED_REQUEST", "Request payload is empty");
        }
        if (requestJson.front() != '{' || requestJson.back() != '}') {
            return fail("E_MCP_MALFORMED_REQUEST", "Request is not valid JSON object text");
        }
        if (requestJson.find("\"method\"") == std::string::npos) {
            return fail("E_MCP_MALFORMED_REQUEST", "Missing required field: method");
        }
        return ok("MCP request valid");
    }

private:
    static EdgeCaseResult ok(const std::string& message) {
        return {true, "", message};
    }

    static EdgeCaseResult fail(const std::string& code, const std::string& message) {
        return {false, code, message};
    }

    static bool isBinary(const std::string& content) {
        for (unsigned char c : content) {
            if (c == 0) return true;
        }
        return false;
    }

    static bool dfsCycle(const std::string& node,
                         const std::map<std::string, std::vector<std::string>>& graph,
                         std::set<std::string>& visited,
                         std::set<std::string>& stack) {
        if (stack.count(node)) return true;
        if (visited.count(node)) return false;
        visited.insert(node);
        stack.insert(node);

        auto it = graph.find(node);
        if (it != graph.end()) {
            for (const auto& next : it->second) {
                if (dfsCycle(next, graph, visited, stack)) return true;
            }
        }
        stack.erase(node);
        return false;
    }
};
