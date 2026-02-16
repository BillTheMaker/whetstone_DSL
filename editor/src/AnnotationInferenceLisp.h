    void inferLispFunctionPatterns(const ASTNode* fn,
                                   const std::set<std::string>& existing,
                                   const std::string& fnName,
                                   std::vector<InferredAnnotation>& out) const {
        if (hasCallNamed(fn, "values") && !existing.count("ContractAnnotation")) {
            out.push_back({fn->id, "ContractAnnotation", "returns", "multiple-values",
                          "values form indicates multi-value return contract", 0.88});
        }
        if (hasDeclaredTypePattern(fn) && !existing.count("ComplexityAnnotation")) {
            out.push_back({fn->id, "ComplexityAnnotation", "timeComplexity", "declared-type",
                          "Type declaration forms constrain complexity interpretation", 0.72});
        }
        if (hasOptimizePattern(fn) && !hasInferred(out, fn->id, "PolicyAnnotation", "critical")) {
            out.push_back({fn->id, "PolicyAnnotation", "perf", "critical",
                          "optimize speed hint implies performance-critical policy", 0.90});
            out.push_back({fn->id, "PolicyAnnotation", "style", "literal",
                          "optimize declaration favors literal policy output", 0.86});
        }
        if (fn->conceptType == "MethodDeclaration" && fnName.size() > 0 &&
            !hasInferred(out, fn->id, "SyntheticAnnotation", "clos-method")) {
            out.push_back({fn->id, "SyntheticAnnotation", "generator", "clos-method",
                          "Method declaration maps to CLOS generic dispatch", 0.76});
        }
    }

    void inferVariable(const ASTNode* varNode, std::vector<InferredAnnotation>& out) const {
        auto* var = static_cast<const Variable*>(varNode);
        if (var->name.size() > 2 && var->name.front() == '*' && var->name.back() == '*' &&
            !hasInferred(out, var->id, "BindingAnnotation", "dynamic")) {
            out.push_back({var->id, "BindingAnnotation", "time", "dynamic",
                          "Earmuff naming indicates dynamic/special binding", 0.95});
        } else if (varNode->parent && varNode->parent->conceptType == "Block" &&
                   !hasInferred(out, var->id, "BindingAnnotation", "static")) {
            out.push_back({var->id, "BindingAnnotation", "time", "static",
                          "Lexical block variable inferred as static binding", 0.70});
        }
    }

    void inferMacro(const ASTNode* macroNode, std::vector<InferredAnnotation>& out) const {
        if (!hasInferred(out, macroNode->id, "MetaAnnotation", "quoted")) {
            out.push_back({macroNode->id, "MetaAnnotation", "state", "quoted",
                          "Macro definitions operate over quoted forms", 0.92});
        }
        if (!hasInferred(out, macroNode->id, "SyntheticAnnotation", "macro")) {
            out.push_back({macroNode->id, "SyntheticAnnotation", "generator", "macro",
                          "Macro expands synthetic code structures", 0.90});
        }
    }

    bool hasCallNamed(const ASTNode* node, const std::string& name) const {
        if (!node) return false;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            if (lowerAscii(fc->functionName) == lowerAscii(name)) return true;
        }
        for (auto* child : node->allChildren()) {
            if (hasCallNamed(child, name)) return true;
        }
        return false;
    }

    bool hasDeclaredTypePattern(const ASTNode* node) const {
        if (!node) return false;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            if (lowerAscii(fc->functionName) == "declare") {
                for (auto* arg : fc->getChildren("arguments")) {
                    if (arg->conceptType != "FunctionCall") continue;
                    auto* inner = static_cast<const FunctionCall*>(arg);
                    if (lowerAscii(inner->functionName) == "type") return true;
                }
            }
        }
        for (auto* child : node->allChildren()) {
            if (hasDeclaredTypePattern(child)) return true;
        }
        return false;
    }

    bool hasOptimizePattern(const ASTNode* node) const {
        if (!node) return false;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            std::string fn = lowerAscii(fc->functionName);
            if (fn == "optimize") return true;
            if (fn == "declare") {
                for (auto* arg : fc->getChildren("arguments")) {
                    if (arg->conceptType != "FunctionCall") continue;
                    auto* inner = static_cast<const FunctionCall*>(arg);
                    if (lowerAscii(inner->functionName) == "optimize") return true;
                }
            }
        }
        for (auto* child : node->allChildren()) {
            if (hasOptimizePattern(child)) return true;
        }
        return false;
    }
