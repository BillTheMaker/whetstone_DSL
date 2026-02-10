#pragma once
// TreeSitterParser Python support.
public:
    //  Python
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parsePython(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_python());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_python_module";
        module->targetLanguage = "python";
        applySpan(module.get(), root);

        convertPythonModule(root, source, module.get());

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parsePythonWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_python());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_python_module";
        result.module->targetLanguage = "python";
        applySpan(result.module.get(), root);

        convertPythonModule(root, source, result.module.get());
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
private:
    //  Python CST â†’ AST
    // ---------------------------------------------------------------
    static void convertPythonModule(TSNode root, const std::string& source, Module* module) {
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "function_definition") {
                auto* fn = convertPythonFunction(child, source);
                if (fn) module->addChild("functions", fn);
            }
        }
    }

    static Function* convertPythonFunction(TSNode node, const std::string& source) {
        // Get function name
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;

        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        applySpan(fn, node);
        fn->name = nodeText(nameNode, source);
        applySpan(fn, node);

        // Parameters
        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertPythonParameters(paramsNode, source, fn);
        }

        // Body
        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertPythonBody(bodyNode, source, fn);
        }

        // Auto-annotate: Python uses tracing GC
        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);

        return fn;
    }

    static void convertPythonParameters(TSNode paramsNode, const std::string& source, Function* fn) {
        uint32_t count = ts_node_named_child_count(paramsNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(paramsNode, i);
            std::string type = nodeType(child);
            if (type == "identifier") {
                auto* param = new Parameter(IdGenerator::next("param"), nodeText(child, source));
                applySpan(param, child);
                fn->addChild("parameters", param);
            } else if (type == "default_parameter") {
                // def f(x=10) â†’ Parameter with defaultValue
                TSNode nameN = childByFieldName(child, "name");
                TSNode valueN = childByFieldName(child, "value");
                if (!ts_node_is_null(nameN)) {
                    auto* param = new Parameter(IdGenerator::next("param"), nodeText(nameN, source));
                    applySpan(param, child);
                    if (!ts_node_is_null(valueN)) {
                        ASTNode* defVal = convertPythonExpression(valueN, source);
                        if (defVal) param->setChild("defaultValue", defVal);
                    }
                    fn->addChild("parameters", param);
                }
            }
        }
    }

    static void convertPythonBody(TSNode bodyNode, const std::string& source, Function* fn) {
        // bodyNode is typically a "block" node
        uint32_t count = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(bodyNode, i);
            ASTNode* stmt = convertPythonStatement(child, source);
            if (stmt) fn->addChild("body", stmt);
        }
    }

    static ASTNode* convertPythonStatement(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "return_statement") {
            auto* ret = new Return();
            ret->id = IdGenerator::next("ret");
            applySpan(ret, node);
            // The return value is the first named child (if any)
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) {
                TSNode valNode = ts_node_named_child(node, 0);
                ASTNode* val = convertPythonExpression(valNode, source);
                if (val) ret->setChild("value", val);
            }
            return ret;
        } else if (type == "if_statement") {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertPythonExpression(condNode, source);
                if (cond) ifStmt->setChild("condition", cond);
            }
            TSNode conseq = childByFieldName(node, "consequence");
            if (!ts_node_is_null(conseq)) {
                uint32_t cc = ts_node_named_child_count(conseq);
                for (uint32_t i = 0; i < cc; ++i) {
                    ASTNode* s = convertPythonStatement(ts_node_named_child(conseq, i), source);
                    if (s) ifStmt->addChild("thenBranch", s);
                }
            }
            return ifStmt;
        } else if (type == "for_statement") {
            auto* forLoop = new ForLoop();
            forLoop->id = IdGenerator::next("for");
            applySpan(forLoop, node);
            TSNode leftNode = childByFieldName(node, "left");
            if (!ts_node_is_null(leftNode)) {
                forLoop->iteratorName = nodeText(leftNode, source);
            }
            TSNode rightNode = childByFieldName(node, "right");
            if (!ts_node_is_null(rightNode)) {
                ASTNode* iter = convertPythonExpression(rightNode, source);
                if (iter) forLoop->setChild("iterable", iter);
            }
            TSNode body = childByFieldName(node, "body");
            if (!ts_node_is_null(body)) {
                uint32_t cc = ts_node_named_child_count(body);
                for (uint32_t i = 0; i < cc; ++i) {
                    ASTNode* s = convertPythonStatement(ts_node_named_child(body, i), source);
                    if (s) forLoop->addChild("body", s);
                }
            }
            return forLoop;
        } else if (type == "expression_statement") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) {
                ASTNode* expr = convertPythonExpression(ts_node_named_child(node, 0), source);
                if (expr) exprStmt->setChild("expression", expr);
            }
            return exprStmt;
        }
        // Fallback: wrap as expression statement
        ASTNode* expr = convertPythonExpression(node, source);
        if (expr) {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            exprStmt->setChild("expression", expr);
            return exprStmt;
        }
        return nullptr;
    }

    static ASTNode* convertPythonExpression(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "binary_operator") {
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
            applySpan(binOp, node);
            TSNode opNode = childByFieldName(node, "operator");
            if (!ts_node_is_null(opNode)) {
                binOp->op = nodeText(opNode, source);
            }
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            if (!ts_node_is_null(leftNode)) {
                ASTNode* left = convertPythonExpression(leftNode, source);
                if (left) binOp->setChild("left", left);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* right = convertPythonExpression(rightNode, source);
                if (right) binOp->setChild("right", right);
            }
            return binOp;
        } else if (type == "identifier") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        } else if (type == "integer") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            applySpan(lit, node);
            return lit;
        } else if (type == "string" || type == "concatenated_string") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "comparison_operator" || type == "boolean_operator") {
            // Treat like binary op
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
            applySpan(binOp, node);
            uint32_t count = ts_node_named_child_count(node);
            if (count >= 2) {
                ASTNode* left = convertPythonExpression(ts_node_named_child(node, 0), source);
                if (left) binOp->setChild("left", left);
                ASTNode* right = convertPythonExpression(ts_node_named_child(node, count - 1), source);
                if (right) binOp->setChild("right", right);
            }
            // Operator is a non-named child between the named ones
            uint32_t totalCount = ts_node_child_count(node);
            for (uint32_t i = 0; i < totalCount; ++i) {
                TSNode c = ts_node_child(node, i);
                if (!ts_node_is_named(c)) {
                    std::string opText = nodeText(c, source);
                    if (!opText.empty() && opText != "(" && opText != ")") {
                        binOp->op = opText;
                        break;
                    }
                }
            }
            return binOp;
        } else if (type == "call") {
            auto* call = new FunctionCall();
            call->id = IdGenerator::next("call");
            applySpan(call, node);
            TSNode funcNode = childByFieldName(node, "function");
            if (!ts_node_is_null(funcNode)) {
                call->functionName = nodeText(funcNode, source);
            }
            TSNode argsNode = childByFieldName(node, "arguments");
            if (!ts_node_is_null(argsNode)) {
                uint32_t count = ts_node_named_child_count(argsNode);
                for (uint32_t i = 0; i < count; ++i) {
                    ASTNode* arg = convertPythonExpression(ts_node_named_child(argsNode, i), source);
                    if (arg) call->addChild("arguments", arg);
                }
            }
            return call;
        } else if (type == "parenthesized_expression") {
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) return convertPythonExpression(ts_node_named_child(node, 0), source);
        } else if (type == "unary_operator") {
            auto* unOp = new UnaryOperation();
            unOp->id = IdGenerator::next("unop");
            applySpan(unOp, node);
            TSNode opNode = childByFieldName(node, "operator");
            if (!ts_node_is_null(opNode)) {
                unOp->op = nodeText(opNode, source);
            }
            TSNode operandNode = childByFieldName(node, "operand");
            if (!ts_node_is_null(operandNode)) {
                ASTNode* operand = convertPythonExpression(operandNode, source);
                if (operand) unOp->setChild("operand", operand);
            }
            return unOp;
        }
        // Fallback: treat as variable reference with raw text
        std::string text = nodeText(node, source);
        if (!text.empty()) {
            auto* ref = new VariableReference(IdGenerator::next("var"), text);
            applySpan(ref, node);
            return ref;
        }
        return nullptr;
    }

    // ---------------------------------------------------------------
