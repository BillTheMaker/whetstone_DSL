#pragma once

// Step 451: Type System Translation — maps type systems across languages
// with annotation-guided safety preservation.

#include <string>
#include <vector>

enum class TypeSafety { Preserved, Widened, Narrowed, Lossy };

struct TypeTranslation {
    std::string sourceType;
    std::string sourceLanguage;
    std::string targetType;
    std::string targetLanguage;
    TypeSafety safety = TypeSafety::Preserved;
    std::string annotation;    // e.g. @Risk(medium) for lossy
    std::string note;
};

struct TypeTranslationResult {
    std::string sourceLanguage;
    std::string targetLanguage;
    std::vector<TypeTranslation> translations;

    bool hasTranslation(const std::string& sourceType) const {
        for (const auto& t : translations)
            if (t.sourceType == sourceType) return true;
        return false;
    }

    TypeTranslation getTranslation(const std::string& sourceType) const {
        for (const auto& t : translations)
            if (t.sourceType == sourceType) return t;
        return {};
    }

    int countLossy() const {
        int n = 0;
        for (const auto& t : translations)
            if (t.safety == TypeSafety::Lossy) ++n;
        return n;
    }
};

class TypeSystemTranslator {
public:
    static TypeTranslationResult translate(const std::vector<std::string>& sourceTypes,
                                            const std::string& srcLang,
                                            const std::string& tgtLang) {
        TypeTranslationResult result;
        result.sourceLanguage = srcLang;
        result.targetLanguage = tgtLang;

        for (const auto& st : sourceTypes) {
            auto tt = mapType(st, srcLang, tgtLang);
            result.translations.push_back(std::move(tt));
        }
        return result;
    }

private:
    static TypeTranslation mapType(const std::string& type,
                                    const std::string& src,
                                    const std::string& tgt) {
        TypeTranslation t;
        t.sourceType = type;
        t.sourceLanguage = src;
        t.targetLanguage = tgt;

        // Nullable types
        if (type == "Optional" || type == "Optional<T>" || type == "T?") {
            if (tgt == "rust") { t.targetType = "Option<T>"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "python") { t.targetType = "Optional[T]"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "java") { t.targetType = "Optional<T>"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "c" || tgt == "cpp") { t.targetType = "T*"; t.safety = TypeSafety::Lossy; t.annotation = "@Risk(medium)"; }
            else { t.targetType = "nullable T"; t.safety = TypeSafety::Widened; }
        }
        // Generic collections
        else if (type == "List<T>" || type == "Vec<T>" || type == "list") {
            if (tgt == "rust") { t.targetType = "Vec<T>"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "python") { t.targetType = "list[T]"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "java") { t.targetType = "List<T>"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "c") { t.targetType = "void*"; t.safety = TypeSafety::Lossy; t.annotation = "@Risk(high)"; t.note = "Generics lost in C"; }
            else if (tgt == "go") { t.targetType = "[]T"; t.safety = TypeSafety::Preserved; }
            else { t.targetType = "array"; t.safety = TypeSafety::Widened; }
        }
        // Map/Dict
        else if (type == "Map<K,V>" || type == "HashMap<K,V>" || type == "dict") {
            if (tgt == "rust") { t.targetType = "HashMap<K,V>"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "python") { t.targetType = "dict[K,V]"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "java") { t.targetType = "Map<K,V>"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "go") { t.targetType = "map[K]V"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "c") { t.targetType = "void*"; t.safety = TypeSafety::Lossy; t.annotation = "@Risk(high)"; }
            else { t.targetType = "map"; t.safety = TypeSafety::Widened; }
        }
        // Enum types
        else if (type == "enum" || type == "Enum") {
            if (tgt == "rust") { t.targetType = "enum"; t.safety = TypeSafety::Preserved; t.note = "Rust enum is more expressive (sum type)"; }
            else if (tgt == "java") { t.targetType = "enum"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "python") { t.targetType = "Enum"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "c") { t.targetType = "enum/constants"; t.safety = TypeSafety::Narrowed; t.note = "C enum has no methods"; }
            else if (tgt == "go") { t.targetType = "const iota"; t.safety = TypeSafety::Narrowed; }
            else { t.targetType = "enum"; t.safety = TypeSafety::Preserved; }
        }
        // Class hierarchy → traits/interfaces
        else if (type == "class" || type == "abstract class") {
            if (tgt == "rust") { t.targetType = "trait + struct"; t.safety = TypeSafety::Preserved; t.note = "Composition over inheritance"; }
            else if (tgt == "java") { t.targetType = "interface + class"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "go") { t.targetType = "interface + struct"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "c") { t.targetType = "struct + vtable"; t.safety = TypeSafety::Lossy; t.annotation = "@Risk(medium)"; }
            else if (tgt == "python") { t.targetType = "class (ABC)"; t.safety = TypeSafety::Preserved; }
            else { t.targetType = "class"; t.safety = TypeSafety::Preserved; }
        }
        // String types
        else if (type == "String" || type == "string" || type == "str") {
            if (tgt == "rust") { t.targetType = "String"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "python") { t.targetType = "str"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "java") { t.targetType = "String"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "c") { t.targetType = "char*"; t.safety = TypeSafety::Lossy; t.annotation = "@Risk(medium)"; t.note = "No length tracking"; }
            else if (tgt == "go") { t.targetType = "string"; t.safety = TypeSafety::Preserved; }
            else { t.targetType = "string"; t.safety = TypeSafety::Preserved; }
        }
        // Primitive int
        else if (type == "int" || type == "i32" || type == "int32") {
            if (tgt == "rust") { t.targetType = "i32"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "python") { t.targetType = "int"; t.safety = TypeSafety::Widened; t.note = "Python int is arbitrary precision"; }
            else if (tgt == "java") { t.targetType = "int"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "c" || tgt == "cpp") { t.targetType = "int"; t.safety = TypeSafety::Preserved; }
            else if (tgt == "go") { t.targetType = "int32"; t.safety = TypeSafety::Preserved; }
            else { t.targetType = "int"; t.safety = TypeSafety::Preserved; }
        }
        else {
            t.targetType = type;
            t.safety = TypeSafety::Preserved;
            t.note = "Unknown type — preserved as-is";
        }
        return t;
    }
};
