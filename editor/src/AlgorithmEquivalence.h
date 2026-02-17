#pragma once

// Step 450: Algorithm-Level Equivalence — recognizes algorithmic patterns in
// source code and maps them to target language standard library equivalents.

#include <string>
#include <vector>

struct AlgorithmPattern {
    std::string name;           // e.g. "linear_search", "bubble_sort"
    std::string category;       // e.g. "search", "sort", "transform"
    std::string description;
    float matchConfidence = 0.0f;
};

struct AlgorithmEquivalent {
    std::string patternName;
    std::string sourceLanguage;
    std::string targetLanguage;
    std::string sourceCode;     // what it looks like in source
    std::string targetCode;     // idiomatic target equivalent
    bool usesStdLib = false;    // true if target uses standard library
    std::string complexity;     // @Complexity value if present
};

struct AlgorithmAnalysisResult {
    std::vector<AlgorithmPattern> detectedPatterns;
    std::vector<AlgorithmEquivalent> equivalents;

    bool hasPattern(const std::string& name) const {
        for (const auto& p : detectedPatterns)
            if (p.name == name) return true;
        return false;
    }

    AlgorithmEquivalent equivalentFor(const std::string& patternName) const {
        for (const auto& e : equivalents)
            if (e.patternName == patternName) return e;
        return {};
    }

    int countStdLib() const {
        int n = 0;
        for (const auto& e : equivalents)
            if (e.usesStdLib) ++n;
        return n;
    }
};

class AlgorithmRecognizer {
public:
    static AlgorithmAnalysisResult analyze(const std::string& source,
                                            const std::string& sourceLanguage,
                                            const std::string& targetLanguage,
                                            const std::string& complexityHint = "") {
        AlgorithmAnalysisResult result;

        // Detect patterns
        detectSortPatterns(source, result);
        detectSearchPatterns(source, result);
        detectMapFilterReduce(source, result);
        detectBuilderPattern(source, result);
        detectIterationPatterns(source, result);

        // Generate equivalents for detected patterns
        for (const auto& p : result.detectedPatterns) {
            auto eq = mapToTarget(p, sourceLanguage, targetLanguage, complexityHint);
            if (!eq.targetCode.empty())
                result.equivalents.push_back(std::move(eq));
        }

        return result;
    }

    static std::vector<std::string> supportedPatterns() {
        return {
            "bubble_sort", "insertion_sort", "selection_sort", "manual_sort",
            "linear_search", "binary_search", "key_lookup",
            "manual_map", "manual_filter", "manual_reduce",
            "builder_pattern", "factory_pattern",
            "counting_loop", "accumulator_loop", "iterator_loop"
        };
    }

private:
    static void detectSortPatterns(const std::string& src, AlgorithmAnalysisResult& r) {
        // Bubble sort: nested loops with swap
        if ((src.find("for") != std::string::npos || src.find("while") != std::string::npos) &&
            src.find("swap") != std::string::npos) {
            r.detectedPatterns.push_back({"bubble_sort", "sort", "bubble sort with swap", 0.80f});
        }
        // Any sort-like pattern with comparison and rearrangement
        if (src.find("sort") != std::string::npos && src.find("(") != std::string::npos) {
            r.detectedPatterns.push_back({"manual_sort", "sort", "manual sort implementation", 0.70f});
        }
    }

    static void detectSearchPatterns(const std::string& src, AlgorithmAnalysisResult& r) {
        // Linear search: loop with equality check and return/break
        if (src.find("for") != std::string::npos &&
            (src.find("==") != std::string::npos || src.find("equals") != std::string::npos) &&
            (src.find("return") != std::string::npos || src.find("break") != std::string::npos)) {
            r.detectedPatterns.push_back({"linear_search", "search", "linear search through collection", 0.75f});
        }
        // Binary search: midpoint calculation
        if (src.find("mid") != std::string::npos &&
            (src.find("low") != std::string::npos || src.find("left") != std::string::npos) &&
            (src.find("high") != std::string::npos || src.find("right") != std::string::npos)) {
            r.detectedPatterns.push_back({"binary_search", "search", "binary search", 0.85f});
        }
        // Key lookup: dictionary/map access
        if (src.find("[key]") != std::string::npos || src.find(".get(") != std::string::npos) {
            r.detectedPatterns.push_back({"key_lookup", "search", "key-based lookup", 0.90f});
        }
    }

    static void detectMapFilterReduce(const std::string& src, AlgorithmAnalysisResult& r) {
        // Manual map: loop building new collection by transforming elements
        if (src.find("for") != std::string::npos &&
            (src.find("append") != std::string::npos || src.find("push") != std::string::npos ||
             src.find("push_back") != std::string::npos)) {
            r.detectedPatterns.push_back({"manual_map", "transform", "manual map/transform loop", 0.70f});
        }
        // Manual filter: loop with condition and selective append
        if (src.find("for") != std::string::npos &&
            src.find("if") != std::string::npos &&
            (src.find("append") != std::string::npos || src.find("push") != std::string::npos)) {
            r.detectedPatterns.push_back({"manual_filter", "transform", "manual filter loop", 0.75f});
        }
        // Manual reduce: accumulator pattern
        if (src.find("for") != std::string::npos &&
            (src.find("+=") != std::string::npos || src.find("sum") != std::string::npos ||
             src.find("total") != std::string::npos || src.find("acc") != std::string::npos)) {
            r.detectedPatterns.push_back({"manual_reduce", "transform", "manual accumulator/reduce", 0.75f});
        }
    }

    static void detectBuilderPattern(const std::string& src, AlgorithmAnalysisResult& r) {
        // Builder: method chaining with .set or .with
        if ((src.find(".set") != std::string::npos || src.find(".with") != std::string::npos) &&
            src.find(".build()") != std::string::npos) {
            r.detectedPatterns.push_back({"builder_pattern", "construction", "builder pattern", 0.90f});
        }
    }

    static void detectIterationPatterns(const std::string& src, AlgorithmAnalysisResult& r) {
        // Counting loop: for i in range / for(int i=0; ...)
        if (src.find("range(") != std::string::npos ||
            (src.find("for") != std::string::npos && src.find("i++") != std::string::npos)) {
            r.detectedPatterns.push_back({"counting_loop", "iteration", "counting loop", 0.85f});
        }
    }

    static AlgorithmEquivalent mapToTarget(const AlgorithmPattern& pattern,
                                            const std::string& src,
                                            const std::string& tgt,
                                            const std::string& complexity) {
        AlgorithmEquivalent eq;
        eq.patternName = pattern.name;
        eq.sourceLanguage = src;
        eq.targetLanguage = tgt;
        eq.complexity = complexity;

        if (pattern.category == "sort") {
            eq.usesStdLib = complexity.empty() || complexity != "preserve";
            if (tgt == "rust") eq.targetCode = "items.sort()";
            else if (tgt == "python") eq.targetCode = "sorted(items)";
            else if (tgt == "java") eq.targetCode = "Collections.sort(items)";
            else if (tgt == "cpp" || tgt == "c++") eq.targetCode = "std::sort(items.begin(), items.end())";
            else if (tgt == "go") eq.targetCode = "sort.Ints(items)";
            else eq.targetCode = "sort(items)";
        } else if (pattern.name == "linear_search") {
            eq.usesStdLib = true;
            if (tgt == "rust") eq.targetCode = "items.iter().find(|x| pred(x))";
            else if (tgt == "python") eq.targetCode = "next((x for x in items if pred(x)), None)";
            else if (tgt == "java") eq.targetCode = "items.stream().filter(pred).findFirst()";
            else if (tgt == "cpp") eq.targetCode = "std::find_if(items.begin(), items.end(), pred)";
            else eq.targetCode = "find(items, pred)";
        } else if (pattern.name == "binary_search") {
            eq.usesStdLib = true;
            if (tgt == "rust") eq.targetCode = "items.binary_search(&key)";
            else if (tgt == "python") eq.targetCode = "bisect.bisect_left(items, key)";
            else if (tgt == "java") eq.targetCode = "Collections.binarySearch(items, key)";
            else if (tgt == "cpp") eq.targetCode = "std::lower_bound(items.begin(), items.end(), key)";
            else eq.targetCode = "binary_search(items, key)";
        } else if (pattern.name == "key_lookup") {
            eq.usesStdLib = true;
            if (tgt == "rust") eq.targetCode = "map.get(&key)";
            else if (tgt == "python") eq.targetCode = "dict.get(key)";
            else if (tgt == "java") eq.targetCode = "map.get(key)";
            else eq.targetCode = "lookup(map, key)";
        } else if (pattern.name == "manual_map") {
            eq.usesStdLib = true;
            if (tgt == "rust") eq.targetCode = "items.iter().map(f).collect()";
            else if (tgt == "python") eq.targetCode = "[f(x) for x in items]";
            else if (tgt == "java") eq.targetCode = "items.stream().map(f).collect(Collectors.toList())";
            else eq.targetCode = "map(items, f)";
        } else if (pattern.name == "manual_filter") {
            eq.usesStdLib = true;
            if (tgt == "rust") eq.targetCode = "items.iter().filter(pred).collect()";
            else if (tgt == "python") eq.targetCode = "[x for x in items if pred(x)]";
            else if (tgt == "java") eq.targetCode = "items.stream().filter(pred).collect(Collectors.toList())";
            else eq.targetCode = "filter(items, pred)";
        } else if (pattern.name == "manual_reduce") {
            eq.usesStdLib = true;
            if (tgt == "rust") eq.targetCode = "items.iter().fold(init, |acc, x| acc + x)";
            else if (tgt == "python") eq.targetCode = "functools.reduce(f, items, init)";
            else if (tgt == "java") eq.targetCode = "items.stream().reduce(init, Integer::sum)";
            else eq.targetCode = "reduce(items, f, init)";
        } else if (pattern.name == "builder_pattern") {
            eq.usesStdLib = false;
            if (tgt == "rust") eq.targetCode = "Builder::new().field(value).build()";
            else if (tgt == "python") eq.targetCode = "dataclass or __init__ with defaults";
            else if (tgt == "java") eq.targetCode = "Builder.newBuilder().setField(value).build()";
            else eq.targetCode = "builder.build()";
        } else if (pattern.name == "counting_loop") {
            eq.usesStdLib = true;
            if (tgt == "rust") eq.targetCode = "for i in 0..n { }";
            else if (tgt == "python") eq.targetCode = "for i in range(n):";
            else if (tgt == "java") eq.targetCode = "for (int i = 0; i < n; i++) { }";
            else eq.targetCode = "for i in 0..n";
        } else {
            eq.targetCode = "";
        }

        return eq;
    }
};
