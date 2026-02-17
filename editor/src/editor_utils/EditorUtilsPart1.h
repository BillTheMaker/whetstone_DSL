static bool annotationInfo(const ASTNode* anno, ImU32& color, std::string& label) {
    if (!anno) return false;
    if (anno->conceptType == "ReclaimAnnotation") {
        auto* a = static_cast<const ReclaimAnnotation*>(anno);
        if (a->strategy == "Tracing") {
            color = IM_COL32(80, 140, 220, 255);
            label = "@Reclaim(Tracing)";
            return true;
        }
    } else if (anno->conceptType == "OwnerAnnotation") {
        auto* a = static_cast<const OwnerAnnotation*>(anno);
        if (a->strategy == "Single") {
            color = IM_COL32(220, 160, 60, 255);
            label = "@Owner(Single)";
            return true;
        }
    } else if (anno->conceptType == "DeallocateAnnotation") {
        auto* a = static_cast<const DeallocateAnnotation*>(anno);
        if (a->strategy == "Explicit") {
            color = IM_COL32(220, 80, 80, 255);
            label = "@Deallocate(Explicit)";
            return true;
        }
    } else if (anno->conceptType == "LifetimeAnnotation") {
        auto* a = static_cast<const LifetimeAnnotation*>(anno);
        if (a->strategy == "RAII") {
            color = IM_COL32(80, 180, 100, 255);
            label = "@Lifetime(RAII)";
            return true;
        }
    } else if (anno->conceptType == "AllocateAnnotation") {
        auto* a = static_cast<const AllocateAnnotation*>(anno);
        if (a->strategy == "Static") {
            color = IM_COL32(160, 160, 160, 255);
            label = "@Allocate(Static)";
            return true;
        }
    }
    return false;
}

static std::string annotationLabel(const ASTNode* anno) {
    if (!anno) return "";
    if (anno->conceptType == "ReclaimAnnotation") {
        auto* a = static_cast<const ReclaimAnnotation*>(anno);
        return "@Reclaim(" + a->strategy + ")";
    }
    if (anno->conceptType == "OwnerAnnotation") {
        auto* a = static_cast<const OwnerAnnotation*>(anno);
        return "@Owner(" + a->strategy + ")";
    }
    if (anno->conceptType == "DeallocateAnnotation") {
        auto* a = static_cast<const DeallocateAnnotation*>(anno);
        return "@Deallocate(" + a->strategy + ")";
    }
    if (anno->conceptType == "LifetimeAnnotation") {
        auto* a = static_cast<const LifetimeAnnotation*>(anno);
        return "@Lifetime(" + a->strategy + ")";
    }
    if (anno->conceptType == "AllocateAnnotation") {
        auto* a = static_cast<const AllocateAnnotation*>(anno);
        return "@Allocate(" + a->strategy + ")";
    }
    return anno->conceptType;
}

static std::string nodeDisplayName(const ASTNode* node) {
    if (!node) return "";
    if (node->conceptType == "Function") return static_cast<const Function*>(node)->name;
    if (node->conceptType == "Variable") return static_cast<const Variable*>(node)->name;
    return node->conceptType;
}

static std::string nextAnnotationId() {
    static int counter = 0;
    return "anno_ui_" + std::to_string(counter++);
}

static Annotation* createAnnotationNode(const std::string& type, const std::string& strategy) {
    if (type == "ReclaimAnnotation") {
        auto* a = new ReclaimAnnotation(nextAnnotationId(), strategy);
        return a;
    }
    if (type == "OwnerAnnotation") {
        auto* a = new OwnerAnnotation(nextAnnotationId(), strategy);
        return a;
    }
    if (type == "DeallocateAnnotation") {
        auto* a = new DeallocateAnnotation(nextAnnotationId(), strategy);
        return a;
    }
    if (type == "LifetimeAnnotation") {
        auto* a = new LifetimeAnnotation(nextAnnotationId(), strategy);
        return a;
    }
    if (type == "AllocateAnnotation") {
        auto* a = new AllocateAnnotation(nextAnnotationId(), strategy);
        return a;
    }
    return nullptr;
}

static int spanScore(const ASTNode* node) {
    if (!node || !node->hasSpan()) return INT_MAX;
    int lines = node->spanEndLine - node->spanStartLine;
    int cols = node->spanEndCol - node->spanStartCol;
    return lines * 10000 + cols;
}

static bool spanContains(const ASTNode* node, int line, int col) {
    if (!node || !node->hasSpan()) return false;
    if (line < node->spanStartLine || line > node->spanEndLine) return false;
    if (line == node->spanStartLine && col < node->spanStartCol) return false;
    if (line == node->spanEndLine && col > node->spanEndCol) return false;
    return true;
}

static ASTNode* findNodeAtPosition(ASTNode* node, int line, int col) {
    if (!node) return nullptr;
    ASTNode* best = nullptr;
    if (spanContains(node, line, col)) best = node;
    for (auto* child : node->allChildren()) {
        ASTNode* cand = findNodeAtPosition(child, line, col);
        if (cand) {
            if (!best || spanScore(cand) < spanScore(best)) best = cand;
        }
    }
    return best;
}

static ASTNode* findAnnotationTarget(ASTNode* node, int line) {
    if (!node) return nullptr;
    ASTNode* best = nullptr;
    if (node->hasSpan() && line >= node->spanStartLine && line <= node->spanEndLine) {
        if (node->conceptType == "Function" || node->conceptType == "Variable") {
            best = node;
        }
    }
    for (auto* child : node->allChildren()) {
        ASTNode* cand = findAnnotationTarget(child, line);
        if (cand) {
            if (!best || spanScore(cand) < spanScore(best)) best = cand;
        }
    }
    return best;
}

static ASTNode* findNodeById(ASTNode* node, const std::string& id) {
    if (!node) return nullptr;
    if (node->id == id) return node;
    for (auto* child : node->allChildren()) {
        if (auto* found = findNodeById(child, id)) return found;
    }
    return nullptr;
}

static void collectAnnotationMarkers(const ASTNode* node, std::vector<AnnotationMarker>& out) {
    if (!node) return;
    if (node->hasSpan()) {
        for (const auto* anno : node->getChildren("annotations")) {
            ImU32 color = 0;
            std::string label;
            if (annotationInfo(anno, color, label)) {
                AnnotationMarker marker;
                marker.line = node->spanStartLine;
                marker.color = color;
                marker.message = label;
                out.push_back(std::move(marker));
            }
        }
    }
    for (auto* child : node->allChildren()) {
        collectAnnotationMarkers(child, out);
    }
}

struct OutlineItem {
    std::string name;
    std::string kind;
    int line = -1;
    int col = -1;
    std::vector<OutlineItem> children;
};

static OutlineItem makeOutlineItem(const std::string& name,
                                   const std::string& kind,
                                   const ASTNode* node) {
    OutlineItem item;
    item.name = name;
    item.kind = kind;
    if (node && node->hasSpan()) {
        item.line = node->spanStartLine;
        item.col = node->spanStartCol;
    }
    return item;
}

static void collectOutlineFromFunction(const Function* fn, std::vector<OutlineItem>& out) {
    OutlineItem item = makeOutlineItem(fn->name, "Function", fn);
    for (auto* p : fn->getChildren("parameters")) {
        if (p->conceptType == "Parameter") {
            auto* param = static_cast<const Parameter*>(p);
            item.children.push_back(makeOutlineItem(param->name, "Parameter", param));
        }
    }
    for (auto* child : fn->getChildren("body")) {
        if (child->conceptType == "Variable") {
            auto* var = static_cast<const Variable*>(child);
            item.children.push_back(makeOutlineItem(var->name, "Variable", var));
        }
    }
    out.push_back(std::move(item));
}

static void collectOutlineFromAST(const Module* mod, std::vector<OutlineItem>& out) {
    if (!mod) return;
    for (auto* child : mod->getChildren("variables")) {
        if (child->conceptType == "Variable") {
            auto* var = static_cast<const Variable*>(child);
            out.push_back(makeOutlineItem(var->name, "Variable", var));
        }
    }
    for (auto* child : mod->getChildren("functions")) {
        if (child->conceptType == "Function") {
            auto* fn = static_cast<const Function*>(child);
            collectOutlineFromFunction(fn, out);
        }
    }
}

// trimCopy is in StringUtils.h

static std::string toLowerCopy(const std::string& input) {
    std::string out = input;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return out;
}

static bool containsCaseInsensitive(const std::string& text, const std::string& filter) {
    if (filter.empty()) return true;
    std::string hay = toLowerCopy(text);
    return hay.find(filter) != std::string::npos;
}

static bool outlineMatchesFilter(const OutlineItem& item, const std::string& filter) {
    if (filter.empty()) return true;
    if (containsCaseInsensitive(item.name, filter)) return true;
    for (const auto& child : item.children) {
        if (outlineMatchesFilter(child, filter)) return true;
    }
    return false;
}

static const char* lspSymbolKindLabel(int kind) {
    switch (kind) {
        case 1: return "File";
        case 2: return "Module";
        case 5: return "Class";
        case 6: return "Method";
        case 12: return "Function";
        case 13: return "Variable";
        case 14: return "Constant";
        case 19: return "String";
        case 23: return "Struct";
        default: return "Symbol";
    }
}

static bool lspSymbolMatchesFilter(const LSPClient::DocumentSymbol& sym, const std::string& filter) {
    if (filter.empty()) return true;
    if (containsCaseInsensitive(sym.name, filter)) return true;
    for (const auto& child : sym.children) {
        if (lspSymbolMatchesFilter(child, filter)) return true;
    }
    return false;
}
