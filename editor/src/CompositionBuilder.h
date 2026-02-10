#pragma once
#include "PrimitivesRegistry.h"
#include "ast/Expression.h"
#include "ast/Statement.h"
#include "ast/Function.h"
#include "ast/Module.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

struct CompositionStep {
    std::string symbol;
    std::string inputType;
    std::string outputType;
};

struct CompositionState {
    std::vector<CompositionStep> steps;
    std::string inputType;
    std::string outputType;
};

static inline std::string inferTypeHint(const std::string& name) {
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    if (lower.find("string") != std::string::npos || lower.find("str") != std::string::npos) return "string";
    if (lower.find("json") != std::string::npos) return "json";
    if (lower.find("list") != std::string::npos || lower.find("array") != std::string::npos) return "list";
    if (lower.find("map") != std::string::npos || lower.find("dict") != std::string::npos) return "map";
    if (lower.find("int") != std::string::npos || lower.find("count") != std::string::npos) return "int";
    return "any";
}

static inline std::vector<CompositionStep> suggestCompositionSteps(
    const std::vector<PrimitiveSymbol>& primitives,
    const std::string& desiredInput,
    const std::string& desiredOutput) {
    std::vector<CompositionStep> out;
    for (const auto& prim : primitives) {
        if (prim.kind != "function") continue;
        CompositionStep step;
        step.symbol = prim.name;
        step.inputType = desiredInput.empty() ? inferTypeHint(prim.name) : desiredInput;
        step.outputType = desiredOutput.empty() ? inferTypeHint(prim.name) : desiredOutput;
        out.push_back(step);
    }
    return out;
}

static inline std::string buildCompositionCode(const std::vector<CompositionStep>& steps,
                                               const std::string& inputVar) {
    if (steps.empty()) return "";
    std::string expr = inputVar;
    for (const auto& step : steps) {
        expr = step.symbol + "(" + expr + ")";
    }
    return expr;
}

static inline std::string buildCompositionFunction(const std::vector<CompositionStep>& steps,
                                                   const std::string& functionName,
                                                   const std::string& inputVar) {
    std::string expr = buildCompositionCode(steps, inputVar);
    if (expr.empty()) return "";
    std::ostringstream ss;
    ss << "def " << functionName << "(" << inputVar << "):\n";
    ss << "    return " << expr << "\n";
    return ss.str();
}
