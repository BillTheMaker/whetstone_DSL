#pragma once
// TreeSitterParser Rust support.
public:
    //  Rust
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseRust(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_rust());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_rust_module";
        module->targetLanguage = "rust";
        applySpan(module.get(), root);

        convertRustCrate(root, source, module.get());

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseRustWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_rust());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_rust_module";
        result.module->targetLanguage = "rust";
        applySpan(result.module.get(), root);

        convertRustCrate(root, source, result.module.get());
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
private:
    //  Rust CST -> AST
    // ---------------------------------------------------------------
    static void convertRustCrate(TSNode root,
                                 const std::string& source,
                                 Module* module) {
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "use_declaration") {
                TSNode arg = childByFieldName(child, "argument");
                std::string importName = nodeText(arg, source);
                if (importName.empty()) importName = nodeText(child, source);
                if (!importName.empty()) {
                    auto* imp = new Import(IdGenerator::next("imp"), importName, "module");
                    module->addChild("imports", imp);
                }
            } else if (type == "function_item") {
                auto* fn = convertRustFunction(child, source, "");
                if (fn) module->addChild("functions", fn);
            } else if (type == "impl_item") {
                convertRustImpl(child, source, module);
            } else if (type == "struct_item") {
                auto* cls = convertRustStruct(child, source);
                if (cls) module->addChild("classes", cls);
            } else if (type == "trait_item") {
                auto* iface = convertRustTrait(child, source);
                if (iface) module->addChild("classes", iface);
            } else if (type == "enum_item") {
                // Record enum name as variable for visibility
                TSNode nameNode = childByFieldName(child, "name");
                if (!ts_node_is_null(nameNode)) {
                    auto* var = new Variable(IdGenerator::next("var"), nodeText(nameNode, source));
                    module->addChild("variables", var);
                }
            }
        }
    }

    static ClassDeclaration* convertRustStruct(TSNode node, const std::string& source) {
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;

        auto* cls = new ClassDeclaration(IdGenerator::next("cls"), nodeText(nameNode, source));
        applySpan(cls, node);

        // Extract fields from field_declaration_list body
        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            uint32_t fc = ts_node_named_child_count(bodyNode);
            for (uint32_t i = 0; i < fc; ++i) {
                TSNode fieldNode = ts_node_named_child(bodyNode, i);
                if (nodeType(fieldNode) == "field_declaration") {
                    TSNode fName = childByFieldName(fieldNode, "name");
                    if (!ts_node_is_null(fName)) {
                        auto* var = new Variable(IdGenerator::next("var"), nodeText(fName, source));
                        applySpan(var, fieldNode);
                        TSNode fType = childByFieldName(fieldNode, "type");
                        if (!ts_node_is_null(fType)) {
                            if (auto* t = convertRustType(fType, source)) var->setChild("type", t);
                        }
                        cls->addChild("fields", var);
                    }
                }
            }
        }

        return cls;
    }

    static InterfaceDeclaration* convertRustTrait(TSNode node, const std::string& source) {
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;

        auto* iface = new InterfaceDeclaration(IdGenerator::next("iface"), nodeText(nameNode, source));
        applySpan(iface, node);

        // Extract method signatures from declaration_list body
        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            uint32_t mc = ts_node_named_child_count(bodyNode);
            for (uint32_t i = 0; i < mc; ++i) {
                TSNode methNode = ts_node_named_child(bodyNode, i);
                std::string mType = nodeType(methNode);
                if (mType == "function_signature_item" || mType == "function_item") {
                    TSNode mName = childByFieldName(methNode, "name");
                    if (!ts_node_is_null(mName)) {
                        auto* meth = new MethodDeclaration(IdGenerator::next("meth"), nodeText(mName, source));
                        applySpan(meth, methNode);
                        meth->className = iface->name;
                        meth->isVirtual = true;

                        TSNode paramsNode = childByFieldName(methNode, "parameters");
                        if (!ts_node_is_null(paramsNode)) {
                            convertRustParameters(paramsNode, source, meth);
                        }

                        TSNode retNode = childByFieldName(methNode, "return_type");
                        if (!ts_node_is_null(retNode)) {
                            if (auto* t = convertRustType(retNode, source)) meth->setChild("returnType", t);
                        }

                        iface->addChild("methods", meth);
                    }
                }
            }
        }

        return iface;
    }

    static void convertRustImpl(TSNode node,
                                const std::string& source,
                                Module* module) {
        TSNode typeNode = childByFieldName(node, "type");
        std::string typeName = nodeText(typeNode, source);
        TSNode bodyNode = childByFieldName(node, "body");
        if (ts_node_is_null(bodyNode)) return;

        // Find existing ClassDeclaration for this type to attach methods
        ClassDeclaration* targetCls = nullptr;
        auto& classes = module->getChildren("classes");
        for (auto* entry : classes) {
            auto* cls = dynamic_cast<ClassDeclaration*>(entry);
            if (cls && cls->name == typeName) {
                targetCls = cls;
                break;
            }
        }

        uint32_t count = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(bodyNode, i);
            if (nodeType(child) == "function_item" || nodeType(child) == "function_signature_item") {
                // Backward compat: add as Function to module.functions
                auto* fn = convertRustFunction(child, source, typeName);
                if (fn) module->addChild("functions", fn);

                // If we have a ClassDeclaration, also add MethodDeclaration
                if (targetCls) {
                    auto* meth = convertRustMethodDecl(child, source, typeName);
                    if (meth) targetCls->addChild("methods", meth);
                }
            }
        }
    }

    static MethodDeclaration* convertRustMethodDecl(TSNode node,
                                                     const std::string& source,
                                                     const std::string& typeName) {
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;

        auto* meth = new MethodDeclaration(IdGenerator::next("meth"), nodeText(nameNode, source));
        applySpan(meth, node);
        meth->className = typeName;

        // Check visibility modifiers
        uint32_t allCount = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < allCount; ++i) {
            TSNode ch = ts_node_named_child(node, i);
            if (nodeType(ch) == "visibility_modifier") {
                meth->visibility = "public";
            }
        }

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertRustParameters(paramsNode, source, meth);
        }

        TSNode retNode = childByFieldName(node, "return_type");
        if (!ts_node_is_null(retNode)) {
            if (auto* t = convertRustType(retNode, source)) meth->setChild("returnType", t);
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertRustBlock(bodyNode, source, meth);
        }

        return meth;
    }

    static Function* convertRustFunction(TSNode node,
                                         const std::string& source,
                                         const std::string& receiverType) {
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;

        // Detect async keyword
        bool isAsync = rustNodeHasAsyncKeyword(node, source);

        Function* fn;
        std::string name = nodeText(nameNode, source);
        if (!receiverType.empty()) name = receiverType + "." + name;

        if (isAsync) {
            fn = new AsyncFunction(IdGenerator::next("fn"), name);
        } else {
            fn = new Function();
            fn->id = IdGenerator::next("fn");
            fn->name = name;
        }
        applySpan(fn, node);

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertRustParameters(paramsNode, source, fn);
        }

        TSNode retNode = childByFieldName(node, "return_type");
        if (!ts_node_is_null(retNode)) {
            if (auto* t = convertRustType(retNode, source)) fn->setChild("returnType", t);
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertRustBlock(bodyNode, source, fn);
        }

        auto* lifetime = new LifetimeAnnotation(IdGenerator::next("anno"), "RAII");
        fn->addChild("annotations", lifetime);
        return fn;
    }

    static bool rustNodeHasAsyncKeyword(TSNode node, const std::string& source) {
        uint32_t totalCount = ts_node_child_count(node);
        for (uint32_t i = 0; i < totalCount; ++i) {
            TSNode ch = ts_node_child(node, i);
            if (!ts_node_is_named(ch) && nodeText(ch, source) == "async") return true;
        }
        // Fallback: check source text prefix
        std::string fullText = nodeText(node, source);
        if (fullText.substr(0, 6) == "async ") return true;
        return false;
    }

    static void convertRustParameters(TSNode paramsNode,
                                      const std::string& source,
                                      Function* fn) {
        uint32_t count = ts_node_named_child_count(paramsNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(paramsNode, i);
            std::string type = nodeType(child);
            if (type == "self_parameter") {
                auto* param = new Parameter(IdGenerator::next("param"), "self");
                applySpan(param, child);
                fn->addChild("parameters", param);
            } else if (type == "parameter") {
                TSNode patternNode = childByFieldName(child, "pattern");
                TSNode typeNode = childByFieldName(child, "type");
                if (ts_node_is_null(patternNode)) continue;
                auto* param = new Parameter(IdGenerator::next("param"), nodeText(patternNode, source));
                applySpan(param, child);
                if (!ts_node_is_null(typeNode)) {
                    if (auto* t = convertRustType(typeNode, source)) param->setChild("type", t);
                }
                fn->addChild("parameters", param);
            }
        }
    }

    static void convertRustBlock(TSNode blockNode,
                                 const std::string& source,
                                 Function* fn) {
        uint32_t count = ts_node_named_child_count(blockNode);
        for (uint32_t i = 0; i < count; ++i) {
            ASTNode* stmt = convertRustStatement(ts_node_named_child(blockNode, i), source);
            if (stmt) fn->addChild("body", stmt);
        }
    }

    static ASTNode* convertRustStatement(TSNode node,
                                         const std::string& source) {
        std::string type = nodeType(node);
        if (type == "return_expression") {
            auto* ret = new Return();
            ret->id = IdGenerator::next("ret");
            applySpan(ret, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* val = convertRustExpression(ts_node_named_child(node, 0), source);
                if (val) ret->setChild("value", val);
            }
            return ret;
        } else if (type == "let_declaration") {
            auto* assign = new Assignment();
            assign->id = IdGenerator::next("assign");
            applySpan(assign, node);
            TSNode patternNode = childByFieldName(node, "pattern");
            TSNode valueNode = childByFieldName(node, "value");
            if (!ts_node_is_null(patternNode)) {
                auto* target = new VariableReference(IdGenerator::next("var"), nodeText(patternNode, source));
                assign->setChild("target", target);
            }
            if (!ts_node_is_null(valueNode)) {
                ASTNode* val = convertRustExpression(valueNode, source);
                if (val) assign->setChild("value", val);
            }
            return assign;
        } else if (type == "expression_statement") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* expr = convertRustExpression(ts_node_named_child(node, 0), source);
                if (expr) exprStmt->setChild("expression", expr);
            }
            return exprStmt;
        } else if (type == "if_expression") {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertRustExpression(condNode, source);
                if (cond) ifStmt->setChild("condition", cond);
            }
            TSNode consNode = childByFieldName(node, "consequence");
            if (!ts_node_is_null(consNode)) {
                uint32_t cc = ts_node_named_child_count(consNode);
                for (uint32_t i = 0; i < cc; ++i) {
                    ASTNode* s = convertRustStatement(ts_node_named_child(consNode, i), source);
                    if (s) ifStmt->addChild("thenBranch", s);
                }
            }
            TSNode altNode = childByFieldName(node, "alternative");
            if (!ts_node_is_null(altNode)) {
                uint32_t ac = ts_node_named_child_count(altNode);
                for (uint32_t i = 0; i < ac; ++i) {
                    ASTNode* s = convertRustStatement(ts_node_named_child(altNode, i), source);
                    if (s) ifStmt->addChild("elseBranch", s);
                }
            }
            return ifStmt;
        } else if (type == "while_expression") {
            auto* loop = new WhileLoop();
            loop->id = IdGenerator::next("while");
            applySpan(loop, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertRustExpression(condNode, source);
                if (cond) loop->setChild("condition", cond);
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertRustStatement(ts_node_named_child(bodyNode, i), source);
                    if (s) loop->addChild("body", s);
                }
            }
            return loop;
        } else if (type == "for_expression") {
            auto* loop = new ForLoop();
            loop->id = IdGenerator::next("for");
            applySpan(loop, node);
            TSNode patternNode = childByFieldName(node, "pattern");
            if (!ts_node_is_null(patternNode)) {
                loop->iteratorName = nodeText(patternNode, source);
            }
            TSNode valueNode = childByFieldName(node, "value");
            if (!ts_node_is_null(valueNode)) {
                ASTNode* iter = convertRustExpression(valueNode, source);
                if (iter) loop->setChild("iterable", iter);
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertRustStatement(ts_node_named_child(bodyNode, i), source);
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
                ASTNode* s = convertRustStatement(ts_node_named_child(node, i), source);
                if (s) block->addChild("statements", s);
            }
            return block;
        }

        ASTNode* expr = convertRustExpression(node, source);
        if (expr) {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            exprStmt->setChild("expression", expr);
            return exprStmt;
        }
        return nullptr;
    }

    static ASTNode* convertRustExpression(TSNode node,
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
                ASTNode* left = convertRustExpression(leftNode, source);
                if (left) binOp->setChild("left", left);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* right = convertRustExpression(rightNode, source);
                if (right) binOp->setChild("right", right);
            }
            return binOp;
        } else if (type == "assignment_expression" || type == "compound_assignment_expr") {
            auto* assign = new Assignment();
            assign->id = IdGenerator::next("assign");
            applySpan(assign, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            if (!ts_node_is_null(leftNode)) {
                ASTNode* target = convertRustExpression(leftNode, source);
                if (target) assign->setChild("target", target);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* value = convertRustExpression(rightNode, source);
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
                    ASTNode* arg = convertRustExpression(ts_node_named_child(argsNode, i), source);
                    if (arg) call->addChild("arguments", arg);
                }
            }
            return call;
        } else if (type == "field_expression") {
            auto* mem = new MemberAccess();
            mem->id = IdGenerator::next("member");
            applySpan(mem, node);
            TSNode valueNode = childByFieldName(node, "value");
            TSNode fieldNode = childByFieldName(node, "field");
            if (!ts_node_is_null(fieldNode)) {
                mem->memberName = nodeText(fieldNode, source);
            }
            if (!ts_node_is_null(valueNode)) {
                ASTNode* target = convertRustExpression(valueNode, source);
                if (target) mem->setChild("target", target);
            }
            return mem;
        } else if (type == "index_expression") {
            auto* access = new IndexAccess();
            access->id = IdGenerator::next("index");
            applySpan(access, node);
            if (ts_node_named_child_count(node) >= 2) {
                ASTNode* target = convertRustExpression(ts_node_named_child(node, 0), source);
                ASTNode* idx = convertRustExpression(ts_node_named_child(node, 1), source);
                if (target) access->setChild("target", target);
                if (idx) access->setChild("index", idx);
            }
            return access;
        } else if (type == "closure_expression") {
            auto* lam = new LambdaExpression(IdGenerator::next("lam"));
            applySpan(lam, node);
            TSNode paramsNode = childByFieldName(node, "parameters");
            if (!ts_node_is_null(paramsNode)) {
                uint32_t pc = ts_node_named_child_count(paramsNode);
                for (uint32_t pi = 0; pi < pc; ++pi) {
                    TSNode pChild = ts_node_named_child(paramsNode, pi);
                    std::string pType = nodeType(pChild);
                    std::string paramName;
                    if (pType == "identifier") {
                        paramName = nodeText(pChild, source);
                    } else if (pType == "parameter") {
                        TSNode pName = childByFieldName(pChild, "pattern");
                        if (ts_node_is_null(pName)) pName = pChild;
                        paramName = nodeText(pName, source);
                    } else {
                        paramName = nodeText(pChild, source);
                    }
                    if (!paramName.empty()) {
                        auto* param = new Parameter(IdGenerator::next("param"), paramName);
                        applySpan(param, pChild);
                        lam->addChild("parameters", param);
                    }
                }
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                if (nodeType(bodyNode) == "block") {
                    uint32_t bc = ts_node_named_child_count(bodyNode);
                    for (uint32_t bi = 0; bi < bc; ++bi) {
                        ASTNode* stmt = convertRustStatement(ts_node_named_child(bodyNode, bi), source);
                        if (stmt) lam->addChild("body", stmt);
                    }
                } else {
                    ASTNode* expr = convertRustExpression(bodyNode, source);
                    if (expr) {
                        auto* exprStmt = new ExpressionStatement();
                        exprStmt->id = IdGenerator::next("exprstmt");
                        exprStmt->setChild("expression", expr);
                        lam->addChild("body", exprStmt);
                    }
                }
            }
            return lam;
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
        } else if (type == "float_literal") {
            auto* lit = new FloatLiteral(IdGenerator::next("float"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "string_literal" || type == "char_literal") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "true" || type == "false") {
            auto* lit = new BooleanLiteral(IdGenerator::next("bool"), type == "true");
            applySpan(lit, node);
            return lit;
        } else if (type == "unit_expression") {
            auto* lit = new NullLiteral();
            lit->id = IdGenerator::next("null");
            applySpan(lit, node);
            return lit;
        } else if (type == "parenthesized_expression") {
            if (ts_node_named_child_count(node) > 0) {
                return convertRustExpression(ts_node_named_child(node, 0), source);
            }
        } else if (type == "array_expression") {
            auto* list = new ListLiteral();
            list->id = IdGenerator::next("list");
            applySpan(list, node);
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                ASTNode* elem = convertRustExpression(ts_node_named_child(node, i), source);
                if (elem) list->addChild("elements", elem);
            }
            return list;
        }

        std::string text = nodeText(node, source);
        if (!text.empty()) {
            auto* ref = new VariableReference(IdGenerator::next("var"), text);
            applySpan(ref, node);
            return ref;
        }
        return nullptr;
    }

    static Type* convertRustType(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "primitive_type") {
            auto* prim = new PrimitiveType();
            prim->id = IdGenerator::next("type");
            prim->kind = nodeText(node, source);
            return prim;
        } else if (type == "reference_type") {
            TSNode valueNode = childByFieldName(node, "value");
            if (!ts_node_is_null(valueNode)) {
                if (auto* inner = convertRustType(valueNode, source)) {
                    auto* opt = new OptionalType();
                    opt->id = IdGenerator::next("type");
                    opt->setChild("innerType", inner);
                    return opt;
                }
            }
        } else if (type == "array_type") {
            auto* arr = new ArrayType();
            arr->id = IdGenerator::next("type");
            TSNode elemNode = childByFieldName(node, "element");
            if (!ts_node_is_null(elemNode)) {
                if (auto* et = convertRustType(elemNode, source)) arr->setChild("elementType", et);
            }
            return arr;
        } else if (type == "tuple_type") {
            auto* tuple = new TupleType();
            tuple->id = IdGenerator::next("type");
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                if (auto* t = convertRustType(ts_node_named_child(node, i), source)) {
                    tuple->addChild("elementTypes", t);
                }
            }
            return tuple;
        }
        auto* custom = new CustomType();
        custom->id = IdGenerator::next("type");
        custom->typeName = nodeText(node, source);
        return custom;
    }

    // ---------------------------------------------------------------
