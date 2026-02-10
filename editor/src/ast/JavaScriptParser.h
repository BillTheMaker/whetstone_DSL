#pragma once
// TreeSitterParser JavaScript support.
public:
    //  JavaScript
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseJavaScript(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_javascript());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_js_module";
        module->targetLanguage = "javascript";
        applySpan(module.get(), root);

        convertJavaScriptModule(root, source, module.get(), "javascript");

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseJavaScriptWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_javascript());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_js_module";
        result.module->targetLanguage = "javascript";
        applySpan(result.module.get(), root);

        convertJavaScriptModule(root, source, result.module.get(), "javascript");
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
private:
    //  JavaScript / TypeScript CST -> AST
    // ---------------------------------------------------------------
    static void convertJavaScriptModule(TSNode root,
                                        const std::string& source,
                                        Module* module,
                                        const std::string& language) {
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "function_declaration") {
                auto* fn = convertJavaScriptFunction(child, source, language);
                if (fn) module->addChild("functions", fn);
            } else if (type == "class_declaration") {
                convertJavaScriptClass(child, source, module, language);
            } else if (type == "lexical_declaration" || type == "variable_declaration") {
                convertJavaScriptVariableFunctions(child, source, module, language);
            } else if (type == "export_statement") {
                TSNode decl = ts_node_named_child(child, 0);
                std::string declType = nodeType(decl);
                if (declType == "function_declaration") {
                    auto* fn = convertJavaScriptFunction(decl, source, language);
                    if (fn) module->addChild("functions", fn);
                } else if (declType == "class_declaration") {
                    convertJavaScriptClass(decl, source, module, language);
                } else if (declType == "lexical_declaration" || declType == "variable_declaration") {
                    convertJavaScriptVariableFunctions(decl, source, module, language);
                }
            }
        }
    }

    static void convertJavaScriptClass(TSNode node,
                                       const std::string& source,
                                       Module* module,
                                       const std::string& language) {
        TSNode nameNode = childByFieldName(node, "name");
        std::string className = nodeText(nameNode, source);
        TSNode bodyNode = childByFieldName(node, "body");
        if (ts_node_is_null(bodyNode)) return;
        uint32_t count = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(bodyNode, i);
            std::string type = nodeType(child);
            if (type == "method_definition") {
                auto* fn = convertJavaScriptMethod(child, source, language, className);
                if (fn) module->addChild("functions", fn);
            }
        }
    }

    static void convertJavaScriptVariableFunctions(TSNode node,
                                                   const std::string& source,
                                                   Module* module,
                                                   const std::string& language) {
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            if (nodeType(child) != "variable_declarator") continue;
            TSNode nameNode = childByFieldName(child, "name");
            TSNode valueNode = childByFieldName(child, "value");
            if (ts_node_is_null(nameNode) || ts_node_is_null(valueNode)) continue;
            std::string valueType = nodeType(valueNode);
            if (valueType == "arrow_function" || valueType == "function" ||
                valueType == "function_expression") {
                auto* fn = convertJavaScriptFunctionExpression(valueNode, source,
                                                               language, nodeText(nameNode, source));
                if (fn) module->addChild("functions", fn);
            }
        }
    }

    static Function* convertJavaScriptFunction(TSNode node,
                                               const std::string& source,
                                               const std::string& language) {
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        fn->name = nodeText(nameNode, source);

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertJavaScriptParameters(paramsNode, source, fn, language);
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertJavaScriptBody(bodyNode, source, fn, language);
        }

        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);
        return fn;
    }

    static Function* convertJavaScriptMethod(TSNode node,
                                             const std::string& source,
                                             const std::string& language,
                                             const std::string& className) {
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        std::string name = nodeText(nameNode, source);
        fn->name = className.empty() ? name : (className + "." + name);

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertJavaScriptParameters(paramsNode, source, fn, language);
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertJavaScriptBody(bodyNode, source, fn, language);
        }

        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);
        return fn;
    }

    static Function* convertJavaScriptFunctionExpression(TSNode node,
                                                         const std::string& source,
                                                         const std::string& language,
                                                         const std::string& nameOverride) {
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        fn->name = nameOverride;

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertJavaScriptParameters(paramsNode, source, fn, language);
        } else if (nodeType(node) == "arrow_function") {
            TSNode paramNode = childByFieldName(node, "parameter");
            if (!ts_node_is_null(paramNode)) {
                auto* param = new Parameter(IdGenerator::next("param"), nodeText(paramNode, source));
                applySpan(param, paramNode);
                fn->addChild("parameters", param);
            }
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertJavaScriptBody(bodyNode, source, fn, language);
        }

        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);
        return fn;
    }

    static void convertJavaScriptParameters(TSNode paramsNode,
                                            const std::string& source,
                                            Function* fn,
                                            const std::string& language) {
        uint32_t count = ts_node_named_child_count(paramsNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(paramsNode, i);
            std::string type = nodeType(child);
            if (type == "identifier" || type == "pattern" || type == "rest_pattern") {
                auto* param = new Parameter(IdGenerator::next("param"), nodeText(child, source));
                applySpan(param, child);
                fn->addChild("parameters", param);
            } else if (type == "required_parameter" || type == "optional_parameter" ||
                       type == "formal_parameter") {
                TSNode nameNode = childByFieldName(child, "pattern");
                if (ts_node_is_null(nameNode)) nameNode = childByFieldName(child, "name");
                if (!ts_node_is_null(nameNode)) {
                    auto* param = new Parameter(IdGenerator::next("param"), nodeText(nameNode, source));
                    applySpan(param, child);
                    TSNode typeNode = childByFieldName(child, "type");
                    if (ts_node_is_null(typeNode)) typeNode = childByFieldName(child, "type_annotation");
                    if (!ts_node_is_null(typeNode) && language == "typescript") {
                        auto* typeAnno = new CustomType();
                        typeAnno->id = IdGenerator::next("type");
                        typeAnno->typeName = nodeText(typeNode, source);
                        param->setChild("type", typeAnno);
                    }
                    fn->addChild("parameters", param);
                }
            } else if (type == "assignment_pattern") {
                TSNode left = childByFieldName(child, "left");
                TSNode right = childByFieldName(child, "right");
                if (!ts_node_is_null(left)) {
                    auto* param = new Parameter(IdGenerator::next("param"), nodeText(left, source));
                    applySpan(param, child);
                    if (!ts_node_is_null(right)) {
                        ASTNode* defVal = convertJavaScriptExpression(right, source, language);
                        if (defVal) param->setChild("defaultValue", defVal);
                    }
                    fn->addChild("parameters", param);
                }
            }
        }
    }

    static void convertJavaScriptBody(TSNode bodyNode,
                                      const std::string& source,
                                      Function* fn,
                                      const std::string& language) {
        std::string type = nodeType(bodyNode);
        if (type == "statement_block" || type == "statement_block") {
            uint32_t count = ts_node_named_child_count(bodyNode);
            for (uint32_t i = 0; i < count; ++i) {
                ASTNode* stmt = convertJavaScriptStatement(ts_node_named_child(bodyNode, i), source, language);
                if (stmt) fn->addChild("body", stmt);
            }
        } else {
            ASTNode* expr = convertJavaScriptExpression(bodyNode, source, language);
            if (expr) {
                auto* exprStmt = new ExpressionStatement();
                exprStmt->id = IdGenerator::next("exprstmt");
                applySpan(exprStmt, bodyNode);
                exprStmt->setChild("expression", expr);
                fn->addChild("body", exprStmt);
            }
        }
    }

    static ASTNode* convertJavaScriptStatement(TSNode node,
                                               const std::string& source,
                                               const std::string& language) {
        std::string type = nodeType(node);
        if (type == "return_statement") {
            auto* ret = new Return();
            ret->id = IdGenerator::next("ret");
            applySpan(ret, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* val = convertJavaScriptExpression(ts_node_named_child(node, 0), source, language);
                if (val) ret->setChild("value", val);
            }
            return ret;
        } else if (type == "expression_statement") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* expr = convertJavaScriptExpression(ts_node_named_child(node, 0), source, language);
                if (expr) exprStmt->setChild("expression", expr);
            }
            return exprStmt;
        } else if (type == "lexical_declaration" || type == "variable_declaration") {
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(node, i);
                if (nodeType(child) != "variable_declarator") continue;
                TSNode nameNode = childByFieldName(child, "name");
                TSNode valueNode = childByFieldName(child, "value");
                if (ts_node_is_null(nameNode)) continue;
                auto* assign = new Assignment();
                assign->id = IdGenerator::next("assign");
                applySpan(assign, child);
                auto* target = new VariableReference(IdGenerator::next("var"), nodeText(nameNode, source));
                applySpan(target, nameNode);
                assign->setChild("target", target);
                if (!ts_node_is_null(valueNode)) {
                    ASTNode* val = convertJavaScriptExpression(valueNode, source, language);
                    if (val) assign->setChild("value", val);
                }
                return assign;
            }
        } else if (type == "if_statement") {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertJavaScriptExpression(condNode, source, language);
                if (cond) ifStmt->setChild("condition", cond);
            }
            TSNode consNode = childByFieldName(node, "consequence");
            if (!ts_node_is_null(consNode)) {
                uint32_t cc = ts_node_named_child_count(consNode);
                for (uint32_t i = 0; i < cc; ++i) {
                    ASTNode* s = convertJavaScriptStatement(ts_node_named_child(consNode, i), source, language);
                    if (s) ifStmt->addChild("thenBranch", s);
                }
            }
            TSNode altNode = childByFieldName(node, "alternative");
            if (!ts_node_is_null(altNode)) {
                uint32_t ac = ts_node_named_child_count(altNode);
                for (uint32_t i = 0; i < ac; ++i) {
                    ASTNode* s = convertJavaScriptStatement(ts_node_named_child(altNode, i), source, language);
                    if (s) ifStmt->addChild("elseBranch", s);
                }
            }
            return ifStmt;
        }
        ASTNode* expr = convertJavaScriptExpression(node, source, language);
        if (expr) {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            exprStmt->setChild("expression", expr);
            return exprStmt;
        }
        return nullptr;
    }

    static ASTNode* convertJavaScriptExpression(TSNode node,
                                                const std::string& source,
                                                const std::string& language) {
        std::string type = nodeType(node);
        if (type == "binary_expression" || type == "logical_expression") {
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
            applySpan(binOp, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            TSNode opNode = childByFieldName(node, "operator");
            if (!ts_node_is_null(opNode)) binOp->op = nodeText(opNode, source);
            if (!ts_node_is_null(leftNode)) {
                ASTNode* left = convertJavaScriptExpression(leftNode, source, language);
                if (left) binOp->setChild("left", left);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* right = convertJavaScriptExpression(rightNode, source, language);
                if (right) binOp->setChild("right", right);
            }
            return binOp;
        } else if (type == "assignment_expression") {
            auto* assign = new Assignment();
            assign->id = IdGenerator::next("assign");
            applySpan(assign, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            if (!ts_node_is_null(leftNode)) {
                ASTNode* target = convertJavaScriptExpression(leftNode, source, language);
                if (target) assign->setChild("target", target);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* value = convertJavaScriptExpression(rightNode, source, language);
                if (value) assign->setChild("value", value);
            }
            return assign;
        } else if (type == "call_expression") {
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
                    ASTNode* arg = convertJavaScriptExpression(ts_node_named_child(argsNode, i), source, language);
                    if (arg) call->addChild("arguments", arg);
                }
            }
            return call;
        } else if (type == "member_expression") {
            auto* mem = new MemberAccess();
            mem->id = IdGenerator::next("member");
            applySpan(mem, node);
            TSNode objNode = childByFieldName(node, "object");
            TSNode propNode = childByFieldName(node, "property");
            if (!ts_node_is_null(propNode)) {
                mem->memberName = nodeText(propNode, source);
            }
            if (!ts_node_is_null(objNode)) {
                ASTNode* target = convertJavaScriptExpression(objNode, source, language);
                if (target) mem->setChild("target", target);
            }
            return mem;
        } else if (type == "subscript_expression") {
            auto* access = new IndexAccess();
            access->id = IdGenerator::next("index");
            applySpan(access, node);
            TSNode objNode = childByFieldName(node, "object");
            TSNode idxNode = childByFieldName(node, "index");
            if (!ts_node_is_null(objNode)) {
                ASTNode* target = convertJavaScriptExpression(objNode, source, language);
                if (target) access->setChild("target", target);
            }
            if (!ts_node_is_null(idxNode)) {
                ASTNode* idx = convertJavaScriptExpression(idxNode, source, language);
                if (idx) access->setChild("index", idx);
            }
            return access;
        } else if (type == "identifier") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        } else if (type == "number") {
            std::string text = nodeText(node, source);
            if (text.find('.') != std::string::npos) {
                auto* lit = new FloatLiteral(IdGenerator::next("float"), text);
                applySpan(lit, node);
                return lit;
            }
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            applySpan(lit, node);
            return lit;
        } else if (type == "string" || type == "string_fragment" || type == "template_string") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "true" || type == "false") {
            auto* lit = new BooleanLiteral(IdGenerator::next("bool"), type == "true");
            applySpan(lit, node);
            return lit;
        } else if (type == "null") {
            auto* lit = new NullLiteral();
            lit->id = IdGenerator::next("null");
            applySpan(lit, node);
            return lit;
        } else if (type == "parenthesized_expression") {
            if (ts_node_named_child_count(node) > 0) {
                return convertJavaScriptExpression(ts_node_named_child(node, 0), source, language);
            }
        }
        std::string text = nodeText(node, source);
        if (!text.empty()) {
            auto* ref = new VariableReference(IdGenerator::next("var"), text);
            applySpan(ref, node);
            return ref;
        }
        return nullptr;
    }

    // ---------------------------------------------------------------
