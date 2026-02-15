#pragma once
// TreeSitterParser Cpp support.
public:
    //  C++
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseCpp(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_cpp());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_cpp_module";
        module->targetLanguage = "cpp";
        applySpan(module.get(), root);

        convertCppTranslationUnit(root, source, module.get());

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseCppWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_cpp());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_cpp_module";
        result.module->targetLanguage = "cpp";
        applySpan(result.module.get(), root);

        convertCppTranslationUnit(root, source, result.module.get());
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
private:
    //  C++ CST â†’ AST
    // ---------------------------------------------------------------
    static void convertCppTranslationUnit(TSNode root, const std::string& source, Module* module) {
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "function_definition") {
                auto* fn = convertCppFunction(child, source);
                if (fn) module->addChild("functions", fn);
            } else if (type == "class_specifier" || type == "struct_specifier") {
                convertCppClassOrStruct(child, source, module, type == "struct_specifier");
            } else if (type == "template_declaration") {
                convertCppTemplateDecl(child, source, module);
            }
        }
    }

    static void convertCppClassOrStruct(TSNode node, const std::string& source,
                                        Module* module, bool isStruct) {
        TSNode nameNode = childByFieldName(node, "name");
        std::string className;
        if (!ts_node_is_null(nameNode)) {
            className = nodeText(nameNode, source);
        }
        if (className.empty()) return;

        auto* cls = new ClassDeclaration(IdGenerator::next("cls"), className);
        applySpan(cls, node);

        // Check for base class
        TSNode baseClause = childByFieldName(node, "base_clause");
        if (ts_node_is_null(baseClause)) {
            // tree-sitter-cpp might use different field names — search children
            uint32_t nc = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < nc; ++i) {
                TSNode ch = ts_node_named_child(node, i);
                if (nodeType(ch) == "base_class_clause") {
                    baseClause = ch;
                    break;
                }
            }
        }
        if (!ts_node_is_null(baseClause)) {
            uint32_t bc = ts_node_named_child_count(baseClause);
            for (uint32_t i = 0; i < bc; ++i) {
                TSNode base = ts_node_named_child(baseClause, i);
                std::string baseText = nodeText(base, source);
                // Strip access specifier prefix
                if (baseText.find("public ") == 0) baseText = baseText.substr(7);
                else if (baseText.find("private ") == 0) baseText = baseText.substr(8);
                else if (baseText.find("protected ") == 0) baseText = baseText.substr(10);
                if (!baseText.empty() && cls->superClass.empty()) {
                    cls->superClass = baseText;
                }
            }
        }

        // Process class body
        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            std::string currentVisibility = isStruct ? "public" : "private";
            uint32_t bc = ts_node_named_child_count(bodyNode);
            for (uint32_t i = 0; i < bc; ++i) {
                TSNode member = ts_node_named_child(bodyNode, i);
                std::string memberType = nodeType(member);
                if (memberType == "access_specifier") {
                    std::string specText = nodeText(member, source);
                    if (specText.find("public") != std::string::npos) currentVisibility = "public";
                    else if (specText.find("private") != std::string::npos) currentVisibility = "private";
                    else if (specText.find("protected") != std::string::npos) currentVisibility = "protected";
                } else if (memberType == "function_definition") {
                    auto* meth = convertCppMethodDecl(member, source, className, currentVisibility);
                    if (meth) cls->addChild("methods", meth);
                } else if (memberType == "declaration") {
                    // Could be a field declaration
                    TSNode declNode = childByFieldName(member, "declarator");
                    if (!ts_node_is_null(declNode)) {
                        std::string fieldName = nodeText(declNode, source);
                        auto* var = new Variable(IdGenerator::next("var"), fieldName);
                        applySpan(var, member);
                        cls->addChild("fields", var);
                    } else {
                        // Try to find field names from named children
                        uint32_t dc = ts_node_named_child_count(member);
                        for (uint32_t d = 0; d < dc; ++d) {
                            TSNode dchild = ts_node_named_child(member, d);
                            std::string dtype = nodeType(dchild);
                            if (dtype == "field_identifier" || dtype == "identifier") {
                                // Skip type specifiers — just get the last identifier-like thing
                            }
                        }
                    }
                } else if (memberType == "field_declaration") {
                    TSNode declNode = childByFieldName(member, "declarator");
                    if (!ts_node_is_null(declNode)) {
                        std::string fieldName = nodeText(declNode, source);
                        auto* var = new Variable(IdGenerator::next("var"), fieldName);
                        applySpan(var, member);
                        cls->addChild("fields", var);
                    }
                }
            }
        }
        module->addChild("classes", cls);
    }

    static MethodDeclaration* convertCppMethodDecl(TSNode node, const std::string& source,
                                                    const std::string& className,
                                                    const std::string& visibility) {
        auto* meth = new MethodDeclaration();
        meth->id = IdGenerator::next("meth");
        meth->className = className;
        meth->visibility = visibility;
        applySpan(meth, node);

        // Check for virtual keyword
        TSNode typeNode = childByFieldName(node, "type");
        if (!ts_node_is_null(typeNode)) {
            std::string typeText = nodeText(typeNode, source);
            auto* retType = new PrimitiveType(IdGenerator::next("type"), typeText);
            meth->setChild("returnType", retType);
        }

        // Check full text for virtual/static keywords
        std::string fullText = nodeText(node, source);
        if (fullText.find("virtual ") != std::string::npos) meth->isVirtual = true;
        if (fullText.find("static ") != std::string::npos) meth->isStatic = true;
        if (fullText.find("override") != std::string::npos) meth->isOverride = true;

        // Declarator
        TSNode declaratorNode = childByFieldName(node, "declarator");
        if (!ts_node_is_null(declaratorNode)) {
            extractCppFunctionName(declaratorNode, source, meth);
            extractCppParameters(declaratorNode, source, meth);
        }

        // Body
        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertCppBody(bodyNode, source, meth);
        }

        return meth;
    }

    static void convertCppTemplateDecl(TSNode node, const std::string& source, Module* module) {
        // template_declaration has template_parameter_list and a child declaration
        TSNode paramsNode = childByFieldName(node, "parameters");
        std::vector<std::pair<std::string, std::string>> typeParams; // name, constraint

        if (!ts_node_is_null(paramsNode)) {
            uint32_t pc = ts_node_named_child_count(paramsNode);
            for (uint32_t p = 0; p < pc; ++p) {
                TSNode param = ts_node_named_child(paramsNode, p);
                std::string paramType = nodeType(param);
                if (paramType == "type_parameter_declaration" ||
                    paramType == "template_type_parameter") {
                    TSNode pname = childByFieldName(param, "name");
                    if (!ts_node_is_null(pname)) {
                        typeParams.push_back({nodeText(pname, source), ""});
                    } else {
                        // Fallback: get all text
                        std::string txt = nodeText(param, source);
                        // Extract name from "typename T" or "class T"
                        size_t sp = txt.rfind(' ');
                        if (sp != std::string::npos) {
                            typeParams.push_back({txt.substr(sp + 1), ""});
                        } else {
                            typeParams.push_back({txt, ""});
                        }
                    }
                }
            }
        }

        // Find the inner declaration (class_specifier, struct_specifier, function_definition)
        uint32_t nc = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < nc; ++i) {
            TSNode child = ts_node_named_child(node, i);
            std::string childType = nodeType(child);
            if (childType == "class_specifier" || childType == "struct_specifier") {
                convertCppClassOrStruct(child, source, module, childType == "struct_specifier");
                // Attach TypeParameters to the last class
                auto& classes = module->getChildren("classes");
                if (!classes.empty()) {
                    auto* cls = dynamic_cast<ClassDeclaration*>(classes.back());
                    if (cls) {
                        for (auto& [tpName, tpConstraint] : typeParams) {
                            auto* tp = new TypeParameter(IdGenerator::next("tp"), tpName);
                            tp->constraint = tpConstraint;
                            cls->addChild("typeParameters", tp);
                        }
                    }
                }
                return;
            } else if (childType == "function_definition") {
                auto* fn = convertCppFunction(child, source);
                if (fn) module->addChild("functions", fn);
                return;
            } else if (childType == "declaration") {
                // Template function declaration without body — skip for now
                return;
            }
        }
    }

    static Function* convertCppFunction(TSNode node, const std::string& source) {
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);

        // In C++ grammar, the structure is:
        //   function_definition: type declarator body
        // The declarator contains the function name and parameters.

        // Return type
        TSNode typeNode = childByFieldName(node, "type");
        if (!ts_node_is_null(typeNode)) {
            std::string typeText = nodeText(typeNode, source);
            auto* retType = new PrimitiveType(IdGenerator::next("type"), typeText);
            fn->setChild("returnType", retType);
        }

        // Declarator: function_declarator which has declarator (name) and parameters
        TSNode declaratorNode = childByFieldName(node, "declarator");
        if (!ts_node_is_null(declaratorNode)) {
            extractCppFunctionName(declaratorNode, source, fn);
            extractCppParameters(declaratorNode, source, fn);
        }

        // Body
        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertCppBody(bodyNode, source, fn);
        }

        // Memory pattern detection from source text
        std::string bodySource = "";
        if (!ts_node_is_null(bodyNode)) {
            bodySource = nodeText(bodyNode, source);
        }
        detectCppMemoryPatterns(bodySource, fn);

        return fn;
    }

    static void extractCppFunctionName(TSNode declNode, const std::string& source, Function* fn) {
        std::string type = nodeType(declNode);
        if (type == "function_declarator") {
            TSNode nameNode = childByFieldName(declNode, "declarator");
            if (!ts_node_is_null(nameNode)) {
                // Could be an identifier directly, or nested further
                fn->name = nodeText(nameNode, source);
            }
        } else if (type == "identifier") {
            fn->name = nodeText(declNode, source);
        } else {
            // Try to find function_declarator among children
            uint32_t count = ts_node_named_child_count(declNode);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(declNode, i);
                std::string childType = nodeType(child);
                if (childType == "function_declarator") {
                    extractCppFunctionName(child, source, fn);
                    return;
                }
            }
            // Fallback
            fn->name = nodeText(declNode, source);
        }
    }

    static void extractCppParameters(TSNode declNode, const std::string& source, Function* fn) {
        std::string type = nodeType(declNode);
        if (type == "function_declarator") {
            TSNode paramsNode = childByFieldName(declNode, "parameters");
            if (!ts_node_is_null(paramsNode)) {
                uint32_t count = ts_node_named_child_count(paramsNode);
                for (uint32_t i = 0; i < count; ++i) {
                    TSNode paramChild = ts_node_named_child(paramsNode, i);
                    std::string paramType = nodeType(paramChild);
                    if (paramType == "parameter_declaration") {
                        convertCppParameter(paramChild, source, fn);
                    }
                }
            }
        } else {
            // Search for function_declarator child
            uint32_t count = ts_node_named_child_count(declNode);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(declNode, i);
                if (nodeType(child) == "function_declarator") {
                    extractCppParameters(child, source, fn);
                    return;
                }
            }
        }
    }

    static void convertCppParameter(TSNode paramNode, const std::string& source, Function* fn) {
        auto* param = new Parameter();
        param->id = IdGenerator::next("param");
        applySpan(param, paramNode);

        // parameter_declaration has type and declarator fields
        TSNode typeNode = childByFieldName(paramNode, "type");
        TSNode declNode = childByFieldName(paramNode, "declarator");

        if (!ts_node_is_null(typeNode)) {
            std::string typeText = nodeText(typeNode, source);
            auto* primType = new PrimitiveType(IdGenerator::next("type"), typeText);
            applySpan(primType, typeNode);
            param->setChild("type", primType);
        }

        if (!ts_node_is_null(declNode)) {
            param->name = nodeText(declNode, source);
        }

        fn->addChild("parameters", param);
    }

    static void convertCppBody(TSNode bodyNode, const std::string& source, Function* fn) {
        // bodyNode is a compound_statement { ... }
        uint32_t count = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(bodyNode, i);
            ASTNode* stmt = convertCppStatement(child, source);
            if (stmt) fn->addChild("body", stmt);
        }
    }

    static ASTNode* convertCppStatement(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "return_statement") {
            auto* ret = new Return();
            ret->id = IdGenerator::next("ret");
            applySpan(ret, node);
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) {
                ASTNode* val = convertCppExpression(ts_node_named_child(node, 0), source);
                if (val) ret->setChild("value", val);
            }
            return ret;
        } else if (type == "expression_statement") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) {
                ASTNode* expr = convertCppExpression(ts_node_named_child(node, 0), source);
                if (expr) exprStmt->setChild("expression", expr);
            }
            return exprStmt;
        } else if (type == "declaration") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            // Scan for lambda expressions inside declarations
            ASTNode* lambdaNode = findCppLambdaInSubtree(node, source);
            if (lambdaNode) exprStmt->setChild("expression", lambdaNode);
            return exprStmt;
        } else if (type == "if_statement") {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            return ifStmt;
        }
        return nullptr;
    }

    // Recursively scan for lambda_expression nodes in a subtree
    static ASTNode* findCppLambdaInSubtree(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "lambda_expression") {
            return convertCppLambdaExpression(node, source);
        }
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            ASTNode* found = findCppLambdaInSubtree(ts_node_named_child(node, i), source);
            if (found) return found;
        }
        return nullptr;
    }

    static LambdaExpression* convertCppLambdaExpression(TSNode node, const std::string& source) {
        auto* lambda = new LambdaExpression(IdGenerator::next("lambda"));
        applySpan(lambda, node);

        // Capture list
        TSNode captureNode = childByFieldName(node, "captures");
        if (!ts_node_is_null(captureNode)) {
            uint32_t cc = ts_node_named_child_count(captureNode);
            for (uint32_t c = 0; c < cc; ++c) {
                TSNode cap = ts_node_named_child(captureNode, c);
                std::string capText = nodeText(cap, source);
                if (!capText.empty()) lambda->captureList.push_back(capText);
            }
        }

        // Parameters
        TSNode declNode = childByFieldName(node, "declarator");
        if (!ts_node_is_null(declNode)) {
            TSNode paramsNode = childByFieldName(declNode, "parameters");
            if (!ts_node_is_null(paramsNode)) {
                uint32_t pc = ts_node_named_child_count(paramsNode);
                for (uint32_t p = 0; p < pc; ++p) {
                    TSNode paramChild = ts_node_named_child(paramsNode, p);
                    if (nodeType(paramChild) == "parameter_declaration") {
                        TSNode pnameNode = childByFieldName(paramChild, "declarator");
                        if (!ts_node_is_null(pnameNode)) {
                            auto* param = new Parameter(IdGenerator::next("param"),
                                                        nodeText(pnameNode, source));
                            applySpan(param, paramChild);
                            lambda->addChild("parameters", param);
                        }
                    }
                }
            }
        }

        // Body
        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            uint32_t bc = ts_node_named_child_count(bodyNode);
            for (uint32_t b = 0; b < bc; ++b) {
                ASTNode* stmt = convertCppStatement(ts_node_named_child(bodyNode, b), source);
                if (stmt) lambda->addChild("body", stmt);
            }
        }
        return lambda;
    }

    static ASTNode* convertCppExpression(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "binary_expression") {
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
            applySpan(binOp, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            TSNode opNode = childByFieldName(node, "operator");
            if (!ts_node_is_null(opNode)) {
                binOp->op = nodeText(opNode, source);
            }
            if (!ts_node_is_null(leftNode)) {
                ASTNode* left = convertCppExpression(leftNode, source);
                if (left) binOp->setChild("left", left);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* right = convertCppExpression(rightNode, source);
                if (right) binOp->setChild("right", right);
            }
            return binOp;
        } else if (type == "identifier") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        } else if (type == "number_literal") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            applySpan(lit, node);
            return lit;
        } else if (type == "string_literal" || type == "raw_string_literal") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "parenthesized_expression") {
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) return convertCppExpression(ts_node_named_child(node, 0), source);
        } else if (type == "lambda_expression") {
            return convertCppLambdaExpression(node, source);
        }
        // Fallback
        std::string text = nodeText(node, source);
        if (!text.empty()) {
            auto* ref = new VariableReference(IdGenerator::next("var"), text);
            applySpan(ref, node);
            return ref;
        }
        return nullptr;
    }

    static void detectCppMemoryPatterns(const std::string& bodySource, Function* fn) {
        bool hasUnique = bodySource.find("unique_ptr") != std::string::npos ||
                         bodySource.find("make_unique") != std::string::npos;
        bool hasShared = bodySource.find("shared_ptr") != std::string::npos ||
                         bodySource.find("make_shared") != std::string::npos;
        bool hasNew = bodySource.find("new ") != std::string::npos;
        bool hasDelete = bodySource.find("delete ") != std::string::npos ||
                         bodySource.find("delete;") != std::string::npos;

        if (hasUnique) {
            auto* anno = new LifetimeAnnotation(IdGenerator::next("anno"), "RAII");
            fn->addChild("annotations", anno);
        }
        if (hasShared) {
            auto* anno = new OwnerAnnotation(IdGenerator::next("anno"), "Shared_ARC");
            fn->addChild("annotations", anno);
        }
        if (hasNew && hasDelete) {
            auto* anno = new DeallocateAnnotation(IdGenerator::next("anno"), "Explicit");
            fn->addChild("annotations", anno);
        }
    }

    // ---------------------------------------------------------------
