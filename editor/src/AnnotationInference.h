#pragma once
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Type.h"
#include "ast/Annotation.h"
#include "ast/PreprocessorNodes.h"
#include "MemoryStrategyInference.h"
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <cctype>
class AnnotationInference {
public:
    struct InferredAnnotation {
        std::string nodeId;
        std::string annotationType;
        std::string key;       // primary property name
        std::string value;     // primary property value
        std::string reason;
        double confidence;
    };

    std::vector<InferredAnnotation> inferAll(const ASTNode* root) const {
        std::vector<InferredAnnotation> out;
        if (!root) return out;

        // Delegate memory annotations to existing MemoryStrategyInference
        if (root->conceptType == "Module") {
            auto* mod = static_cast<const Module*>(root);
            MemoryStrategyInference memInf;
            auto memSuggestions = memInf.inferAnnotations(root);
            for (const auto& s : memSuggestions) {
                out.push_back({s.nodeId, s.annotationType, "strategy", s.strategy, s.reason, s.confidence});
            }
            if (mod->targetLanguage == "c") {
                inferCModule(mod, out);
            } else if (mod->targetLanguage == "wat" || mod->targetLanguage == "wasm") {
                inferWatModule(mod, out);
            }
        }

        // Infer all other annotation types
        inferNode(root, out);
        return out;
    }

private:
    void inferNode(const ASTNode* node, std::vector<InferredAnnotation>& out) const {
        if (!node) return;

        if (node->conceptType == "Function" || node->conceptType == "AsyncFunction" ||
            node->conceptType == "MethodDeclaration") {
            inferFunction(node, out);
        } else if (node->conceptType == "Variable") {
            inferVariable(node, out);
        } else if (node->conceptType == "MacroDefinition") {
            inferMacro(node, out);
        } else if (node->conceptType == "ClassDeclaration") {
            if (!hasInferred(out, node->id, "VisibilityAnnotation", "public")) {
                out.push_back({node->id, "VisibilityAnnotation", "level", "public",
                              "Class declarations default to public surface", 0.70});
            }
        }
        else if (node->conceptType == "ForLoop" || node->conceptType == "WhileLoop") {
            inferLoop(node, out);
        }

        for (auto* child : node->allChildren()) {
            inferNode(child, out);
        }
    }

    void inferFunction(const ASTNode* fn, std::vector<InferredAnnotation>& out) const {
        // Skip if already annotated
        auto annos = fn->getChildren("annotations");
        std::set<std::string> existing;
        for (const auto* a : annos) existing.insert(a->conceptType);

        // Async detection
        if (fn->conceptType == "AsyncFunction" && !existing.count("ExecAnnotation")) {
            out.push_back({fn->id, "ExecAnnotation", "mode", "async",
                          "async function detected", 0.95});
        }

        // Pure function detection (no side effects)
        if (!existing.count("PureAnnotation") && isPure(fn)) {
            out.push_back({fn->id, "PureAnnotation", "", "",
                          "No side effects detected", 0.70});
        }

        // Recursive / tail call detection
        std::string fnName;
        if (fn->conceptType == "Function") {
            fnName = static_cast<const Function*>(fn)->name;
        } else if (fn->conceptType == "MethodDeclaration") {
            fnName = static_cast<const Function*>(fn)->name;
        }
        if (!fnName.empty() && !existing.count("TailCallAnnotation")) {
            if (isTailRecursive(fn, fnName)) {
                out.push_back({fn->id, "TailCallAnnotation", "", "",
                              "Tail-recursive call detected", 0.80});
            }
        }
        inferLispFunctionPatterns(fn, existing, fnName, out);
        // Visibility inference for methods
        if (fn->conceptType == "MethodDeclaration" && !existing.count("VisibilityAnnotation")) {
            out.push_back({fn->id, "VisibilityAnnotation", "level", "public",
                          "Default method visibility", 0.60});
        }
        // Exception handling detection
        if (!existing.count("ExceptionAnnotation") && hasExceptionHandling(fn)) {
            out.push_back({fn->id, "ExceptionAnnotation", "style", "unchecked",
                          "try/catch pattern detected", 0.75});
        }
        // Blocking detection
        if (!existing.count("BlockingAnnotation") && hasBlockingCalls(fn)) {
            out.push_back({fn->id, "BlockingAnnotation", "kind", "io",
                          "IO operations detected", 0.65});
        }
        // Parallel detection (goroutine-like patterns)
        if (!existing.count("ParallelAnnotation") && hasParallelPatterns(fn)) {
            out.push_back({fn->id, "ParallelAnnotation", "kind", "task",
                          "Parallel dispatch pattern detected", 0.70});
        }
        // Complexity inference
        if (!existing.count("ComplexityAnnotation")) {
            int depth = nestingDepth(fn);
            std::string complexity = "O(1)";
            if (depth >= 2) complexity = "O(n^2)";
            else if (depth >= 1) complexity = "O(n)";
            out.push_back({fn->id, "ComplexityAnnotation", "timeComplexity", complexity,
                          "Nesting depth analysis", 0.55});
        }
    }
    void inferLoop(const ASTNode* loop, std::vector<InferredAnnotation>& out) const {
        auto annos = loop->getChildren("annotations");
        std::set<std::string> existing;
        for (const auto* a : annos) existing.insert(a->conceptType);

        if (!existing.count("LoopAnnotation")) {
            out.push_back({loop->id, "LoopAnnotation", "hint", "vectorize",
                          "Loop detected, vectorization candidate", 0.50});
        }
    }

    bool isPure(const ASTNode* fn) const {
        auto body = fn->getChildren("body");
        for (auto* stmt : body) {
            if (hasSideEffect(stmt)) return false;
        }
        return true;
    }

    bool hasSideEffect(const ASTNode* node) const {
        if (!node) return false;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            if (fc->functionName == "print" || fc->functionName == "write" ||
                fc->functionName == "send" || fc->functionName == "log" ||
                fc->functionName == "console.log")
                return true;
        }
        if (node->conceptType == "Assignment") return true;
        for (auto* child : node->allChildren()) {
            if (hasSideEffect(child)) return true;
        }
        return false;
    }

    bool isTailRecursive(const ASTNode* fn, const std::string& name) const {
        auto body = fn->getChildren("body");
        if (body.empty()) return false;
        auto* last = body.back();
        if (last->conceptType == "Return") {
            auto* val = last->getChild("value");
            if (val && val->conceptType == "FunctionCall") {
                auto* fc = static_cast<const FunctionCall*>(val);
                return fc->functionName == name;
            }
        }
        if (last->conceptType == "ExpressionStatement") {
            auto* expr = last->getChild("expression");
            if (expr && expr->conceptType == "FunctionCall") {
                auto* fc = static_cast<const FunctionCall*>(expr);
                return fc->functionName == name;
            }
        }
        return false;
    }

#include "AnnotationInferenceLisp.h"

    bool hasExceptionHandling(const ASTNode* node) const {
        if (!node) return false;
        // Look for try/catch patterns in function calls
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            if (fc->functionName == "try" || fc->functionName == "catch" ||
                fc->functionName == "throw" || fc->functionName == "raise")
                return true;
        }
        for (auto* child : node->allChildren()) {
            if (hasExceptionHandling(child)) return true;
        }
        return false;
    }

    bool hasBlockingCalls(const ASTNode* node) const {
        if (!node) return false;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            if (fc->functionName == "read" || fc->functionName == "write" ||
                fc->functionName == "open" || fc->functionName == "close" ||
                fc->functionName == "sleep" || fc->functionName == "recv" ||
                fc->functionName == "send" || fc->functionName == "connect")
                return true;
        }
        for (auto* child : node->allChildren()) {
            if (hasBlockingCalls(child)) return true;
        }
        return false;
    }

    bool hasParallelPatterns(const ASTNode* node) const {
        if (!node) return false;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            if (fc->functionName == "go" || fc->functionName == "spawn" ||
                fc->functionName == "thread" || fc->functionName == "parallel_for")
                return true;
        }
        if (node->conceptType == "ScheduleTask") return true;
        for (auto* child : node->allChildren()) {
            if (hasParallelPatterns(child)) return true;
        }
        return false;
    }

    int nestingDepth(const ASTNode* node) const {
        if (!node) return 0;
        int maxChild = 0;
        bool isLoop = (node->conceptType == "ForLoop" || node->conceptType == "WhileLoop");
        for (auto* child : node->allChildren()) {
            maxChild = std::max(maxChild, nestingDepth(child));
        }
        return maxChild + (isLoop ? 1 : 0);
    }

    // --- Step 318: Inference-to-Routing Bridge ---

public:
    // Infer Subject 9 routing annotations from code patterns.
    std::vector<InferredAnnotation> inferRoutingAnnotations(const ASTNode* node) const {
        std::vector<InferredAnnotation> out;
        if (!node) return out;

        // Check which routing annotations already exist
        auto annos = node->getChildren("annotations");
        std::set<std::string> existing;
        for (const auto* a : annos) existing.insert(a->conceptType);

        int depth = nestingDepth(node);
        bool isAsync = (node->conceptType == "AsyncFunction");
        bool hasErrorHandling = hasExceptionHandling(node);
        bool hasBlocking = hasBlockingCalls(node);
        int bodySize = countNodes(node);
        bool hasCrossFileRefs = hasCrossFileCalls(node);

        // --- ContextWidth ---
        if (!existing.count("ContextWidthAnnotation")) {
            if (hasCrossFileRefs) {
                out.push_back({node->id, "ContextWidthAnnotation", "width", "project",
                              "Cross-file references detected", 0.80});
            } else if (isAsync || bodySize > 15) {
                out.push_back({node->id, "ContextWidthAnnotation", "width", "file",
                              "Async or large function needs file context", 0.70});
            } else {
                out.push_back({node->id, "ContextWidthAnnotation", "width", "local",
                              "Simple local function", 0.75});
            }
        }

        // --- Automatability ---
        if (!existing.count("AutomatabilityAnnotation")) {
            auto bodyChildren = node->getChildren("body");
            int bodyStmts = (int)bodyChildren.size();
            if (depth >= 2 || (isAsync && hasErrorHandling)) {
                out.push_back({node->id, "AutomatabilityAnnotation", "strategy", "llm",
                              "Complex logic or async+error requires LLM", 0.75});
            } else if (bodyStmts <= 1) {
                out.push_back({node->id, "AutomatabilityAnnotation", "strategy", "deterministic",
                              "Simple getter/setter pattern", 0.85});
            } else {
                out.push_back({node->id, "AutomatabilityAnnotation", "strategy", "template",
                              "Straightforward transformation", 0.70});
            }
        }

        // --- Ambiguity (high complexity) ---
        if (!existing.count("AmbiguityAnnotation") && depth >= 2) {
            out.push_back({node->id, "AmbiguityAnnotation", "level", "high",
                          "High cyclomatic complexity (nesting depth " + std::to_string(depth) + ")",
                          0.80});
        }

        // --- Review (complex or cross-file) ---
        if (!existing.count("ReviewAnnotation") && (depth >= 2 || hasCrossFileRefs)) {
            out.push_back({node->id, "ReviewAnnotation", "required", "true",
                          "Complex or cross-file code requires human review", 0.75});
        }

        return out;
    }

    // Rough token count for the given context width.
    int estimateContextTokens(const ASTNode* node, const std::string& width) const {
        if (!node) return 0;
        int nodeTokens = countNodes(node) * 4; // ~4 tokens per AST node
        if (nodeTokens < 10) nodeTokens = 10;

        if (width == "local") {
            return nodeTokens;
        } else if (width == "file") {
            // Walk up to find module root, estimate full file
            const ASTNode* root = node;
            while (root->parent) root = root->parent;
            int fileTokens = countNodes(root) * 4;
            return std::max(fileTokens, nodeTokens);
        } else if (width == "project" || width == "cross-project") {
            // Estimate multiple files worth of context
            const ASTNode* root = node;
            while (root->parent) root = root->parent;
            int fileTokens = countNodes(root) * 4;
            return std::max(fileTokens * 5, nodeTokens); // ~5 files estimated
        }
        return nodeTokens;
    }

    // Given routing annotations, suggest worker type.
    std::string suggestWorkerType(const std::vector<InferredAnnotation>& annos) const {
        // Check for explicit automatability
        for (const auto& a : annos) {
            if (a.annotationType == "AutomatabilityAnnotation") {
                const std::string& v = a.value;
                if (v == "deterministic" || v == "template" ||
                    v == "slm" || v == "llm" || v == "human")
                    return v;
            }
        }

        // Check for review/ambiguity → human
        bool hasReview = false, hasHighAmbiguity = false;
        for (const auto& a : annos) {
            if (a.annotationType == "ReviewAnnotation") hasReview = true;
            if (a.annotationType == "AmbiguityAnnotation" &&
                (a.value == "high" || a.value == "critical"))
                hasHighAmbiguity = true;
        }
        if (hasReview && hasHighAmbiguity) return "human";

        // Check context width for hints
        for (const auto& a : annos) {
            if (a.annotationType == "ContextWidthAnnotation") {
                if (a.value == "project" || a.value == "cross-project") return "llm";
                if (a.value == "local") return "template";
            }
        }

        return "slm"; // default
    }

private:
    int countNodes(const ASTNode* node) const {
        if (!node) return 0;
        int count = 1;
        for (auto* child : node->allChildren()) {
            count += countNodes(child);
        }
        return count;
    }

    bool hasCrossFileCalls(const ASTNode* node) const {
        if (!node) return false;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            // Dotted names like "module.function" suggest cross-file reference
            if (fc->functionName.find('.') != std::string::npos)
                return true;
        }
        for (auto* child : node->allChildren()) {
            if (hasCrossFileCalls(child)) return true;
        }
        return false;
    }

    void inferCModule(const Module* mod, std::vector<InferredAnnotation>& out) const {
        if (!mod) return;
        inferCHeaderGuard(mod, out);
        for (auto* fnNode : mod->getChildren("functions")) {
            if (fnNode->conceptType != "Function") continue;
            inferCFunction(fnNode, out);
        }
    }

    void inferWatModule(const Module* mod, std::vector<InferredAnnotation>& out) const {
        if (!mod) return;

        for (auto* fnNode : mod->getChildren("functions")) {
            if (fnNode->conceptType != "Function") continue;
            if (!hasInferred(out, fnNode->id, "ExecAnnotation", "stack")) {
                out.push_back({fnNode->id, "ExecAnnotation", "mode", "stack",
                              "WAT executes in a stack-based VM", 0.95});
            }
        }

        if (!mod->getChildren("imports").empty() &&
            !hasInferred(out, mod->id, "LinkAnnotation", "import")) {
            out.push_back({mod->id, "LinkAnnotation", "kind", "import",
                          "WAT module imports host/runtime functions", 0.90});
        }

        bool hasExport = false;
        bool hasTable = false;
        bool hasMemory = false;
        for (auto* stmt : mod->getChildren("statements")) {
            if (stmt->conceptType != "PragmaDirective") continue;
            auto* p = static_cast<const PragmaDirective*>(stmt);
            std::string d = lowerAscii(p->directive);
            hasExport = hasExport || d == "export";
            hasTable = hasTable || d == "table";
            hasMemory = hasMemory || d == "memory";
        }

        if (hasExport && !hasInferred(out, mod->id, "VisibilityAnnotation", "public")) {
            out.push_back({mod->id, "VisibilityAnnotation", "level", "public",
                          "Exported WAT symbols are public API", 0.90});
        }
        if (hasTable && !hasInferred(out, mod->id, "ShimAnnotation", "vtable")) {
            out.push_back({mod->id, "ShimAnnotation", "kind", "vtable",
                          "WAT table dispatch resembles virtual table shims", 0.82});
        }
        if (hasMemory && !hasInferred(out, mod->id, "AlignAnnotation", "4")) {
            out.push_back({mod->id, "AlignAnnotation", "bytes", "4",
                          "Linear memory defaults to aligned word operations", 0.70});
        }
        if (hasMemory && !hasInferred(out, mod->id, "LayoutAnnotation", "linear")) {
            out.push_back({mod->id, "LayoutAnnotation", "mode", "linear",
                          "WAT memory is a linear buffer layout", 0.86});
        }
    }

    void inferCHeaderGuard(const Module* mod, std::vector<InferredAnnotation>& out) const {
        bool hasIfndef = false;
        bool hasEndif = false;
        for (auto* stmt : mod->getChildren("statements")) {
            if (stmt->conceptType != "PragmaDirective") continue;
            auto* pragma = static_cast<const PragmaDirective*>(stmt);
            std::string d = lowerAscii(pragma->directive);
            if (startsWith(d, "ifndef") || startsWith(d, "ifdef")) hasIfndef = true;
            if (startsWith(d, "endif")) hasEndif = true;
        }
        if (hasIfndef && hasEndif &&
            !hasInferred(out, mod->id, "SyntheticAnnotation", "guard")) {
            out.push_back({mod->id, "SyntheticAnnotation", "generator", "guard",
                          "C header guard pattern detected", 0.93});
        }
    }

    void inferCFunction(const ASTNode* fn, std::vector<InferredAnnotation>& out) const {
        std::set<std::string> existing;
        for (const auto* a : fn->getChildren("annotations")) {
            existing.insert(a->conceptType);
        }

        bool hasGoto = false;
        bool hasVoidPtrPattern = false;
        bool hasArrayAccess = false;
        bool hasBoundsCheck = false;
        for (auto* bodyNode : fn->getChildren("body")) {
            hasGoto = hasGoto || hasGotoPattern(bodyNode);
            hasVoidPtrPattern = hasVoidPtrPattern || hasVoidPtrPatternNode(bodyNode);
            hasArrayAccess = hasArrayAccess || hasArrayAccessNode(bodyNode);
            hasBoundsCheck = hasBoundsCheck || hasBoundsCheckNode(bodyNode);
        }

        if (hasGoto && !existing.count("ComplexityAnnotation")) {
            out.push_back({fn->id, "ComplexityAnnotation", "timeComplexity", "high",
                          "goto usage increases control-flow complexity", 0.90});
        }
        if (hasGoto && !existing.count("RiskAnnotation")) {
            out.push_back({fn->id, "RiskAnnotation", "level", "medium",
                          "goto usage is error-prone in C code paths", 0.82});
        }
        if (hasVoidPtrPattern && !existing.count("RiskAnnotation")) {
            out.push_back({fn->id, "RiskAnnotation", "level", "high",
                          "void* conversion erases type guarantees", 0.90});
        }
        if (hasVoidPtrPattern && !existing.count("AmbiguityAnnotation")) {
            out.push_back({fn->id, "AmbiguityAnnotation", "level", "medium",
                          "void* usage introduces type ambiguity", 0.84});
        }
        if (hasArrayAccess && !hasBoundsCheck && !existing.count("BoundsCheckAnnotation")) {
            out.push_back({fn->id, "BoundsCheckAnnotation", "mode", "unchecked",
                          "Array access without guard detected", 0.80});
        }
    }

    bool hasGotoPattern(const ASTNode* node) const {
        if (!node) return false;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            if (lowerAscii(fc->functionName) == "goto") return true;
        }
        for (auto* child : node->allChildren()) {
            if (hasGotoPattern(child)) return true;
        }
        return false;
    }

    bool hasVoidPtrPatternNode(const ASTNode* node) const {
        if (!node) return false;
        if (node->conceptType == "Variable") {
            auto* typeNode = node->getChild("type");
            if (containsVoidPtr(typeNode)) return true;
        }
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            if (lowerAscii(fc->functionName).find("void*") != std::string::npos) return true;
        }
        for (auto* child : node->allChildren()) {
            if (hasVoidPtrPatternNode(child)) return true;
        }
        return false;
    }

    bool hasArrayAccessNode(const ASTNode* node) const {
        if (!node) return false;
        if (node->conceptType == "IndexAccess") return true;
        for (auto* child : node->allChildren()) {
            if (hasArrayAccessNode(child)) return true;
        }
        return false;
    }

    bool hasBoundsCheckNode(const ASTNode* node) const {
        if (!node) return false;
        if (node->conceptType == "BoundsCheckAnnotation") return true;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            std::string fn = lowerAscii(fc->functionName);
            if (fn == "bounds_check" || fn == "assert" || fn == "check_bounds") return true;
        }
        for (auto* child : node->allChildren()) {
            if (hasBoundsCheckNode(child)) return true;
        }
        return false;
    }

    static bool containsVoidPtr(const ASTNode* typeNode) {
        if (!typeNode) return false;
        if (typeNode->conceptType == "PrimitiveType") {
            auto* p = static_cast<const PrimitiveType*>(typeNode);
            return lowerAscii(p->kind).find("void*") != std::string::npos;
        }
        if (typeNode->conceptType == "CustomType") {
            auto* c = static_cast<const CustomType*>(typeNode);
            return lowerAscii(c->typeName).find("void*") != std::string::npos;
        }
        return false;
    }

    static bool hasInferred(const std::vector<InferredAnnotation>& out,
                            const std::string& nodeId,
                            const std::string& annotationType,
                            const std::string& value) {
        for (const auto& a : out) {
            if (a.nodeId == nodeId && a.annotationType == annotationType && a.value == value) {
                return true;
            }
        }
        return false;
    }

    static std::string lowerAscii(const std::string& value) {
        std::string out = value;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    static bool startsWith(const std::string& value, const std::string& prefix) {
        return value.rfind(prefix, 0) == 0;
    }
};
