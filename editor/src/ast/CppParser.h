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
            return exprStmt;
        } else if (type == "if_statement") {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            return ifStmt;
        }
        return nullptr;
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
