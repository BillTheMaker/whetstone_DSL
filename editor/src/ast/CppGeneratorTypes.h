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
