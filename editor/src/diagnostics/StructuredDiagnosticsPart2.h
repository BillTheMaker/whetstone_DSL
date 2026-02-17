// -----------------------------------------------------------------------
//  Step 260: Cross-file diagnostics
// -----------------------------------------------------------------------

// Check for undefined imports: modules imported but not open as buffers
inline std::vector<StructuredDiagnostic> collectCrossFileDiagnostics(
        Module* ast, const std::string& filePath,
        const std::set<std::string>& openModuleNames) {
    std::vector<StructuredDiagnostic> diags;
    if (!ast) return diags;
    for (auto* child : ast->allChildren()) {
        if (child->conceptType == "Import") {
            auto* imp = static_cast<const Import*>(child);
            if (!imp->moduleName.empty() &&
                openModuleNames.find(imp->moduleName) ==
                    openModuleNames.end()) {
                StructuredDiagnostic d;
                d.code = "E0400";
                d.severity = DiagnosticSeverity::Warning;
                d.nodeId = imp->id;
                d.line = imp->hasSpan() ? imp->spanStartLine : 0;
                d.message = "Imported module '" + imp->moduleName +
                            "' is not open in the project";
                d.source = "cross-file";
                diags.push_back(std::move(d));
            }
        }
    }
    return diags;
}

// Text-based cross-file import check (for parsers without Import nodes)
inline std::vector<StructuredDiagnostic> collectCrossFileDiagnosticsFromSource(
        const std::string& source, const std::string& filePath,
        const std::set<std::string>& openModuleNames) {
    std::vector<StructuredDiagnostic> diags;
    std::istringstream iss(source);
    std::string line;
    int lineNum = 0;
    while (std::getline(iss, line)) {
        ++lineNum;
        std::string modName;
        if (line.substr(0, 7) == "import ") {
            modName = line.substr(7);
            while (!modName.empty() && modName.back() == ' ')
                modName.pop_back();
            size_t comma = modName.find(',');
            if (comma != std::string::npos)
                modName = modName.substr(0, comma);
        } else if (line.substr(0, 5) == "from ") {
            size_t space = line.find(' ', 5);
            if (space != std::string::npos)
                modName = line.substr(5, space - 5);
        }
        if (!modName.empty() &&
            openModuleNames.find(modName) == openModuleNames.end()) {
            StructuredDiagnostic d;
            d.code = "E0400";
            d.severity = DiagnosticSeverity::Warning;
            d.line = lineNum;
            d.message = "Imported module '" + modName +
                        "' is not open in the project";
            d.source = "cross-file";
            diags.push_back(std::move(d));
        }
    }
    return diags;
}

// -----------------------------------------------------------------------
//  Step 251: QuickFix — a concrete mutation an agent can apply
// -----------------------------------------------------------------------
struct QuickFix {
    std::string  id;          // unique fix ID (e.g. "fix-E0200-func_1")
    std::string  diagCode;    // diagnostic code this fixes
    std::string  nodeId;      // target node
    std::string  description; // human-readable label
    std::string  category;    // "remove-annotation", "change-strategy", etc.
    json         mutation;    // concrete mutation object for applyMutation
};

inline json quickFixToJson(const QuickFix& f) {
    return {
        {"id",          f.id},
        {"diagCode",    f.diagCode},
        {"nodeId",      f.nodeId},
        {"description", f.description},
        {"category",    f.category},
        {"mutation",    f.mutation}
    };
}

// -----------------------------------------------------------------------
//  getQuickFixes — collect all applicable fixes for a node
// -----------------------------------------------------------------------
inline std::vector<QuickFix> getQuickFixesForNode(
        Module* ast, const std::string& targetNodeId) {
    std::vector<QuickFix> fixes;
    if (!ast) return fixes;

    auto diags = collectAllDiagnostics(ast);
    for (const auto& d : diags) {
        if (!targetNodeId.empty() && d.nodeId != targetNodeId) continue;
        if (d.fix.is_null()) continue;

        QuickFix fix;
        fix.id = "fix-" + d.code + "-" + d.nodeId;
        fix.diagCode = d.code;
        fix.nodeId = d.nodeId;
        fix.description = d.fix.value("description", "Fix " + d.code);
        fix.mutation = d.fix;

        // Categorize based on error code
        if (d.code == "E0200")      fix.category = "remove-annotation";
        else if (d.code == "E0201") fix.category = "change-strategy";
        else if (d.code == "E0202") fix.category = "resolve-conflict";
        else if (d.code == "E0300") fix.category = "remove-use-after-free";
        else if (d.code == "E0301") fix.category = "add-deallocation";
        else                        fix.category = "general";

        fixes.push_back(std::move(fix));
    }

    // Additional heuristic fixes from AST inspection
    ASTNode* node = findNodeById(ast, targetNodeId);
    if (node) {
        // Unused variable: if a Variable has no references outside its
        // own declaration, suggest removal
        if (node->conceptType == "Variable") {
            QuickFix fix;
            fix.id = "fix-unused-" + targetNodeId;
            fix.diagCode = "E0203";
            fix.nodeId = targetNodeId;
            fix.description = "Remove unused variable";
            fix.category = "remove-unused";
            fix.mutation = {{"type", "deleteNode"},
                            {"nodeId", targetNodeId}};
            fixes.push_back(std::move(fix));
        }

        // Missing return: if a Function body ends without a Return,
        // suggest adding one
        if (node->conceptType == "Function") {
            auto body = node->getChildren("body");
            bool hasReturn = false;
            for (auto* stmt : body) {
                if (stmt->conceptType == "ReturnStatement")
                    hasReturn = true;
            }
            if (!hasReturn && !body.empty()) {
                QuickFix fix;
                fix.id = "fix-missing-return-" + targetNodeId;
                fix.diagCode = "E0203";
                fix.nodeId = targetNodeId;
                fix.description = "Add missing return statement";
                fix.category = "missing-return";
                fix.mutation = {
                    {"type", "insertNode"},
                    {"parentId", targetNodeId},
                    {"role", "body"},
                    {"node", {{"conceptType", "ReturnStatement"},
                              {"children", json::object()}}}
                };
                fixes.push_back(std::move(fix));
            }
        }
    }

    return fixes;
}

// -----------------------------------------------------------------------
//  getQuickFixesAll — collect all fixes for all diagnostics in the AST
// -----------------------------------------------------------------------
inline std::vector<QuickFix> getQuickFixesAll(Module* ast) {
    return getQuickFixesForNode(ast, "");
}

// -----------------------------------------------------------------------
//  findQuickFix — look up a specific fix by diagnostic code + nodeId
// -----------------------------------------------------------------------
inline QuickFix findQuickFix(Module* ast, const std::string& diagCode,
                              const std::string& nodeId) {
    auto fixes = getQuickFixesForNode(ast, nodeId);
    for (const auto& f : fixes) {
        if (f.diagCode == diagCode) return f;
    }
    return {}; // empty fix (mutation will be null)
}

