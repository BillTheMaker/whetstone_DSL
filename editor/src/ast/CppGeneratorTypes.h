#pragma once
// CppGenerator type + annotation helpers.
    std::string visitListType(const ListType* type) override {
        std::ostringstream oss;
        oss << "std::vector<";

        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "auto";  // Default if no element type specified
        }

        oss << ">";
        return oss.str();
    }

    std::string visitSetType(const SetType* type) override {
        std::ostringstream oss;
        oss << "std::set<";

        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "auto";  // Default if no element type specified
        }

        oss << ">";
        return oss.str();
    }

    std::string visitMapType(const MapType* type) override {
        std::ostringstream oss;
        oss << "std::map<";

        auto keyType = type->getChild("keyType");
        auto valueType = type->getChild("valueType");

        if (keyType) {
            oss << generate(keyType);
        } else {
            oss << "auto";  // Default if no key type specified
        }

        oss << ", ";

        if (valueType) {
            oss << generate(valueType);
        } else {
            oss << "auto";  // Default if no value type specified
        }

        oss << ">";
        return oss.str();
    }

    std::string visitTupleType(const TupleType* type) override {
        std::ostringstream oss;
        oss << "std::tuple<";

        auto elementTypes = type->getChildren("elementTypes");
        for (size_t i = 0; i < elementTypes.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(elementTypes[i]);
        }

        oss << ">";
        return oss.str();
    }

    std::string visitArrayType(const ArrayType* type) override {
        std::ostringstream oss;
        oss << "std::array<";

        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "auto";  // Default if no element type specified
        }

        oss << ", /* size unknown */>";
        return oss.str();
    }

    std::string visitOptionalType(const OptionalType* type) override {
        std::ostringstream oss;
        oss << "std::optional<";

        auto innerType = type->getChild("innerType");
        if (innerType) {
            oss << generate(innerType);
        } else {
            oss << "auto";  // Default if no inner type specified
        }

        oss << ">";
        return oss.str();
    }

    std::string visitCustomType(const CustomType* type) override {
        return type->typeName;
    }

    // --- New AST node visitors (Step 304) ---

    std::string visitClassDeclaration(const ASTNode* node) override {
        auto* cls = static_cast<const ClassDeclaration*>(node);
        std::ostringstream oss;
        auto annotations = cls->getChildren("annotations");
        for (const auto* a : annotations) oss << generate(a) << "\n";
        // Template prefix if has GenericType children with isClassTemplate
        auto tpChildren = cls->getChildren("typeParameters");
        for (const auto* tpc : tpChildren) {
            if (tpc->conceptType == "GenericType") {
                auto* gt = static_cast<const GenericType*>(tpc);
                if (gt->isClassTemplate) {
                    oss << "template<";
                    auto params = gt->getChildren("typeParameters");
                    for (size_t i = 0; i < params.size(); ++i) {
                        if (i > 0) oss << ", ";
                        auto* tp = static_cast<const TypeParameter*>(params[i]);
                        if (tp->isVariadic) oss << "typename... " << tp->name;
                        else oss << "typename " << tp->name;
                    }
                    oss << ">\n";
                    break;
                }
            }
        }
        auto methods = cls->getChildren("methods");
        const bool useStruct = methods.empty();

        oss << (useStruct ? "struct " : "class ") << cls->name;
        auto bases = cls->getBases();
        if (!bases.empty()) {
            oss << " : ";
            for (size_t i = 0; i < bases.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << bases[i].accessSpecifier;
                if (bases[i].isVirtual) oss << " virtual";
                oss << " " << bases[i].name;
            }
        }
        oss << " {\n";
        if (!useStruct) oss << "public:\n";
        auto fields = cls->getChildren("fields");
        for (const auto* f : fields) {
            auto* var = static_cast<const Variable*>(f);
            oss << "    " << inferFieldType(var) << " " << var->name << ";\n";
        }

        if (!methods.empty() && !fields.empty()) oss << "\n";
        for (const auto* m : methods) {
            auto* meth = static_cast<const MethodDeclaration*>(m);
            oss << "    ";
            if (meth->isStatic) oss << "static ";

            std::string methodName = meth->name;
            auto dot = methodName.find_last_of('.');
            if (dot != std::string::npos && dot + 1 < methodName.size()) {
                methodName = methodName.substr(dot + 1);
            }
            const bool isCtor = (methodName == "__init__" || methodName == "constructor" || methodName == cls->name);

            if (isCtor) {
                oss << cls->name;
            } else {
                oss << inferReturnType(meth) << " " << methodName;
            }

            oss << "(";
            auto params = meth->getChildren("parameters");
            bool firstParam = true;
            for (size_t i = 0; i < params.size(); ++i) {
                auto* param = static_cast<const Parameter*>(params[i]);
                if (!param) continue;
                if (param->name == "self" || param->name == "cls") continue;
                if (!firstParam) oss << ", ";
                firstParam = false;
                oss << inferParameterType(param, meth) << " " << param->name;
            }
            oss << ");\n";
        }
        oss << "};\n";
        return oss.str();
    }

    static std::string mapScalarTypeName(const std::string& rawType) {
        std::string kind = rawType;
        std::transform(kind.begin(), kind.end(), kind.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (kind == "string" || kind == "str") return "std::string";
        if (kind == "int" || kind == "integer") return "int";
        if (kind == "bool" || kind == "boolean") return "bool";
        if (kind == "float" || kind == "double") return "double";
        return rawType;
    }

    static std::string inferFieldType(const Variable* var) {
        if (!var) return "auto /* TODO: specify type */";
        auto* type = var->getChild("type");
        if (type && type->conceptType == "PrimitiveType") {
            return mapScalarTypeName(static_cast<const PrimitiveType*>(type)->kind);
        }
        if (type && type->conceptType == "CustomType") {
            return mapScalarTypeName(static_cast<const CustomType*>(type)->typeName);
        }
        if (type && type->conceptType == "ListType") {
            auto* elem = type->getChild("elementType");
            if (!elem) return "std::vector<std::string>";
            if (elem->conceptType == "PrimitiveType") {
                return "std::vector<" + mapScalarTypeName(static_cast<const PrimitiveType*>(elem)->kind) + ">";
            }
            if (elem->conceptType == "CustomType") {
                return "std::vector<" + mapScalarTypeName(static_cast<const CustomType*>(elem)->typeName) + ">";
            }
            return "std::vector<std::string>";
        }
        auto* init = var->getChild("initializer");
        if (init) {
            if (init->conceptType == "StringLiteral") return "std::string";
            if (init->conceptType == "IntegerLiteral") return "int";
            if (init->conceptType == "BooleanLiteral") return "bool";
            if (init->conceptType == "FloatLiteral") return "double";
        }

        std::string lower = var->name;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (lower.find("name") != std::string::npos ||
            lower.find("id") != std::string::npos ||
            lower.find("payload") != std::string::npos) {
            return "std::string";
        }
        if (lower.find("priority") != std::string::npos ||
            lower.find("count") != std::string::npos ||
            lower.find("size") != std::string::npos ||
            lower.find("index") != std::string::npos) {
            return "int";
        }
        return "auto /* TODO: specify type */";
    }

    static std::string inferReturnType(const MethodDeclaration* meth) {
        if (!meth) return "auto";
        auto* ret = meth->getChild("returnType");
        if (!ret) {
            std::string lower = meth->name;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lower.find("enqueue") != std::string::npos || lower.find("push") != std::string::npos) {
                return "void";
            }
            if (lower.find("dequeue") != std::string::npos || lower.find("peek") != std::string::npos ||
                lower.find("pop") != std::string::npos) {
                return "std::string";
            }
            if (lower.find("size") != std::string::npos) return "int";
            if (lower.find("empty") != std::string::npos) return "bool";
            return "void";
        }
        if (ret->conceptType == "PrimitiveType") {
            return mapScalarTypeName(static_cast<const PrimitiveType*>(ret)->kind);
        }
        if (ret->conceptType == "CustomType") {
            return mapScalarTypeName(static_cast<const CustomType*>(ret)->typeName);
        }
        return "auto";
    }

    static std::string inferParameterType(const Parameter* param,
                                          const MethodDeclaration* method = nullptr) {
        if (!param) return "auto";
        auto* type = param->getChild("type");
        if (type && type->conceptType == "PrimitiveType") {
            return mapScalarTypeName(static_cast<const PrimitiveType*>(type)->kind);
        }
        if (type && type->conceptType == "CustomType") {
            return mapScalarTypeName(static_cast<const CustomType*>(type)->typeName);
        }

        std::string lower = param->name;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (lower.find("name") != std::string::npos ||
            lower.find("id") != std::string::npos ||
            lower.find("payload") != std::string::npos) {
            return "std::string";
        }
        if (lower.find("priority") != std::string::npos ||
            lower.find("count") != std::string::npos ||
            lower.find("size") != std::string::npos ||
            lower.find("index") != std::string::npos) {
            return "int";
        }
        if (method) {
            std::string m = method->name;
            std::transform(m.begin(), m.end(), m.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if ((m.find("enqueue") != std::string::npos ||
                 m.find("push") != std::string::npos) && lower == "item") {
                return "WorkItem";
            }
        }
        return "auto";
    }

    std::string visitInterfaceDeclaration(const ASTNode* node) override {
        auto* iface = static_cast<const InterfaceDeclaration*>(node);
        std::ostringstream oss;
        oss << "class " << iface->name << " {\npublic:\n";
        auto methods = iface->getChildren("methods");
        for (const auto* m : methods) oss << generate(m);
        oss << "};\n";
        return oss.str();
    }

    std::string visitMethodDeclaration(const ASTNode* node) override {
        auto* meth = static_cast<const MethodDeclaration*>(node);
        std::ostringstream oss;
        auto annotations = meth->getChildren("annotations");
        for (const auto* a : annotations) oss << "    " << generate(a) << "\n";
        oss << "    ";
        if (meth->isStatic) oss << "static ";
        if (meth->isVirtual) oss << "virtual ";
        oss << inferReturnType(meth) << " " << meth->name << "(";
        auto params = meth->getChildren("parameters");
        bool firstParam = true;
        for (size_t i = 0; i < params.size(); ++i) {
            auto* param = static_cast<const Parameter*>(params[i]);
            if (!param) continue;
            if (param->name == "self" || param->name == "cls") continue;
            if (!firstParam) oss << ", ";
            firstParam = false;
            oss << inferParameterType(param, meth) << " " << param->name;
        }
        oss << ")";
        if (meth->isOverride) oss << " override";
        auto body = meth->getChildren("body");
        if (body.empty()) {
            oss << " {}\n";
        } else {
            oss << " {\n";
            for (const auto* s : body) oss << "        " << generate(s) << "\n";
            oss << "    }\n";
        }
        return oss.str();
    }

    std::string visitGenericType(const ASTNode* node) override {
        auto* gen = static_cast<const GenericType*>(node);
        std::ostringstream oss;
        oss << gen->baseName << "<";
        auto params = gen->getChildren("typeParameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(params[i]);
        }
        oss << ">";
        return oss.str();
    }

    std::string visitTypeParameter(const ASTNode* node) override {
        auto* tp = static_cast<const TypeParameter*>(node);
        return tp->name;
    }

    std::string visitAsyncFunction(const ASTNode* node) override {
        auto* af = static_cast<const AsyncFunction*>(node);
        std::ostringstream oss;
        auto annotations = af->getChildren("annotations");
        for (const auto* a : annotations) oss << generate(a) << "\n";
        oss << "std::future<void> " << af->name << "(";
        auto params = af->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << visitParameter(static_cast<const Parameter*>(params[i]));
        }
        oss << ") {\n";
        auto body = af->getChildren("body");
        for (const auto* s : body) oss << "    " << generate(s) << "\n";
        oss << "}\n";
        return oss.str();
    }

    std::string visitAwaitExpression(const ASTNode* node) override {
        auto* aw = static_cast<const AwaitExpression*>(node);
        auto* expr = aw->getChild("expression");
        return "co_await " + (expr ? generate(expr) : "/* missing */");
    }

    std::string visitLambdaExpression(const ASTNode* node) override {
        auto* lam = static_cast<const LambdaExpression*>(node);
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < lam->captureList.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << lam->captureList[i];
        }
        oss << "](";
        auto params = lam->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "auto " << static_cast<const Parameter*>(params[i])->name;
        }
        oss << ") { ";
        auto body = lam->getChildren("body");
        for (const auto* s : body) oss << generate(s) << " ";
        oss << "}";
        return oss.str();
    }

    std::string visitDecoratorAnnotation(const ASTNode* node) override {
        auto* dec = static_cast<const DecoratorAnnotation*>(node);
        return "// @" + dec->name;
    }

    std::string visitDerefStrategy(const DerefStrategy* annotation) override {
        // Generate C++-style comment for deref strategy
        if (annotation->strategy == "batched") {
            return "// @deref(batched) - Use batched memory management (smart pointers)";
        } else if (annotation->strategy == "streamed") {
            return "// @deref(streamed) - Use streamed memory management (RAII)";
        } else if (annotation->strategy == "manual") {
            return "// @deref(manual) - Use manual memory management (new/delete)";
        } else {
            return "// @deref(" + annotation->strategy + ") - Memory management strategy";
        }
    }

    std::string visitOptimizationLock(const OptimizationLock* annotation) override {
        return "// @lock(" + annotation->lockedBy + ") - Optimization locked: " + annotation->lockReason;
    }

    std::string visitLangSpecific(const LangSpecific* annotation) override {
        return "// @lang_specific(" + annotation->language + ", " + annotation->idiomType + ")";
    }

    std::string visitDeallocateAnnotation(const DeallocateAnnotation* annotation) override {
        return "// @dealloc(" + annotation->strategy + ") - Memory deallocation strategy";
    }

    std::string visitLifetimeAnnotation(const LifetimeAnnotation* annotation) override {
        return "// @lifetime(" + annotation->strategy + ") - Object lifetime management";
    }

    std::string visitReclaimAnnotation(const ReclaimAnnotation* annotation) override {
        return "// @reclaim(" + annotation->strategy + ") - Memory reclamation strategy";
    }

    std::string visitOwnerAnnotation(const OwnerAnnotation* annotation) override {
        return "// @owner(" + annotation->strategy + ") - Ownership management strategy";
    }

    std::string visitAllocateAnnotation(const AllocateAnnotation* annotation) override {
        return "// @allocate(" + annotation->strategy + ") - Memory allocation strategy";
    }

    std::string visitHotColdAnnotation(const HotColdAnnotation* annotation) override {
        if (annotation->hint == "Hot")
            return "__attribute__((hot))";
        if (annotation->hint == "Cold")
            return "__attribute__((cold))";
        return "// @hotcold(" + annotation->hint + ")";
    }

    std::string visitInlineAnnotation(const InlineAnnotation* annotation) override {
        if (annotation->mode == "Always")
            return "[[gnu::always_inline]] inline";
        if (annotation->mode == "Never")
            return "__attribute__((noinline))";
        return "inline";
    }

    std::string visitPureAnnotation(const PureAnnotation*) override {
        return "[[nodiscard]]";
    }

    std::string visitConstExprAnnotation(const ConstExprAnnotation*) override {
        return "constexpr";
    }

    // --- Preprocessor/Enum/Namespace visitors (Step 340) ---

    std::string visitIncludeDirective(const ASTNode* node) override {
        auto* inc = static_cast<const IncludeDirective*>(node);
        if (inc->isSystem)
            return "#include <" + inc->path + ">";
        return "#include \"" + inc->path + "\"";
    }

    std::string visitPragmaDirective(const ASTNode* node) override {
        auto* prag = static_cast<const PragmaDirective*>(node);
        return "#pragma " + prag->directive;
    }

    std::string visitMacroDefinition(const ASTNode* node) override {
        auto* mac = static_cast<const MacroDefinition*>(node);
        std::ostringstream oss;
        oss << "#define " << mac->name;
        if (mac->isFunctionLike) {
            oss << "(";
            for (size_t i = 0; i < mac->parameters.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << mac->parameters[i];
            }
            oss << ")";
        }
        if (!mac->body.empty()) oss << " " << mac->body;
        return oss.str();
    }

    std::string visitEnumDeclaration(const ASTNode* node) override {
        auto* e = static_cast<const EnumDeclaration*>(node);
        std::ostringstream oss;
        if (e->isScoped)
            oss << "enum class " << e->name;
        else
            oss << "enum " << e->name;
        if (!e->underlyingType.empty())
            oss << " : " << e->underlyingType;
        oss << " {\n";
        auto members = e->getChildren("members");
        for (size_t i = 0; i < members.size(); ++i) {
            auto* m = static_cast<const EnumMember*>(members[i]);
            oss << "    " << m->name;
            if (!m->value.empty()) oss << " = " << m->value;
            if (i + 1 < members.size()) oss << ",";
            oss << "\n";
        }
        oss << "};\n";
        return oss.str();
    }

    std::string visitNamespaceDeclaration(const ASTNode* node) override {
        auto* ns = static_cast<const NamespaceDeclaration*>(node);
        std::ostringstream oss;
        if (ns->name.empty())
            oss << "namespace {\n";
        else
            oss << "namespace " << ns->name << " {\n";
        auto body = ns->getChildren("body");
        for (const auto* child : body)
            oss << "    " << generate(child) << "\n";
        oss << "}\n";
        return oss.str();
    }

    std::string visitTypeAlias(const ASTNode* node) override {
        auto* ta = static_cast<const TypeAlias*>(node);
        if (ta->isUsing)
            return "using " + ta->aliasName + " = " + ta->targetType + ";";
        return "typedef " + ta->targetType + " " + ta->aliasName + ";";
    }

private:
    // Check enclosing function's memory annotations to determine smart-pointer wrapper
    std::string getMemoryTypeWrapper(const Variable* variable) const {
        const ASTNode* cur = variable->parent;
        while (cur && cur->conceptType != "Function") {
            cur = cur->parent;
        }
        if (!cur) return "";

        for (auto* anno : cur->getChildren("annotations")) {
            if (anno->conceptType == "ReclaimAnnotation") {
                auto* ra = static_cast<const ReclaimAnnotation*>(anno);
                if (ra->strategy == "Tracing" || ra->strategy == "Cycle")
                    return "std::shared_ptr";
            }
            if (anno->conceptType == "LifetimeAnnotation") {
                auto* la = static_cast<const LifetimeAnnotation*>(anno);
                if (la->strategy == "RAII")
                    return "std::unique_ptr";
            }
            if (anno->conceptType == "OwnerAnnotation") {
                auto* oa = static_cast<const OwnerAnnotation*>(anno);
                if (oa->strategy == "Shared_ARC") return "std::shared_ptr";
                if (oa->strategy == "Single") return "std::unique_ptr";
            }
        }
        return "";
    }
};
