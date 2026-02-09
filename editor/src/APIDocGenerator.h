#pragma once
#include <string>
#include <vector>
#include <sstream>

class APIDocGenerator {
public:
    struct APIEntry {
        std::string component;
        std::string category;    // "parser", "generator", "validator", "optimizer", "editor", "api"
        std::string description;
        std::vector<std::string> methods;
    };

    std::vector<APIEntry> getAPIEntries() const {
        return {
            // Parsers
            {"TreeSitterParser", "parser",
             "Parses source code (Python, C++, Elisp) into Whetstone AST using tree-sitter grammars",
             {"parsePython(source)", "parsePythonWithDiagnostics(source)",
              "parseCpp(source)", "parseCppWithDiagnostics(source)",
              "parseElisp(source)", "parseElispWithDiagnostics(source)"}},

            // Generators
            {"PythonGenerator", "generator",
             "Generates Python source code from Whetstone AST",
             {"generate(node)"}},

            {"CppGenerator", "generator",
             "Generates C++ source code from Whetstone AST with memory annotation support",
             {"generate(node)"}},

            {"ElispGenerator", "generator",
             "Generates Emacs Lisp source code from Whetstone AST",
             {"generate(node)"}},

            // Validators
            {"AnnotationValidator", "validator",
             "Validates memory annotations for correctness: missing intent, aliasing, conflicts",
             {"validate(root)"}},

            {"StrategyValidator", "validator",
             "Post-optimization invariant checking: use-after-free, leak, aliasing, destructor reachability",
             {"validateInvariants(root)"}},

            // Optimizers
            {"TransformEngine", "optimizer",
             "AST transformation engine with constant folding and dead code elimination",
             {"setRoot(root)", "constantFolding()", "deadCodeElimination()", "applyAll()"}},

            {"StrategyAwareOptimizer", "optimizer",
             "Annotation-constrained optimization respecting memory strategy rules",
             {"setRoot(root)", "inlineVariable(id)", "reorderStatements(id)",
              "duplicateNode(id)", "optimizeFunction(id)"}},

            {"IncrementalOptimizer", "optimizer",
             "Incremental transforms with journal tracking, undo support, and provenance",
             {"setRoot(root)", "applyTransform(name)", "getTransformHistory()",
              "undoTransform(id)", "undoLast()", "getProvenance(nodeId)"}},

            // Analysis
            {"MemoryStrategyInference", "analysis",
             "Infers appropriate memory annotations from language and usage patterns",
             {"inferAnnotations(root)"}},

            {"CrossLanguageProjector", "analysis",
             "Deep-copies AST with language change and annotation adaptation",
             {"project(source, targetLanguage)", "annotationsPreserved(original, projected)"}},

            {"ContextAPI", "api",
             "Scope analysis, call hierarchy, and data-flow dependency graph",
             {"getInScopeSymbols(nodeId)", "getCallHierarchy(functionId)",
              "getDependencyGraph(functionId)"}},

            // Mutation APIs
            {"ASTMutationAPI", "api",
             "AST mutation with OptimizationLock checking and journal recording",
             {"setProperty(nodeId, key, value)", "updateNode(nodeId, properties)",
              "deleteNode(nodeId)", "insertNode(parentId, role, node)"}},

            {"BatchMutationAPI", "api",
             "Atomic batch mutations with reverse-order rollback on failure",
             {"applySequence(mutations)"}},

            // Editor components
            {"TextEditor", "editor",
             "Classical text editor with undo/redo, find/replace, and selection",
             {"insertText(text)", "deleteRange(start, end)", "undo()", "redo()",
              "find(query)", "replace(query, replacement)", "getSelection()"}},

            {"TextASTSync", "editor",
             "Bidirectional text-to-AST synchronization with debounce",
             {"syncTextToAST(text)", "syncASTToText(ast)", "setDebounceMs(ms)"}},

            {"SyntaxHighlighter", "editor",
             "Tree-sitter CST walk producing colored token spans for Python, C++, and Elisp",
             {"highlight(source, language)"}},

            {"KeybindingManager", "editor",
             "Configurable keybinding profiles: VSCode (default), JetBrains, Emacs",
             {"setProfile(profile)", "getBinding(action)", "setCustomBinding(action, combo)"}},

            {"BufferManager", "editor",
             "Multi-buffer management for open/close/switch operations",
             {"openBuffer(path)", "closeBuffer(id)", "switchBuffer(id)",
              "getActiveBuffer()", "listBuffers()"}},

            {"EditorMode", "editor",
             "Per-language editing behavior: indentation, comments, brackets, snippets",
             {"setLanguage(language)", "getIndentRule()", "getCommentStyle()",
              "getBracketPairs()", "getSnippets()"}},

            // Infrastructure
            {"Pipeline", "infrastructure",
             "End-to-end pipeline: parse → infer → validate → optimize → generate",
             {"run(source, sourceLanguage, targetLanguage)", "parse(source, language)",
              "generate(ast, language)"}},

            {"WebSocketAgentServer", "infrastructure",
             "WebSocket endpoint for external agent integration with JSON-RPC routing",
             {"start(port)", "registerHandler(method, handler)",
              "listSessions()", "getSessionInfo(id)"}},

            {"ASTSchema", "infrastructure",
             "AST schema validation against concept rules",
             {"validate(node)", "addRule(conceptType, rule)"}},
        };
    }

    std::string generateMarkdown() const {
        std::ostringstream out;
        auto entries = getAPIEntries();

        out << "# Whetstone DSL API Reference\n\n";
        out << "**Components:** " << entries.size() << "\n\n";

        std::string currentCategory;
        for (const auto& entry : entries) {
            if (entry.category != currentCategory) {
                currentCategory = entry.category;
                out << "## " << capitalize(currentCategory) << "\n\n";
            }

            out << "### " << entry.component << "\n\n";
            out << entry.description << "\n\n";
            out << "**Methods:**\n";
            for (const auto& method : entry.methods) {
                out << "- `" << method << "`\n";
            }
            out << "\n";
        }

        return out.str();
    }

    // Verify that all major API categories are covered
    bool verifyCoverage() const {
        auto entries = getAPIEntries();
        bool hasParser = false, hasGenerator = false, hasValidator = false;
        bool hasOptimizer = false, hasEditor = false, hasAPI = false;

        for (const auto& e : entries) {
            if (e.category == "parser") hasParser = true;
            if (e.category == "generator") hasGenerator = true;
            if (e.category == "validator") hasValidator = true;
            if (e.category == "optimizer") hasOptimizer = true;
            if (e.category == "editor") hasEditor = true;
            if (e.category == "api") hasAPI = true;
        }

        return hasParser && hasGenerator && hasValidator &&
               hasOptimizer && hasEditor && hasAPI;
    }

private:
    static std::string capitalize(const std::string& s) {
        if (s.empty()) return s;
        std::string result = s;
        result[0] = static_cast<char>(std::toupper(result[0]));
        return result;
    }
};
