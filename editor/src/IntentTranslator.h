#pragma once

// Step 449: Intent-Driven Translation — reads @Intent annotations and selects
// target-idiomatic implementations rather than line-by-line translation.

#include <algorithm>
#include <string>
#include <vector>

struct IntentMapping {
    std::string intent;           // e.g. "sort list ascending"
    std::string sourceLanguage;
    std::string targetLanguage;
    std::string sourcePattern;    // how it looks in source
    std::string targetPattern;    // idiomatic target equivalent
    float confidence = 0.0f;     // 0.0-1.0
    bool isIdiomatic = false;    // true if target uses idiomatic construct
};

struct TranslationChoice {
    std::string functionName;
    std::string intent;
    std::string sourceLanguage;
    std::string targetLanguage;
    std::string sourceCode;
    std::string targetCode;
    float confidence = 0.0f;
    bool isIdiomatic = false;
    bool needsReview = false;
    std::string reviewReason;
};

struct IntentTranslationResult {
    std::string sourceLanguage;
    std::string targetLanguage;
    std::vector<TranslationChoice> translations;

    bool hasTranslationFor(const std::string& funcName) const {
        for (const auto& t : translations)
            if (t.functionName == funcName) return true;
        return false;
    }

    TranslationChoice getTranslation(const std::string& funcName) const {
        for (const auto& t : translations)
            if (t.functionName == funcName) return t;
        return {};
    }

    int countIdiomatic() const {
        int n = 0;
        for (const auto& t : translations)
            if (t.isIdiomatic) ++n;
        return n;
    }

    int countNeedsReview() const {
        int n = 0;
        for (const auto& t : translations)
            if (t.needsReview) ++n;
        return n;
    }
};

struct FunctionIntent {
    std::string name;
    std::string intent;   // @Intent value
    std::string source;   // source code body
};

class IntentTranslator {
public:
    static IntentTranslationResult translate(
        const std::vector<FunctionIntent>& functions,
        const std::string& sourceLanguage,
        const std::string& targetLanguage) {

        IntentTranslationResult result;
        result.sourceLanguage = sourceLanguage;
        result.targetLanguage = targetLanguage;

        for (const auto& func : functions) {
            TranslationChoice choice;
            choice.functionName = func.name;
            choice.intent = func.intent;
            choice.sourceLanguage = sourceLanguage;
            choice.targetLanguage = targetLanguage;
            choice.sourceCode = func.source;

            if (!func.intent.empty()) {
                auto mapping = findIdiomaticMapping(func.intent, sourceLanguage, targetLanguage);
                if (mapping.confidence > 0.0f) {
                    choice.targetCode = mapping.targetPattern;
                    choice.confidence = mapping.confidence;
                    choice.isIdiomatic = mapping.isIdiomatic;
                    choice.needsReview = false;
                } else {
                    // Intent present but no idiomatic mapping — structural translation
                    choice.targetCode = structuralTranslate(func.source, sourceLanguage, targetLanguage);
                    choice.confidence = 0.70f;
                    choice.isIdiomatic = false;
                    choice.needsReview = false;
                }
            } else {
                // No intent — structural translation, lower confidence
                choice.targetCode = structuralTranslate(func.source, sourceLanguage, targetLanguage);
                choice.confidence = 0.50f;
                choice.isIdiomatic = false;
                choice.needsReview = true;
                choice.reviewReason = "No @Intent annotation — structural translation only";
            }

            result.translations.push_back(std::move(choice));
        }

        return result;
    }

    // Public for testing
    static IntentMapping findIdiomaticMapping(const std::string& intent,
                                               const std::string& src,
                                               const std::string& tgt) {
        std::string normIntent = toLower(intent);

        // Sort intents
        if (normIntent.find("sort") != std::string::npos) {
            return makeSortMapping(normIntent, src, tgt);
        }
        // Find/lookup intents
        if (normIntent.find("find") != std::string::npos ||
            normIntent.find("lookup") != std::string::npos ||
            normIntent.find("search") != std::string::npos) {
            return makeFindMapping(normIntent, src, tgt);
        }
        // Filter intents
        if (normIntent.find("filter") != std::string::npos) {
            return makeFilterMapping(normIntent, src, tgt);
        }
        // Map/transform intents
        if (normIntent.find("map") != std::string::npos ||
            normIntent.find("transform") != std::string::npos) {
            return makeMapMapping(normIntent, src, tgt);
        }
        // Accumulate/reduce intents
        if (normIntent.find("sum") != std::string::npos ||
            normIntent.find("reduce") != std::string::npos ||
            normIntent.find("accumulate") != std::string::npos) {
            return makeReduceMapping(normIntent, src, tgt);
        }
        // IO intents
        if (normIntent.find("read file") != std::string::npos ||
            normIntent.find("read input") != std::string::npos) {
            return makeReadMapping(normIntent, src, tgt);
        }

        return {"", src, tgt, "", "", 0.0f, false};
    }

private:
    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        return s;
    }

    static IntentMapping makeSortMapping(const std::string& intent,
                                          const std::string& src,
                                          const std::string& tgt) {
        std::string pattern;
        if (tgt == "python") pattern = "sorted(items)";
        else if (tgt == "rust") pattern = "items.sort()";
        else if (tgt == "java") pattern = "Collections.sort(items)";
        else if (tgt == "go") pattern = "sort.Slice(items, func(i, j int) bool { return items[i] < items[j] })";
        else if (tgt == "cpp" || tgt == "c++") pattern = "std::sort(items.begin(), items.end())";
        else if (tgt == "javascript" || tgt == "js") pattern = "items.sort((a, b) => a - b)";
        else if (tgt == "sql") pattern = "ORDER BY column ASC";
        else pattern = "sort(items)";
        return {intent, src, tgt, "sort implementation", pattern, 0.95f, true};
    }

    static IntentMapping makeFindMapping(const std::string& intent,
                                          const std::string& src,
                                          const std::string& tgt) {
        std::string pattern;
        if (tgt == "python") pattern = "items.get(key) or next((x for x in items if pred(x)), None)";
        else if (tgt == "rust") pattern = "items.iter().find(|x| pred(x))";
        else if (tgt == "java") pattern = "items.stream().filter(pred).findFirst()";
        else if (tgt == "sql") pattern = "SELECT * FROM table WHERE key = value";
        else if (tgt == "cpp" || tgt == "c++") pattern = "std::find_if(items.begin(), items.end(), pred)";
        else if (tgt == "go") pattern = "for _, v := range items { if pred(v) { return v } }";
        else pattern = "find(items, pred)";
        return {intent, src, tgt, "find/lookup", pattern, 0.90f, true};
    }

    static IntentMapping makeFilterMapping(const std::string& intent,
                                            const std::string& src,
                                            const std::string& tgt) {
        std::string pattern;
        if (tgt == "python") pattern = "[x for x in items if pred(x)]";
        else if (tgt == "rust") pattern = "items.iter().filter(|x| pred(x)).collect()";
        else if (tgt == "java") pattern = "items.stream().filter(pred).collect(Collectors.toList())";
        else if (tgt == "sql") pattern = "SELECT * FROM table WHERE condition";
        else if (tgt == "cpp" || tgt == "c++") pattern = "std::copy_if(items.begin(), items.end(), std::back_inserter(out), pred)";
        else pattern = "filter(items, pred)";
        return {intent, src, tgt, "filter collection", pattern, 0.90f, true};
    }

    static IntentMapping makeMapMapping(const std::string& intent,
                                         const std::string& src,
                                         const std::string& tgt) {
        std::string pattern;
        if (tgt == "python") pattern = "[f(x) for x in items]";
        else if (tgt == "rust") pattern = "items.iter().map(|x| f(x)).collect()";
        else if (tgt == "java") pattern = "items.stream().map(f).collect(Collectors.toList())";
        else if (tgt == "cpp" || tgt == "c++") pattern = "std::transform(items.begin(), items.end(), out.begin(), f)";
        else pattern = "map(items, f)";
        return {intent, src, tgt, "map/transform", pattern, 0.90f, true};
    }

    static IntentMapping makeReduceMapping(const std::string& intent,
                                            const std::string& src,
                                            const std::string& tgt) {
        std::string pattern;
        if (tgt == "python") pattern = "sum(items) or functools.reduce(f, items)";
        else if (tgt == "rust") pattern = "items.iter().fold(init, |acc, x| acc + x)";
        else if (tgt == "java") pattern = "items.stream().reduce(init, Integer::sum)";
        else if (tgt == "sql") pattern = "SELECT SUM(column) FROM table";
        else if (tgt == "cpp" || tgt == "c++") pattern = "std::accumulate(items.begin(), items.end(), init)";
        else pattern = "reduce(items, f, init)";
        return {intent, src, tgt, "accumulate/reduce", pattern, 0.90f, true};
    }

    static IntentMapping makeReadMapping(const std::string& intent,
                                          const std::string& src,
                                          const std::string& tgt) {
        std::string pattern;
        if (tgt == "python") pattern = "with open(path) as f: content = f.read()";
        else if (tgt == "rust") pattern = "std::fs::read_to_string(path)?";
        else if (tgt == "java") pattern = "Files.readString(Path.of(path))";
        else if (tgt == "go") pattern = "os.ReadFile(path)";
        else if (tgt == "cpp" || tgt == "c++") pattern = "std::ifstream ifs(path); std::string content((std::istreambuf_iterator<char>(ifs)), {})";
        else pattern = "read_file(path)";
        return {intent, src, tgt, "read file/input", pattern, 0.85f, true};
    }

    static std::string structuralTranslate(const std::string& source,
                                            const std::string& srcLang,
                                            const std::string& tgtLang) {
        // Placeholder: in a real transpiler this does syntax-level translation.
        // For now, return a comment indicating structural translation needed.
        return "// TODO: structural translation from " + srcLang + " to " + tgtLang +
               "\n// Original: " + (source.size() > 80 ? source.substr(0, 80) + "..." : source);
    }
};
