#pragma once
#include "PrimitivesRegistry.h"
#include "ast/Function.h"
#include "ast/Parameter.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include <string>
#include <vector>
#include <algorithm>

struct AgentCodeGenResult {
    ASTNode* node = nullptr;
    std::string note;
    std::vector<std::string> usedSymbols;
};

class AgentCodeGen {
public:
    AgentCodeGenResult generate(const std::string& spec,
                                PrimitivesRegistry& registry,
                                const std::string& language,
                                bool preferImports) const {
        AgentCodeGenResult result;

        auto functions = registry.getAvailableFunctions();
        if (functions.empty()) {
            result.note = "No available functions; returning empty stub.";
            result.node = makeEmptyFunction("generated");
            return result;
        }

        PrimitiveSymbol chosen = chooseFunction(functions, preferImports);
        result.usedSymbols.push_back(chosen.name);

        auto* fn = new Function(nextId("fn"), "generated");
        auto* param = new Parameter(nextId("param"), "input");
        fn->addChild("parameters", param);

        auto* call = new FunctionCall();
        call->id = nextId("call");
        call->functionName = chosen.name;
        call->addChild("arguments", new VariableReference(nextId("var"), "input"));

        auto* stmt = new ExpressionStatement();
        stmt->id = nextId("exprstmt");
        stmt->setChild("expression", call);
        fn->addChild("body", stmt);

        result.node = fn;
        result.note = buildNote(spec, chosen, language);
        return result;
    }

private:
    static PrimitiveSymbol chooseFunction(const std::vector<PrimitiveSymbol>& funcs,
                                          bool preferImports) {
        if (preferImports) {
            for (const auto& sym : funcs) {
                if (sym.source == "import") return sym;
            }
        }
        for (const auto& sym : funcs) {
            if (sym.source == "builtin") return sym;
        }
        return funcs.front();
    }

    static Function* makeEmptyFunction(const std::string& name) {
        auto* fn = new Function(nextId("fn"), name);
        return fn;
    }

    static std::string buildNote(const std::string& spec,
                                 const PrimitiveSymbol& sym,
                                 const std::string& language) {
        std::string note = "Generated using " + sym.name + " (" + sym.source + ")";
        if (!spec.empty()) note += " for spec: " + spec;
        if (!language.empty()) note += " in " + language;
        return note;
    }

    static std::string nextId(const std::string& prefix) {
        static int counter = 0;
        return prefix + "_" + std::to_string(counter++);
    }
};
