#pragma once
// TreeSitterParser Elisp support.
public:
    //  Elisp
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseElisp(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_elisp());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_elisp_module";
        module->targetLanguage = "elisp";
        applySpan(module.get(), root);

        convertElispSourceFile(root, source, module.get());

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseElispWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_elisp());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_elisp_module";
        result.module->targetLanguage = "elisp";
        applySpan(result.module.get(), root);

        convertElispSourceFile(root, source, result.module.get());
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
private:
    //  Elisp CST â†’ AST
    // ---------------------------------------------------------------
    static void convertElispSourceFile(TSNode root, const std::string& source, Module* module) {
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "function_definition" || type == "defun") {
                auto* fn = convertElispDefun(child, source);
                if (fn) module->addChild("functions", fn);
            } else if (type == "list") {
                auto* fn = tryConvertElispDefunFromList(child, source);
                if (fn) module->addChild("functions", fn);
            } else if (type == "special_form") {
                auto* fn = tryConvertElispDefunFromSpecialForm(child, source);
                if (fn) module->addChild("functions", fn);
            }
        }
    }

    static Function* convertElispDefun(TSNode node, const std::string& source) {
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);

        // tree-sitter-elisp function_definition has:
        //   field "name" â†’ symbol (function name)
        //   field "parameters" â†’ list (arglist)
        //   remaining named children â†’ body forms (no field name)

        TSNode nameNode = childByFieldName(node, "name");
        if (!ts_node_is_null(nameNode)) {
            fn->name = nodeText(nameNode, source);
        }

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertElispArglist(paramsNode, source, fn);
        }

        // Body: iterate all named children, skip name and parameters
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            // Skip the name symbol and parameters list
            if (!ts_node_is_null(nameNode) &&
                ts_node_start_byte(child) == ts_node_start_byte(nameNode) &&
                ts_node_end_byte(child) == ts_node_end_byte(nameNode))
                continue;
            if (!ts_node_is_null(paramsNode) &&
                ts_node_start_byte(child) == ts_node_start_byte(paramsNode) &&
                ts_node_end_byte(child) == ts_node_end_byte(paramsNode))
                continue;

            // This is a body form
            ASTNode* expr = convertElispExpression(child, source);
            if (expr) {
                auto* exprStmt = new ExpressionStatement();
                exprStmt->id = IdGenerator::next("exprstmt");
                applySpan(exprStmt, child);
                exprStmt->setChild("expression", expr);
                fn->addChild("body", exprStmt);
            }
        }

        // Fallback: if no name/params fields, try positional approach
        if (fn->name.empty()) {
            uint32_t nc = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < nc; ++i) {
                TSNode child = ts_node_named_child(node, i);
                std::string childType = nodeType(child);
                if (childType == "symbol" && fn->name.empty()) {
                    fn->name = nodeText(child, source);
                } else if (childType == "list" && fn->getChildren("parameters").empty()) {
                    convertElispArglist(child, source, fn);
                }
            }
        }

        // Auto-annotate: Elisp uses tracing GC
        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);

        return fn;
    }

    // Handle (defun ...) when parsed as a generic list node
    static Function* tryConvertElispDefunFromList(TSNode node, const std::string& source) {
        // Check if first child is "defun" symbol
        uint32_t count = ts_node_named_child_count(node);
        if (count < 3) return nullptr;

        TSNode firstChild = ts_node_named_child(node, 0);
        std::string firstText = nodeText(firstChild, source);
        if (firstText != "defun") return nullptr;

        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");

        // Second child is the name
        TSNode nameChild = ts_node_named_child(node, 1);
        fn->name = nodeText(nameChild, source);

        // Third child is the arglist (a list)
        TSNode arglistChild = ts_node_named_child(node, 2);
        if (nodeType(arglistChild) == "list") {
            convertElispArglist(arglistChild, source, fn);
        }

        // Remaining children are body forms
        for (uint32_t i = 3; i < count; ++i) {
            TSNode bodyChild = ts_node_named_child(node, i);
            ASTNode* expr = convertElispExpression(bodyChild, source);
            if (expr) {
                if (i == count - 1) {
                    // Last form â€” wrap in ExpressionStatement (implicit return)
                    auto* exprStmt = new ExpressionStatement();
                    exprStmt->id = IdGenerator::next("exprstmt");
                    applySpan(exprStmt, bodyChild);
                    exprStmt->setChild("expression", expr);
                    fn->addChild("body", exprStmt);
                } else {
                    auto* exprStmt = new ExpressionStatement();
                    exprStmt->id = IdGenerator::next("exprstmt");
                    applySpan(exprStmt, bodyChild);
                    exprStmt->setChild("expression", expr);
                    fn->addChild("body", exprStmt);
                }
            }
        }

        // Auto-annotate
        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);

        return fn;
    }

    static Function* tryConvertElispDefunFromSpecialForm(TSNode node, const std::string& source) {
        // special_form might contain defun
        uint32_t count = ts_node_named_child_count(node);
        if (count < 3) return nullptr;

        TSNode firstChild = ts_node_named_child(node, 0);
        std::string firstText = nodeText(firstChild, source);
        if (firstText != "defun") return nullptr;

        // Same logic as tryConvertElispDefunFromList
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");

        TSNode nameChild = ts_node_named_child(node, 1);
        fn->name = nodeText(nameChild, source);

        TSNode arglistChild = ts_node_named_child(node, 2);
        std::string argType = nodeType(arglistChild);
        if (argType == "list") {
            convertElispArglist(arglistChild, source, fn);
        }

        for (uint32_t i = 3; i < count; ++i) {
            TSNode bodyChild = ts_node_named_child(node, i);
            ASTNode* expr = convertElispExpression(bodyChild, source);
            if (expr) {
                auto* exprStmt = new ExpressionStatement();
                exprStmt->id = IdGenerator::next("exprstmt");
                applySpan(exprStmt, bodyChild);
                exprStmt->setChild("expression", expr);
                fn->addChild("body", exprStmt);
            }
        }

        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);

        return fn;
    }

    static void convertElispArglist(TSNode node, const std::string& source, Function* fn) {
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            std::string type = nodeType(child);
            if (type == "symbol" || type == "identifier") {
                auto* param = new Parameter(IdGenerator::next("param"), nodeText(child, source));
                applySpan(param, child);
                fn->addChild("parameters", param);
            }
        }
    }

    static void convertElispBodyField(TSNode bodyNode, const std::string& source, Function* fn) {
        // body might be a single node or we need to iterate children
        uint32_t count = ts_node_named_child_count(bodyNode);
        if (count > 0) {
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(bodyNode, i);
                ASTNode* expr = convertElispExpression(child, source);
                if (expr) {
                    auto* exprStmt = new ExpressionStatement();
                    exprStmt->id = IdGenerator::next("exprstmt");
                    applySpan(exprStmt, child);
                    exprStmt->setChild("expression", expr);
                    fn->addChild("body", exprStmt);
                }
            }
        } else {
            ASTNode* expr = convertElispExpression(bodyNode, source);
            if (expr) {
                auto* exprStmt = new ExpressionStatement();
                exprStmt->id = IdGenerator::next("exprstmt");
                applySpan(exprStmt, bodyNode);
                exprStmt->setChild("expression", expr);
                fn->addChild("body", exprStmt);
            }
        }
    }

    static void convertElispBodyFromChildren(TSNode node, const std::string& source, Function* fn) {
        // Skip: first named child should be name, second should be arglist, rest is body
        uint32_t count = ts_node_named_child_count(node);
        int bodyStart = -1;
        int arglistSeen = 0;
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            std::string type = nodeType(child);
            if (type == "symbol" && fn->name.empty()) {
                fn->name = nodeText(child, source);
                continue;
            }
            if (type == "list" && fn->getChildren("parameters").empty()) {
                convertElispArglist(child, source, fn);
                arglistSeen = 1;
                continue;
            }
            if (arglistSeen || (int)i >= 2) {
                // Body form
                ASTNode* expr = convertElispExpression(child, source);
                if (expr) {
                    auto* exprStmt = new ExpressionStatement();
                    exprStmt->id = IdGenerator::next("exprstmt");
                    applySpan(exprStmt, child);
                    exprStmt->setChild("expression", expr);
                    fn->addChild("body", exprStmt);
                }
            }
        }
    }

    static ASTNode* convertElispExpression(TSNode node, const std::string& source) {
        std::string type = nodeType(node);

        if (type == "list") {
            // Check if operator-form: (+ x 1), (- x 1), (* x y), (/ x y)
            uint32_t count = ts_node_named_child_count(node);
            if (count >= 3) {
                TSNode firstChild = ts_node_named_child(node, 0);
                std::string firstText = nodeText(firstChild, source);
                if (firstText == "+" || firstText == "-" || firstText == "*" || firstText == "/") {
                    auto* binOp = new BinaryOperation();
                    binOp->id = IdGenerator::next("binop");
                    binOp->op = firstText;
                    applySpan(binOp, node);
                    ASTNode* left = convertElispExpression(ts_node_named_child(node, 1), source);
                    ASTNode* right = convertElispExpression(ts_node_named_child(node, 2), source);
                    if (left) binOp->setChild("left", left);
                    if (right) binOp->setChild("right", right);
                    return binOp;
                }
            }
            // Generic list â€” could be a function call
            if (count >= 1) {
                auto* call = new FunctionCall();
                call->id = IdGenerator::next("call");
                applySpan(call, node);
                TSNode funcName = ts_node_named_child(node, 0);
                call->functionName = nodeText(funcName, source);
                for (uint32_t i = 1; i < count; ++i) {
                    ASTNode* arg = convertElispExpression(ts_node_named_child(node, i), source);
                    if (arg) call->addChild("arguments", arg);
                }
                return call;
            }
        } else if (type == "symbol" || type == "identifier") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        } else if (type == "integer" || type == "number") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            applySpan(lit, node);
            return lit;
        } else if (type == "string") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "special_form") {
            // Could be (if ...), (let ...), etc.
            return convertElispSpecialForm(node, source);
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

    static ASTNode* convertElispSpecialForm(TSNode node, const std::string& source) {
        uint32_t count = ts_node_named_child_count(node);
        if (count < 1) return nullptr;

        TSNode firstChild = ts_node_named_child(node, 0);
        std::string formName = nodeText(firstChild, source);

        if (formName == "if" && count >= 3) {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            ASTNode* cond = convertElispExpression(ts_node_named_child(node, 1), source);
            if (cond) ifStmt->setChild("condition", cond);
            return ifStmt;
        }

        // Generic: treat as function call
        auto* call = new FunctionCall();
        call->id = IdGenerator::next("call");
        applySpan(call, node);
        call->functionName = formName;
        for (uint32_t i = 1; i < count; ++i) {
            ASTNode* arg = convertElispExpression(ts_node_named_child(node, i), source);
            if (arg) call->addChild("arguments", arg);
        }
        return call;
    }

    // ---------------------------------------------------------------
