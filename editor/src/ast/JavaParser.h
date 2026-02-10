#pragma once
// TreeSitterParser Java support.
public:
    //  Java
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseJava(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_java());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_java_module";
        module->targetLanguage = "java";
        applySpan(module.get(), root);

        convertJavaCompilationUnit(root, source, module.get());

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseJavaWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_java());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_java_module";
        result.module->targetLanguage = "java";
        applySpan(result.module.get(), root);

        convertJavaCompilationUnit(root, source, result.module.get());
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
private:
    //  Java CST -> AST
    // ---------------------------------------------------------------
    static void convertJavaCompilationUnit(TSNode root,
                                           const std::string& source,
                                           Module* module) {
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "import_declaration") {
                std::string importName;
                uint32_t ic = ts_node_named_child_count(child);
                for (uint32_t j = 0; j < ic; ++j) {
                    TSNode part = ts_node_named_child(child, j);
                    std::string pType = nodeType(part);
                    if (pType == "scoped_identifier" || pType == "identifier") {
                        importName = nodeText(part, source);
                        break;
                    }
                }
                if (importName.empty()) {
                    importName = nodeText(child, source);
                }
                if (!importName.empty()) {
                    auto* imp = new Import(IdGenerator::next("imp"), importName, "module");
                    module->addChild("imports", imp);
                }
            } else if (type == "class_declaration" || type == "interface_declaration" ||
                       type == "enum_declaration" || type == "record_declaration" ||
                       type == "annotation_type_declaration") {
                convertJavaTypeDeclaration(child, source, module);
            }
        }
    }

    static void convertJavaTypeDeclaration(TSNode node,
                                           const std::string& source,
                                           Module* module) {
        TSNode nameNode = childByFieldName(node, "name");
        std::string className = nodeText(nameNode, source);
        TSNode bodyNode = childByFieldName(node, "body");
        if (ts_node_is_null(bodyNode)) return;

        uint32_t count = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(bodyNode, i);
            std::string type = nodeType(child);
            if (type == "method_declaration") {
                auto* fn = convertJavaMethod(child, source, className);
                if (fn) module->addChild("functions", fn);
            } else if (type == "constructor_declaration" || type == "compact_constructor_declaration") {
                auto* fn = convertJavaConstructor(child, source, className);
                if (fn) module->addChild("functions", fn);
            } else if (type == "field_declaration") {
                convertJavaFieldDeclaration(child, source, module, className);
            } else if (type == "class_declaration" || type == "interface_declaration" ||
                       type == "enum_declaration" || type == "record_declaration" ||
                       type == "annotation_type_declaration") {
                convertJavaTypeDeclaration(child, source, module);
            }
        }
    }

    static void convertJavaFieldDeclaration(TSNode node,
                                            const std::string& source,
                                            Module* module,
                                            const std::string& className) {
        TSNode typeNode = childByFieldName(node, "type");
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            if (nodeType(child) != "variable_declarator") continue;
            TSNode nameNode = childByFieldName(child, "name");
            if (ts_node_is_null(nameNode)) continue;
            std::string varName = nodeText(nameNode, source);
            if (!className.empty()) varName = className + "." + varName;
            auto* var = new Variable(IdGenerator::next("var"), varName);
            applySpan(var, child);
            if (!ts_node_is_null(typeNode)) {
                if (auto* t = convertJavaType(typeNode, source)) var->setChild("type", t);
            }
            TSNode valueNode = childByFieldName(child, "value");
            if (!ts_node_is_null(valueNode)) {
                ASTNode* init = convertJavaExpression(valueNode, source);
                if (init) var->setChild("initializer", init);
            }
            module->addChild("variables", var);
        }
    }

    static Function* convertJavaMethod(TSNode node,
                                       const std::string& source,
                                       const std::string& className) {
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        std::string methodName = nodeText(nameNode, source);
        fn->name = className.empty() ? methodName : (className + "." + methodName);

        TSNode typeNode = childByFieldName(node, "type");
        if (!ts_node_is_null(typeNode)) {
            if (auto* t = convertJavaType(typeNode, source)) fn->setChild("returnType", t);
        }

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertJavaParameters(paramsNode, source, fn);
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertJavaBlockStatements(bodyNode, source, fn);
        }

        attachJavaAnnotations(node, source, fn);
        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);
        return fn;
    }

    static Function* convertJavaConstructor(TSNode node,
                                            const std::string& source,
                                            const std::string& className) {
        if (className.empty()) return nullptr;
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        fn->name = className + ".constructor";

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertJavaParameters(paramsNode, source, fn);
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertJavaBlockStatements(bodyNode, source, fn);
        }

        attachJavaAnnotations(node, source, fn);
        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);
        return fn;
    }

    static void convertJavaParameters(TSNode paramsNode,
                                      const std::string& source,
                                      Function* fn) {
        uint32_t count = ts_node_named_child_count(paramsNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(paramsNode, i);
            std::string type = nodeType(child);
            if (type == "formal_parameter" || type == "spread_parameter") {
                TSNode nameNode = childByFieldName(child, "name");
                if (ts_node_is_null(nameNode)) {
                    nameNode = findDescendantByType(child, "_variable_declarator_id");
                    if (!ts_node_is_null(nameNode)) {
                        TSNode innerName = childByFieldName(nameNode, "name");
                        if (!ts_node_is_null(innerName)) nameNode = innerName;
                    }
                }
                if (ts_node_is_null(nameNode)) continue;
                std::string name = nodeText(nameNode, source);
                if (type == "spread_parameter") name = "..." + name;
                auto* param = new Parameter(IdGenerator::next("param"), name);
                applySpan(param, child);
                TSNode typeNode = childByFieldName(child, "type");
                if (!ts_node_is_null(typeNode)) {
                    if (auto* t = convertJavaType(typeNode, source)) param->setChild("type", t);
                }
                fn->addChild("parameters", param);
            }
        }
    }

    static void convertJavaBlockStatements(TSNode bodyNode,
                                           const std::string& source,
                                           Function* fn) {
        uint32_t count = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < count; ++i) {
            ASTNode* stmt = convertJavaStatement(ts_node_named_child(bodyNode, i), source);
            if (stmt) fn->addChild("body", stmt);
        }
    }

    static ASTNode* convertJavaStatement(TSNode node,
                                         const std::string& source) {
        std::string type = nodeType(node);
        if (type == "return_statement") {
            auto* ret = new Return();
            ret->id = IdGenerator::next("ret");
            applySpan(ret, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* val = convertJavaExpression(ts_node_named_child(node, 0), source);
                if (val) ret->setChild("value", val);
            }
            return ret;
        } else if (type == "expression_statement") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* expr = convertJavaExpression(ts_node_named_child(node, 0), source);
                if (expr) exprStmt->setChild("expression", expr);
            }
            return exprStmt;
        } else if (type == "local_variable_declaration") {
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(node, i);
                if (nodeType(child) != "variable_declarator") continue;
                TSNode nameNode = childByFieldName(child, "name");
                if (ts_node_is_null(nameNode)) continue;
                auto* assign = new Assignment();
                assign->id = IdGenerator::next("assign");
                applySpan(assign, child);
                auto* target = new VariableReference(IdGenerator::next("var"), nodeText(nameNode, source));
                applySpan(target, nameNode);
                assign->setChild("target", target);
                TSNode valueNode = childByFieldName(child, "value");
                if (!ts_node_is_null(valueNode)) {
                    ASTNode* val = convertJavaExpression(valueNode, source);
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
                ASTNode* cond = convertJavaExpression(condNode, source);
                if (cond) ifStmt->setChild("condition", cond);
            }
            TSNode consNode = childByFieldName(node, "consequence");
            if (!ts_node_is_null(consNode)) {
                uint32_t cc = ts_node_named_child_count(consNode);
                for (uint32_t i = 0; i < cc; ++i) {
                    ASTNode* s = convertJavaStatement(ts_node_named_child(consNode, i), source);
                    if (s) ifStmt->addChild("thenBranch", s);
                }
            }
            TSNode altNode = childByFieldName(node, "alternative");
            if (!ts_node_is_null(altNode)) {
                uint32_t ac = ts_node_named_child_count(altNode);
                for (uint32_t i = 0; i < ac; ++i) {
                    ASTNode* s = convertJavaStatement(ts_node_named_child(altNode, i), source);
                    if (s) ifStmt->addChild("elseBranch", s);
                }
            }
            return ifStmt;
        } else if (type == "while_statement") {
            auto* loop = new WhileLoop();
            loop->id = IdGenerator::next("while");
            applySpan(loop, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertJavaExpression(condNode, source);
                if (cond) loop->setChild("condition", cond);
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertJavaStatement(ts_node_named_child(bodyNode, i), source);
                    if (s) loop->addChild("body", s);
                }
            }
            return loop;
        } else if (type == "for_statement") {
            auto* loop = new WhileLoop();
            loop->id = IdGenerator::next("for");
            applySpan(loop, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertJavaExpression(condNode, source);
                if (cond) loop->setChild("condition", cond);
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertJavaStatement(ts_node_named_child(bodyNode, i), source);
                    if (s) loop->addChild("body", s);
                }
            }
            return loop;
        } else if (type == "enhanced_for_statement") {
            auto* loop = new ForLoop();
            loop->id = IdGenerator::next("for");
            applySpan(loop, node);
            TSNode nameNode = childByFieldName(node, "name");
            if (!ts_node_is_null(nameNode)) {
                loop->iteratorName = nodeText(nameNode, source);
            }
            TSNode valueNode = childByFieldName(node, "value");
            if (!ts_node_is_null(valueNode)) {
                ASTNode* iter = convertJavaExpression(valueNode, source);
                if (iter) loop->setChild("iterable", iter);
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertJavaStatement(ts_node_named_child(bodyNode, i), source);
                    if (s) loop->addChild("body", s);
                }
            }
            return loop;
        } else if (type == "block") {
            auto* block = new Block();
            block->id = IdGenerator::next("block");
            applySpan(block, node);
            uint32_t bc = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < bc; ++i) {
                ASTNode* s = convertJavaStatement(ts_node_named_child(node, i), source);
                if (s) block->addChild("statements", s);
            }
            return block;
        } else if (type == "try_statement") {
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                auto* block = new Block();
                block->id = IdGenerator::next("block");
                applySpan(block, bodyNode);
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertJavaStatement(ts_node_named_child(bodyNode, i), source);
                    if (s) block->addChild("statements", s);
                }
                return block;
            }
        }

        ASTNode* expr = convertJavaExpression(node, source);
        if (expr) {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            exprStmt->setChild("expression", expr);
            return exprStmt;
        }
        return nullptr;
    }

    static ASTNode* convertJavaExpression(TSNode node,
                                          const std::string& source) {
        std::string type = nodeType(node);
        if (type == "binary_expression") {
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
            applySpan(binOp, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            TSNode opNode = childByFieldName(node, "operator");
            if (!ts_node_is_null(opNode)) binOp->op = nodeText(opNode, source);
            if (!ts_node_is_null(leftNode)) {
                ASTNode* left = convertJavaExpression(leftNode, source);
                if (left) binOp->setChild("left", left);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* right = convertJavaExpression(rightNode, source);
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
                ASTNode* target = convertJavaExpression(leftNode, source);
                if (target) assign->setChild("target", target);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* value = convertJavaExpression(rightNode, source);
                if (value) assign->setChild("value", value);
            }
            return assign;
        } else if (type == "method_invocation") {
            auto* call = new FunctionCall();
            call->id = IdGenerator::next("call");
            applySpan(call, node);
            TSNode nameNode = childByFieldName(node, "name");
            TSNode objectNode = childByFieldName(node, "object");
            if (!ts_node_is_null(objectNode) && !ts_node_is_null(nameNode)) {
                call->functionName = nodeText(objectNode, source) + "." + nodeText(nameNode, source);
            } else if (!ts_node_is_null(nameNode)) {
                call->functionName = nodeText(nameNode, source);
            } else {
                call->functionName = nodeText(node, source);
            }
            TSNode argsNode = childByFieldName(node, "arguments");
            if (!ts_node_is_null(argsNode)) {
                uint32_t count = ts_node_named_child_count(argsNode);
                for (uint32_t i = 0; i < count; ++i) {
                    ASTNode* arg = convertJavaExpression(ts_node_named_child(argsNode, i), source);
                    if (arg) call->addChild("arguments", arg);
                }
            }
            return call;
        } else if (type == "field_access") {
            auto* mem = new MemberAccess();
            mem->id = IdGenerator::next("member");
            applySpan(mem, node);
            TSNode objNode = childByFieldName(node, "object");
            TSNode fieldNode = childByFieldName(node, "field");
            if (!ts_node_is_null(fieldNode)) {
                mem->memberName = nodeText(fieldNode, source);
            }
            if (!ts_node_is_null(objNode)) {
                ASTNode* target = convertJavaExpression(objNode, source);
                if (target) mem->setChild("target", target);
            }
            return mem;
        } else if (type == "array_access") {
            auto* access = new IndexAccess();
            access->id = IdGenerator::next("index");
            applySpan(access, node);
            TSNode arrayNode = childByFieldName(node, "array");
            TSNode indexNode = childByFieldName(node, "index");
            if (!ts_node_is_null(arrayNode)) {
                ASTNode* target = convertJavaExpression(arrayNode, source);
                if (target) access->setChild("target", target);
            }
            if (!ts_node_is_null(indexNode)) {
                ASTNode* idx = convertJavaExpression(indexNode, source);
                if (idx) access->setChild("index", idx);
            }
            return access;
        } else if (type == "identifier") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        } else if (type == "integer_literal") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            applySpan(lit, node);
            return lit;
        } else if (type == "floating_point_literal") {
            auto* lit = new FloatLiteral(IdGenerator::next("float"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "string_literal" || type == "character_literal") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "true" || type == "false") {
            auto* lit = new BooleanLiteral(IdGenerator::next("bool"), type == "true");
            applySpan(lit, node);
            return lit;
        } else if (type == "null_literal") {
            auto* lit = new NullLiteral();
            lit->id = IdGenerator::next("null");
            applySpan(lit, node);
            return lit;
        } else if (type == "parenthesized_expression") {
            if (ts_node_named_child_count(node) > 0) {
                return convertJavaExpression(ts_node_named_child(node, 0), source);
            }
        } else if (type == "lambda_expression") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        }

        std::string text = nodeText(node, source);
        if (!text.empty()) {
            auto* ref = new VariableReference(IdGenerator::next("var"), text);
            applySpan(ref, node);
            return ref;
        }
        return nullptr;
    }

    static Type* convertJavaType(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "integral_type" || type == "floating_point_type" ||
            type == "boolean_type" || type == "void_type") {
            auto* prim = new PrimitiveType();
            prim->id = IdGenerator::next("type");
            prim->kind = nodeText(node, source);
            return prim;
        } else if (type == "array_type") {
            auto* arr = new ArrayType();
            arr->id = IdGenerator::next("type");
            TSNode element = childByFieldName(node, "element");
            if (!ts_node_is_null(element)) {
                if (auto* et = convertJavaType(element, source)) arr->setChild("elementType", et);
            }
            return arr;
        }
        auto* custom = new CustomType();
        custom->id = IdGenerator::next("type");
        custom->typeName = nodeText(node, source);
        return custom;
    }

    static void attachJavaAnnotations(TSNode node,
                                      const std::string& source,
                                      ASTNode* target) {
        if (!target) return;
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            if (nodeType(child) != "modifiers") continue;
            uint32_t mc = ts_node_named_child_count(child);
            for (uint32_t j = 0; j < mc; ++j) {
                TSNode modChild = ts_node_named_child(child, j);
                std::string mType = nodeType(modChild);
                if (mType == "marker_annotation" || mType == "annotation") {
                    auto* anno = new LangSpecific();
                    anno->id = IdGenerator::next("anno");
                    anno->language = "java";
                    TSNode nameNode = childByFieldName(modChild, "name");
                    anno->idiomType = ts_node_is_null(nameNode) ? "" : nodeText(nameNode, source);
                    anno->rawSyntax = nodeText(modChild, source);
                    target->addChild("annotations", anno);
                }
            }
        }
    }

    // ---------------------------------------------------------------
