#pragma once
#include "PrimitivesRegistry.h"
#include "ast/Module.h"
#include "ast/ClassDeclaration.h"
#include "ast/Variable.h"
#include "ast/Type.h"
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

        if (looksLikeClassOrModuleSpec(spec)) {
            result.node = buildClassOrModule(spec, language);
            result.note = "Generated class/module structure from specification.";
            return result;
        }

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
        param->setChild("type", new PrimitiveType(nextId("type"), "string"));
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
                if (sym.source == "import" && !sym.vulnerable) return sym;
            }
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
        if (!sym.tags.empty()) {
            note += " | tags: ";
            for (size_t i = 0; i < sym.tags.size(); ++i) {
                if (i > 0) note += ", ";
                note += sym.tags[i];
            }
        }
        return note;
    }

    static std::string nextId(const std::string& prefix) {
        static int counter = 0;
        return prefix + "_" + std::to_string(counter++);
    }

    static bool containsInsensitive(const std::string& text,
                                    const std::string& token) {
        auto lower = [](const std::string& s) {
            std::string out = s;
            std::transform(out.begin(), out.end(), out.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return out;
        };
        std::string t = lower(text);
        std::string k = lower(token);
        return t.find(k) != std::string::npos;
    }

    static bool looksLikeClassOrModuleSpec(const std::string& spec) {
        return containsInsensitive(spec, "class") ||
               containsInsensitive(spec, "struct") ||
               containsInsensitive(spec, "priorityqueue") ||
               containsInsensitive(spec, "workitem");
    }

    static Variable* makeField(const std::string& name, const std::string& primitiveKind) {
        auto* v = new Variable(nextId("var"), name);
        auto* t = new PrimitiveType(nextId("type"), primitiveKind);
        v->setChild("type", t);
        return v;
    }

    static MethodDeclaration* makeMethod(const std::string& name,
                                         const std::string& retType,
                                         const std::vector<std::pair<std::string, std::string>>& params = {}) {
        auto* m = new MethodDeclaration(nextId("meth"), name);
        if (!retType.empty()) {
            m->setChild("returnType", new PrimitiveType(nextId("type"), retType));
        }
        for (const auto& p : params) {
            auto* param = new Parameter(nextId("param"), p.first);
            param->setChild("type", new PrimitiveType(nextId("type"), p.second));
            m->addChild("parameters", param);
        }
        return m;
    }

    static ASTNode* buildClassOrModule(const std::string& spec,
                                       const std::string& language) {
        auto* mod = new Module(nextId("mod"), "generated_module", language.empty() ? "cpp" : language);

        const bool wantsWorkItem = containsInsensitive(spec, "workitem");
        const bool wantsPriorityQueue = containsInsensitive(spec, "priorityqueue") ||
                                        containsInsensitive(spec, "queue");

        if (wantsWorkItem) {
            auto* workItem = new ClassDeclaration(nextId("cls"), "WorkItem");
            workItem->addChild("fields", makeField("job_id", "string"));
            workItem->addChild("fields", makeField("priority", "int"));
            workItem->addChild("fields", makeField("payload", "string"));
            mod->addChild("classes", workItem);
        }

        if (wantsPriorityQueue) {
            auto* pq = new ClassDeclaration(nextId("cls"), "PriorityQueue");
            pq->addChild("fields", makeField("items", "string"));

            if (wantsWorkItem) {
                pq->addChild("methods", makeMethod("__init__", "", {}));
                pq->addChild("methods", makeMethod("enqueue", "void", {{"item", "string"}}));
                pq->addChild("methods", makeMethod("dequeue", "string", {}));
                pq->addChild("methods", makeMethod("peek", "string", {}));
                pq->addChild("methods", makeMethod("size", "int", {}));
                pq->addChild("methods", makeMethod("empty", "bool", {}));
            } else {
                pq->addChild("methods", makeMethod("enqueue", "void", {{"item", "string"}}));
                pq->addChild("methods", makeMethod("dequeue", "string", {}));
            }

            mod->addChild("classes", pq);
        }

        if (!wantsWorkItem && !wantsPriorityQueue) {
            auto* cls = new ClassDeclaration(nextId("cls"), "GeneratedClass");
            cls->addChild("methods", makeMethod("run", "void", {}));
            mod->addChild("classes", cls);
        }

        (void)language;
        return mod;
    }
};
