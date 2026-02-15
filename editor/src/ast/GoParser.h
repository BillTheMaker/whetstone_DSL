#pragma once
// TreeSitterParser Go support.
public:
    //  Go
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseGo(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_go());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_go_module";
        module->targetLanguage = "go";
        applySpan(module.get(), root);

        convertGoSourceFile(root, source, module.get());

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseGoWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_go());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_go_module";
        result.module->targetLanguage = "go";
        applySpan(result.module.get(), root);

        convertGoSourceFile(root, source, result.module.get());
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

private:
    //  Go CST -> AST
    // ---------------------------------------------------------------
    static void convertGoSourceFile(TSNode root,
                                    const std::string& source,
                                    Module* module) {
        // First pass: collect type declarations (structs/interfaces)
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "type_declaration") {
                convertGoTypeDeclaration(child, source, module);
            }
        }
        // Second pass: functions, methods, vars, imports
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "import_declaration") {
                convertGoImport(child, source, module);
            } else if (type == "function_declaration") {
                auto* fn = convertGoFunction(child, source, "");
                if (fn) module->addChild("functions", fn);
            } else if (type == "method_declaration") {
                convertGoMethodDecl(child, source, module);
            } else if (type == "var_declaration") {
                convertGoVarDeclaration(child, source, module);
            }
        }
    }

    static void convertGoImport(TSNode node,
                                const std::string& source,
                                Module* module) {
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode spec = ts_node_named_child(node, i);
            if (nodeType(spec) != "import_spec") continue;
            TSNode pathNode = childByFieldName(spec, "path");
            std::string path = nodeText(pathNode, source);
            if (!path.empty()) {
                auto* imp = new Import(IdGenerator::next("imp"), path, "module");
                module->addChild("imports", imp);
            }
        }
    }

    static void convertGoVarDeclaration(TSNode node,
                                        const std::string& source,
                                        Module* module) {
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode spec = ts_node_named_child(node, i);
            if (nodeType(spec) != "var_spec") continue;
            TSNode nameNode = childByFieldName(spec, "name");
            TSNode typeNode = childByFieldName(spec, "type");
            TSNode valueNode = childByFieldName(spec, "value");
            if (ts_node_is_null(nameNode)) continue;
            auto* var = new Variable(IdGenerator::next("var"), nodeText(nameNode, source));
            applySpan(var, spec);
            if (!ts_node_is_null(typeNode)) {
                if (auto* t = convertGoType(typeNode, source)) var->setChild("type", t);
            }
            if (!ts_node_is_null(valueNode)) {
                ASTNode* init = convertGoExpression(valueNode, source);
                if (init) var->setChild("initializer", init);
            }
            module->addChild("variables", var);
        }
    }

    static void convertGoTypeDeclaration(TSNode node,
                                         const std::string& source,
                                         Module* module) {
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode spec = ts_node_named_child(node, i);
            if (nodeType(spec) != "type_spec") continue;
            TSNode nameNode = childByFieldName(spec, "name");
            if (ts_node_is_null(nameNode)) continue;
            std::string typeName = nodeText(nameNode, source);

            TSNode typeNode = childByFieldName(spec, "type");
            if (!ts_node_is_null(typeNode)) {
                std::string typeKind = nodeType(typeNode);
                if (typeKind == "struct_type") {
                    auto* cls = new ClassDeclaration(IdGenerator::next("cls"), typeName);
                    applySpan(cls, spec);
                    // Extract fields from struct body
                    uint32_t fc = ts_node_named_child_count(typeNode);
                    for (uint32_t f = 0; f < fc; ++f) {
                        TSNode field = ts_node_named_child(typeNode, f);
                        if (nodeType(field) == "field_declaration" ||
                            nodeType(field) == "field_declaration_list") {
                            TSNode fnameNode = childByFieldName(field, "name");
                            if (!ts_node_is_null(fnameNode)) {
                                auto* var = new Variable(IdGenerator::next("var"),
                                                         nodeText(fnameNode, source));
                                applySpan(var, field);
                                cls->addChild("fields", var);
                            }
                        }
                    }
                    module->addChild("classes", cls);
                    continue;
                } else if (typeKind == "interface_type") {
                    auto* iface = new InterfaceDeclaration(IdGenerator::next("iface"), typeName);
                    applySpan(iface, spec);
                    // Extract method signatures
                    uint32_t mc = ts_node_named_child_count(typeNode);
                    for (uint32_t m = 0; m < mc; ++m) {
                        TSNode methSpec = ts_node_named_child(typeNode, m);
                        if (nodeType(methSpec) == "method_spec" ||
                            nodeType(methSpec) == "method_elem") {
                            TSNode methName = childByFieldName(methSpec, "name");
                            if (!ts_node_is_null(methName)) {
                                auto* meth = new MethodDeclaration(
                                    IdGenerator::next("meth"), nodeText(methName, source));
                                meth->className = typeName;
                                meth->isVirtual = true;
                                applySpan(meth, methSpec);
                                iface->addChild("methods", meth);
                            }
                        }
                    }
                    module->addChild("classes", iface);
                    continue;
                }
            }
            // Fallback: non-struct/interface type alias → Variable
            auto* var = new Variable(IdGenerator::next("var"), typeName);
            module->addChild("variables", var);
        }
    }

    static Function* convertGoFunction(TSNode node,
                                       const std::string& source,
                                       const std::string& receiverType) {
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        std::string name = nodeText(nameNode, source);
        fn->name = receiverType.empty() ? name : (receiverType + "." + name);

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertGoParameters(paramsNode, source, fn);
        }
        TSNode resultNode = childByFieldName(node, "result");
        if (!ts_node_is_null(resultNode)) {
            if (auto* t = convertGoType(resultNode, source)) fn->setChild("returnType", t);
        }
        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertGoBlock(bodyNode, source, fn);
        }

        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Escape");
        fn->addChild("annotations", reclaim);
        return fn;
    }

    static Function* convertGoMethod(TSNode node,
                                     const std::string& source) {
        TSNode recvNode = childByFieldName(node, "receiver");
        std::string receiverType;
        if (!ts_node_is_null(recvNode)) {
            TSNode typeNode = findDescendantByField(recvNode, "type");
            if (!ts_node_is_null(typeNode)) {
                receiverType = nodeText(typeNode, source);
            }
            if (receiverType.empty()) {
                TSNode nameNode = findDescendantByField(recvNode, "name");
                if (!ts_node_is_null(nameNode)) {
                    receiverType = nodeText(nameNode, source);
                }
            }
            receiverType = trimPointerPrefix(receiverType);
        }
        return convertGoFunction(node, source, receiverType);
    }

    // Create a MethodDeclaration and attach to existing ClassDeclaration
    static void convertGoMethodDecl(TSNode node,
                                    const std::string& source,
                                    Module* module) {
        TSNode recvNode = childByFieldName(node, "receiver");
        std::string receiverType;
        if (!ts_node_is_null(recvNode)) {
            TSNode typeNode = findDescendantByField(recvNode, "type");
            if (!ts_node_is_null(typeNode)) {
                receiverType = nodeText(typeNode, source);
            }
            if (receiverType.empty()) {
                TSNode nameNode = findDescendantByField(recvNode, "name");
                if (!ts_node_is_null(nameNode)) {
                    receiverType = nodeText(nameNode, source);
                }
            }
            receiverType = trimPointerPrefix(receiverType);
        }

        // Extract method name
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return;
        std::string methodName = nodeText(nameNode, source);

        // Create MethodDeclaration
        auto* meth = new MethodDeclaration(IdGenerator::next("meth"), methodName);
        meth->className = receiverType;
        applySpan(meth, node);

        // Parameters
        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertGoParameters(paramsNode, source, meth);
        }
        // Return type
        TSNode resultNode = childByFieldName(node, "result");
        if (!ts_node_is_null(resultNode)) {
            if (auto* t = convertGoType(resultNode, source)) meth->setChild("returnType", t);
        }
        // Body
        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertGoBlock(bodyNode, source, meth);
        }

        // Try to attach to existing ClassDeclaration
        bool attached = false;
        if (!receiverType.empty()) {
            auto& classes = module->getChildren("classes");
            for (auto* entry : classes) {
                auto* cls = dynamic_cast<ClassDeclaration*>(entry);
                if (cls && cls->name == receiverType) {
                    cls->addChild("methods", meth);
                    attached = true;
                    break;
                }
            }
        }
        // Also add as backward-compat Function in functions list
        auto* fn = convertGoFunction(node, source, receiverType);
        if (fn) module->addChild("functions", fn);

        // If not attached to any class, the MethodDeclaration is still owned by the class search
        if (!attached) {
            // Create a ClassDeclaration stub for the receiver type
            if (!receiverType.empty()) {
                auto* cls = new ClassDeclaration(IdGenerator::next("cls"), receiverType);
                cls->addChild("methods", meth);
                module->addChild("classes", cls);
            } else {
                delete meth;
            }
        }
    }

    static void convertGoParameters(TSNode paramsNode,
                                    const std::string& source,
                                    Function* fn) {
        uint32_t count = ts_node_named_child_count(paramsNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(paramsNode, i);
            if (nodeType(child) != "parameter_declaration" &&
                nodeType(child) != "variadic_parameter_declaration") {
                continue;
            }
            TSNode nameNode = childByFieldName(child, "name");
            TSNode typeNode = childByFieldName(child, "type");
            if (ts_node_is_null(nameNode)) continue;
            auto* param = new Parameter(IdGenerator::next("param"), nodeText(nameNode, source));
            applySpan(param, child);
            if (!ts_node_is_null(typeNode)) {
                if (auto* t = convertGoType(typeNode, source)) param->setChild("type", t);
            }
            fn->addChild("parameters", param);
        }
    }

    static void convertGoBlock(TSNode blockNode,
                               const std::string& source,
                               Function* fn) {
        uint32_t count = ts_node_named_child_count(blockNode);
        for (uint32_t i = 0; i < count; ++i) {
            ASTNode* stmt = convertGoStatement(ts_node_named_child(blockNode, i), source);
            if (stmt) fn->addChild("body", stmt);
        }
    }

    static ASTNode* convertGoStatement(TSNode node,
                                       const std::string& source) {
        std::string type = nodeType(node);
        if (type == "return_statement") {
            auto* ret = new Return();
            ret->id = IdGenerator::next("ret");
            applySpan(ret, node);
            TSNode valueNode = childByFieldName(node, "value");
            if (!ts_node_is_null(valueNode)) {
                ASTNode* val = convertGoExpression(valueNode, source);
                if (val) ret->setChild("value", val);
            }
            return ret;
        } else if (type == "short_var_declaration") {
            auto* assign = new Assignment();
            assign->id = IdGenerator::next("assign");
            applySpan(assign, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            if (!ts_node_is_null(leftNode)) {
                ASTNode* target = convertGoExpression(leftNode, source);
                if (target) assign->setChild("target", target);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* value = convertGoExpression(rightNode, source);
                if (value) assign->setChild("value", value);
            }
            return assign;
        } else if (type == "assignment_statement") {
            auto* assign = new Assignment();
            assign->id = IdGenerator::next("assign");
            applySpan(assign, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            if (!ts_node_is_null(leftNode)) {
                ASTNode* target = convertGoExpression(leftNode, source);
                if (target) assign->setChild("target", target);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* value = convertGoExpression(rightNode, source);
                if (value) assign->setChild("value", value);
            }
            return assign;
        } else if (type == "expression_statement") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* expr = convertGoExpression(ts_node_named_child(node, 0), source);
                if (expr) exprStmt->setChild("expression", expr);
            }
            return exprStmt;
        } else if (type == "if_statement") {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertGoExpression(condNode, source);
                if (cond) ifStmt->setChild("condition", cond);
            }
            TSNode consNode = childByFieldName(node, "consequence");
            if (!ts_node_is_null(consNode)) {
                uint32_t cc = ts_node_named_child_count(consNode);
                for (uint32_t i = 0; i < cc; ++i) {
                    ASTNode* s = convertGoStatement(ts_node_named_child(consNode, i), source);
                    if (s) ifStmt->addChild("thenBranch", s);
                }
            }
            TSNode altNode = childByFieldName(node, "alternative");
            if (!ts_node_is_null(altNode)) {
                uint32_t ac = ts_node_named_child_count(altNode);
                for (uint32_t i = 0; i < ac; ++i) {
                    ASTNode* s = convertGoStatement(ts_node_named_child(altNode, i), source);
                    if (s) ifStmt->addChild("elseBranch", s);
                }
            }
            return ifStmt;
        } else if (type == "for_statement") {
            auto* loop = new ForLoop();
            loop->id = IdGenerator::next("for");
            applySpan(loop, node);
            TSNode rangeNode = childByFieldName(node, "right");
            if (!ts_node_is_null(rangeNode)) {
                ASTNode* iter = convertGoExpression(rangeNode, source);
                if (iter) loop->setChild("iterable", iter);
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertGoStatement(ts_node_named_child(bodyNode, i), source);
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
                ASTNode* s = convertGoStatement(ts_node_named_child(node, i), source);
                if (s) block->addChild("statements", s);
            }
            return block;
        }

        ASTNode* expr = convertGoExpression(node, source);
        if (expr) {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            exprStmt->setChild("expression", expr);
            return exprStmt;
        }
        return nullptr;
    }

    static ASTNode* convertGoExpression(TSNode node,
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
                ASTNode* left = convertGoExpression(leftNode, source);
                if (left) binOp->setChild("left", left);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* right = convertGoExpression(rightNode, source);
                if (right) binOp->setChild("right", right);
            }
            return binOp;
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
                    ASTNode* arg = convertGoExpression(ts_node_named_child(argsNode, i), source);
                    if (arg) call->addChild("arguments", arg);
                }
            }
            return call;
        } else if (type == "selector_expression") {
            auto* mem = new MemberAccess();
            mem->id = IdGenerator::next("member");
            applySpan(mem, node);
            TSNode operandNode = childByFieldName(node, "operand");
            TSNode fieldNode = childByFieldName(node, "field");
            if (!ts_node_is_null(fieldNode)) {
                mem->memberName = nodeText(fieldNode, source);
            }
            if (!ts_node_is_null(operandNode)) {
                ASTNode* target = convertGoExpression(operandNode, source);
                if (target) mem->setChild("target", target);
            }
            return mem;
        } else if (type == "index_expression") {
            auto* access = new IndexAccess();
            access->id = IdGenerator::next("index");
            applySpan(access, node);
            TSNode operandNode = childByFieldName(node, "operand");
            TSNode indexNode = childByFieldName(node, "index");
            if (!ts_node_is_null(operandNode)) {
                ASTNode* target = convertGoExpression(operandNode, source);
                if (target) access->setChild("target", target);
            }
            if (!ts_node_is_null(indexNode)) {
                ASTNode* idx = convertGoExpression(indexNode, source);
                if (idx) access->setChild("index", idx);
            }
            return access;
        } else if (type == "identifier") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        } else if (type == "int_literal") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            applySpan(lit, node);
            return lit;
        } else if (type == "float_literal") {
            auto* lit = new FloatLiteral(IdGenerator::next("float"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "raw_string_literal" || type == "interpreted_string_literal") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "true" || type == "false") {
            auto* lit = new BooleanLiteral(IdGenerator::next("bool"), type == "true");
            applySpan(lit, node);
            return lit;
        } else if (type == "nil") {
            auto* lit = new NullLiteral();
            lit->id = IdGenerator::next("null");
            applySpan(lit, node);
            return lit;
        } else if (type == "parenthesized_expression") {
            if (ts_node_named_child_count(node) > 0) {
                return convertGoExpression(ts_node_named_child(node, 0), source);
            }
        } else if (type == "expression_list") {
            // Unwrap single-element expression lists
            uint32_t elc = ts_node_named_child_count(node);
            if (elc == 1) {
                return convertGoExpression(ts_node_named_child(node, 0), source);
            }
            if (elc > 0) {
                return convertGoExpression(ts_node_named_child(node, 0), source);
            }
        } else if (type == "func_literal") {
            auto* lambda = new LambdaExpression(IdGenerator::next("lambda"));
            applySpan(lambda, node);
            TSNode paramsNode = childByFieldName(node, "parameters");
            if (!ts_node_is_null(paramsNode)) {
                uint32_t pc = ts_node_named_child_count(paramsNode);
                for (uint32_t p = 0; p < pc; ++p) {
                    TSNode paramChild = ts_node_named_child(paramsNode, p);
                    if (nodeType(paramChild) == "parameter_declaration" ||
                        nodeType(paramChild) == "variadic_parameter_declaration") {
                        TSNode pnameNode = childByFieldName(paramChild, "name");
                        if (!ts_node_is_null(pnameNode)) {
                            auto* param = new Parameter(IdGenerator::next("param"),
                                                        nodeText(pnameNode, source));
                            applySpan(param, paramChild);
                            lambda->addChild("parameters", param);
                        }
                    }
                }
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t b = 0; b < bc; ++b) {
                    ASTNode* stmt = convertGoStatement(ts_node_named_child(bodyNode, b), source);
                    if (stmt) lambda->addChild("body", stmt);
                }
            }
            return lambda;
        }

        std::string text = nodeText(node, source);
        if (!text.empty()) {
            auto* ref = new VariableReference(IdGenerator::next("var"), text);
            applySpan(ref, node);
            return ref;
        }
        return nullptr;
    }

    static Type* convertGoType(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "qualified_type") {
            auto* custom = new CustomType();
            custom->id = IdGenerator::next("type");
            custom->typeName = nodeText(node, source);
            return custom;
        } else if (type == "pointer_type") {
            TSNode elemNode = childByFieldName(node, "type");
            if (!ts_node_is_null(elemNode)) {
                if (auto* inner = convertGoType(elemNode, source)) {
                    auto* opt = new OptionalType();
                    opt->id = IdGenerator::next("type");
                    opt->setChild("innerType", inner);
                    return opt;
                }
            }
        } else if (type == "array_type" || type == "slice_type") {
            auto* arr = new ArrayType();
            arr->id = IdGenerator::next("type");
            TSNode elemNode = childByFieldName(node, "element");
            if (!ts_node_is_null(elemNode)) {
                if (auto* et = convertGoType(elemNode, source)) arr->setChild("elementType", et);
            }
            return arr;
        } else if (type == "map_type") {
            auto* map = new MapType();
            map->id = IdGenerator::next("type");
            TSNode keyNode = childByFieldName(node, "key");
            TSNode valNode = childByFieldName(node, "value");
            if (!ts_node_is_null(keyNode)) {
                if (auto* kt = convertGoType(keyNode, source)) map->setChild("keyType", kt);
            }
            if (!ts_node_is_null(valNode)) {
                if (auto* vt = convertGoType(valNode, source)) map->setChild("valueType", vt);
            }
            return map;
        } else if (type == "struct_type") {
            auto* custom = new CustomType();
            custom->id = IdGenerator::next("type");
            custom->typeName = "struct";
            return custom;
        } else if (type == "interface_type") {
            auto* custom = new CustomType();
            custom->id = IdGenerator::next("type");
            custom->typeName = "interface{}";
            return custom;
        }
        auto* custom = new CustomType();
        custom->id = IdGenerator::next("type");
        custom->typeName = nodeText(node, source);
        return custom;
    }

    // ---------------------------------------------------------------
