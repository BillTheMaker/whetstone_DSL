#pragma once

// Step 453: Memory Model Translation — translates ownership/lifecycle across
// memory models with safety tracking annotations.

#include <string>
#include <vector>

enum class OwnershipModel { Manual, RAII, GC, BorrowChecked, ARC };
enum class SafetyDelta { Gained, Lost, Neutral };

struct MemoryTranslation {
    std::string pattern;          // e.g. "@Owner(Unique)", "@Owner(Shared_ARC)"
    OwnershipModel sourceModel;
    OwnershipModel targetModel;
    std::string sourceLanguage;
    std::string targetLanguage;
    std::string sourceCode;
    std::string targetCode;
    SafetyDelta safetyChange = SafetyDelta::Neutral;
    std::string annotation;
    std::string note;
};

struct MemoryAnalysisResult {
    std::string sourceLanguage;
    std::string targetLanguage;
    OwnershipModel sourceModel = OwnershipModel::Manual;
    OwnershipModel targetModel = OwnershipModel::Manual;
    std::vector<MemoryTranslation> translations;

    bool hasPattern(const std::string& p) const {
        for (const auto& t : translations)
            if (t.pattern == p) return true;
        return false;
    }

    int countSafetyGained() const {
        int n = 0;
        for (const auto& t : translations)
            if (t.safetyChange == SafetyDelta::Gained) ++n;
        return n;
    }

    int countSafetyLost() const {
        int n = 0;
        for (const auto& t : translations)
            if (t.safetyChange == SafetyDelta::Lost) ++n;
        return n;
    }
};

class MemoryModelTranslator {
public:
    static MemoryAnalysisResult analyze(const std::string& source,
                                         const std::string& srcLang,
                                         const std::string& tgtLang) {
        MemoryAnalysisResult result;
        result.sourceLanguage = srcLang;
        result.targetLanguage = tgtLang;
        result.sourceModel = detectOwnership(source, srcLang);
        result.targetModel = defaultOwnership(tgtLang);

        if (hasUniqueOwnership(source, srcLang))
            result.translations.push_back(translateUnique(srcLang, tgtLang));
        if (hasSharedOwnership(source, srcLang))
            result.translations.push_back(translateShared(srcLang, tgtLang));
        if (hasManualMemory(source))
            result.translations.push_back(translateManual(srcLang, tgtLang));
        if (hasScopeLifetime(source))
            result.translations.push_back(translateScope(srcLang, tgtLang));

        return result;
    }

private:
    static OwnershipModel detectOwnership(const std::string& src, const std::string& lang) {
        if (lang == "rust") return OwnershipModel::BorrowChecked;
        if (lang == "python" || lang == "java" || lang == "go" || lang == "javascript")
            return OwnershipModel::GC;
        if (src.find("unique_ptr") != std::string::npos ||
            src.find("shared_ptr") != std::string::npos) return OwnershipModel::RAII;
        if (src.find("malloc") != std::string::npos ||
            src.find("free(") != std::string::npos) return OwnershipModel::Manual;
        if (lang == "cpp" || lang == "c++") return OwnershipModel::RAII;
        if (lang == "c") return OwnershipModel::Manual;
        return OwnershipModel::Manual;
    }

    static OwnershipModel defaultOwnership(const std::string& lang) {
        if (lang == "rust") return OwnershipModel::BorrowChecked;
        if (lang == "cpp" || lang == "c++") return OwnershipModel::RAII;
        if (lang == "c") return OwnershipModel::Manual;
        if (lang == "python" || lang == "java" || lang == "go" || lang == "javascript")
            return OwnershipModel::GC;
        return OwnershipModel::Manual;
    }

    static bool hasUniqueOwnership(const std::string& src, const std::string& lang) {
        if (lang == "rust") return src.find("Box<") != std::string::npos || src.find("let mut") != std::string::npos;
        if (lang == "cpp") return src.find("unique_ptr") != std::string::npos;
        return false;
    }

    static bool hasSharedOwnership(const std::string& src, const std::string& lang) {
        if (lang == "rust") return src.find("Arc<") != std::string::npos || src.find("Rc<") != std::string::npos;
        if (lang == "cpp") return src.find("shared_ptr") != std::string::npos;
        return false;
    }

    static bool hasManualMemory(const std::string& src) {
        return src.find("malloc(") != std::string::npos ||
               src.find("calloc(") != std::string::npos ||
               src.find("free(") != std::string::npos ||
               src.find("new ") != std::string::npos;
    }

    static bool hasScopeLifetime(const std::string& src) {
        return src.find("RAII") != std::string::npos ||
               src.find("defer ") != std::string::npos ||
               src.find("try-finally") != std::string::npos ||
               src.find("with ") != std::string::npos ||
               src.find("using (") != std::string::npos;
    }

    static MemoryTranslation translateUnique(const std::string& src, const std::string& tgt) {
        MemoryTranslation t;
        t.pattern = "@Owner(Unique)";
        t.sourceLanguage = src;
        t.targetLanguage = tgt;

        if (tgt == "rust") {
            t.targetCode = "Box::new(value) or owned value";
            t.safetyChange = (src == "c") ? SafetyDelta::Gained : SafetyDelta::Neutral;
            t.annotation = "@Owner(Unique), @Lifetime(owned)";
        } else if (tgt == "cpp" || tgt == "c++") {
            t.targetCode = "std::unique_ptr<T>(new T(...))";
            t.safetyChange = (src == "c") ? SafetyDelta::Gained : SafetyDelta::Neutral;
            t.annotation = "@Owner(Unique)";
        } else if (tgt == "c") {
            t.targetCode = "T* p = malloc(sizeof(T)); /* single owner, must free */";
            t.safetyChange = SafetyDelta::Lost;
            t.annotation = "@Owner(Manual), @Risk(high)";
            t.note = "Unique ownership lost — manual free required";
        } else {
            t.targetCode = "value  // GC handles lifetime";
            t.safetyChange = SafetyDelta::Neutral;
            t.annotation = "@Owner(GC)";
            t.note = "GC language — no explicit ownership needed";
        }
        t.sourceModel = (src == "rust") ? OwnershipModel::BorrowChecked :
                        (src == "cpp") ? OwnershipModel::RAII : OwnershipModel::Manual;
        t.targetModel = defaultOwnership(tgt);
        return t;
    }

    static MemoryTranslation translateShared(const std::string& src, const std::string& tgt) {
        MemoryTranslation t;
        t.pattern = "@Owner(Shared_ARC)";
        t.sourceLanguage = src;
        t.targetLanguage = tgt;

        if (tgt == "rust") {
            t.targetCode = "Arc::new(value) or Rc::new(value)";
            t.safetyChange = SafetyDelta::Neutral;
            t.annotation = "@Owner(Shared_ARC)";
        } else if (tgt == "cpp" || tgt == "c++") {
            t.targetCode = "std::shared_ptr<T>(new T(...))";
            t.safetyChange = SafetyDelta::Neutral;
            t.annotation = "@Owner(Shared_ARC)";
        } else if (tgt == "c") {
            t.targetCode = "T* p = malloc(sizeof(T)); /* manual refcount */";
            t.safetyChange = SafetyDelta::Lost;
            t.annotation = "@Owner(Manual), @Risk(high)";
        } else {
            t.targetCode = "value  // GC handles sharing";
            t.safetyChange = SafetyDelta::Neutral;
            t.annotation = "@Owner(GC)";
        }
        t.sourceModel = (src == "rust") ? OwnershipModel::BorrowChecked : OwnershipModel::RAII;
        t.targetModel = defaultOwnership(tgt);
        return t;
    }

    static MemoryTranslation translateManual(const std::string& src, const std::string& tgt) {
        MemoryTranslation t;
        t.pattern = "@Owner(Manual)";
        t.sourceLanguage = src;
        t.targetLanguage = tgt;
        t.sourceModel = OwnershipModel::Manual;

        if (tgt == "rust") {
            t.targetCode = "let value = T::new(...);  // owned, borrow checker enforces lifetime";
            t.safetyChange = SafetyDelta::Gained;
            t.annotation = "@Owner(BorrowChecked), @Risk(lowered)";
            t.note = "Borrow checker eliminates use-after-free and double-free";
        } else if (tgt == "cpp" || tgt == "c++") {
            t.targetCode = "auto p = std::make_unique<T>(...);";
            t.safetyChange = SafetyDelta::Gained;
            t.annotation = "@Owner(RAII)";
        } else if (tgt == "java" || tgt == "python" || tgt == "go") {
            t.targetCode = "T value = new T(...)  // GC manages lifetime";
            t.safetyChange = SafetyDelta::Gained;
            t.annotation = "@Owner(GC), @Risk(lowered)";
        } else {
            t.targetCode = "malloc/free  // manual";
            t.safetyChange = SafetyDelta::Neutral;
            t.annotation = "@Owner(Manual)";
        }
        t.targetModel = defaultOwnership(tgt);
        return t;
    }

    static MemoryTranslation translateScope(const std::string& src, const std::string& tgt) {
        MemoryTranslation t;
        t.pattern = "@Lifetime(Scope)";
        t.sourceLanguage = src;
        t.targetLanguage = tgt;

        if (tgt == "rust") {
            t.targetCode = "{ let resource = acquire(); /* dropped at end of scope */ }";
            t.safetyChange = SafetyDelta::Neutral;
            t.annotation = "@Lifetime(Scope), @Owner(RAII)";
        } else if (tgt == "cpp" || tgt == "c++") {
            t.targetCode = "{ auto resource = Resource(); /* destructor at scope end */ }";
            t.safetyChange = SafetyDelta::Neutral;
            t.annotation = "@Lifetime(Scope)";
        } else if (tgt == "python") {
            t.targetCode = "with resource_manager() as r: ...";
            t.safetyChange = SafetyDelta::Neutral;
            t.annotation = "@Lifetime(Scope)";
        } else if (tgt == "go") {
            t.targetCode = "defer resource.Close()";
            t.safetyChange = SafetyDelta::Neutral;
            t.annotation = "@Lifetime(deferred)";
        } else if (tgt == "java") {
            t.targetCode = "try (var r = new Resource()) { ... }";
            t.safetyChange = SafetyDelta::Neutral;
            t.annotation = "@Lifetime(try-with-resources)";
        } else {
            t.targetCode = "acquire(); ... release();  // manual scope";
            t.safetyChange = SafetyDelta::Lost;
            t.annotation = "@Lifetime(Manual), @Risk(medium)";
        }
        t.sourceModel = OwnershipModel::RAII;
        t.targetModel = defaultOwnership(tgt);
        return t;
    }
};
