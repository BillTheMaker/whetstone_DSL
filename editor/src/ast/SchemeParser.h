#pragma once

#include "ASTNode.h"
#include "Module.h"
#include "Function.h"
#include "Variable.h"
#include "Parameter.h"
#include "Type.h"
#include "Statement.h"
#include "Expression.h"
#include "AsyncNodes.h"
#include "PreprocessorNodes.h"
#include "Annotation.h"
#include "../ast/Parser.h"
#include <cctype>
#include <string>
#include <vector>

class SchemeParser {
public:
    static std::unique_ptr<Module> parseScheme(const std::string& source) {
        ParseResult result = parseSchemeWithDiagnostics(source);
        return std::move(result.module);
    }

    static ParseResult parseSchemeWithDiagnostics(const std::string& source) {
        ParseResult result;
        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_scheme_module";
        result.module->targetLanguage = "scheme";

        std::vector<SExpr> forms = parseForms(source, result.diagnostics);
        for (const auto& form : forms) {
            convertTopLevelForm(form, result.module.get());
        }
        return result;
    }

private:
    struct SExpr {
        bool isList = false;
        std::string atom;
        std::vector<SExpr> list;
    };

    static bool isWhitespace(char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    static bool isDelimiter(char c) {
        return isWhitespace(c) || c == '(' || c == ')' || c == '\'' || c == '`' || c == ';';
    }

    static bool isInteger(const std::string& s) {
        if (s.empty()) return false;
        size_t i = (s[0] == '-' || s[0] == '+') ? 1 : 0;
        if (i >= s.size()) return false;
        for (; i < s.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
        }
        return true;
    }

    static bool isFloat(const std::string& s) {
        if (s.empty()) return false;
        bool seenDot = false;
        bool seenDigit = false;
        size_t i = (s[0] == '-' || s[0] == '+') ? 1 : 0;
        if (i >= s.size()) return false;
        for (; i < s.size(); ++i) {
            char c = s[i];
            if (std::isdigit(static_cast<unsigned char>(c))) {
                seenDigit = true;
            } else if (c == '.' && !seenDot) {
                seenDot = true;
            } else {
                return false;
            }
        }
        return seenDigit && seenDot;
    }

    static bool isStringToken(const std::string& s) {
        return s.size() >= 2 && s.front() == '"' && s.back() == '"';
    }

    static std::string canonicalHead(const std::string& symbol) {
        size_t pos = symbol.rfind(':');
        if (pos == std::string::npos) return symbol;
        return symbol.substr(pos + 1);
    }

    static std::string atomText(const SExpr& expr) {
        return expr.isList ? "" : expr.atom;
    }

    static std::string headSymbol(const SExpr& expr) {
        if (!expr.isList || expr.list.empty() || expr.list[0].isList) return "";
        return canonicalHead(expr.list[0].atom);
    }

    static void skipWhitespaceAndComments(const std::string& source, size_t& pos) {
        while (pos < source.size()) {
            if (isWhitespace(source[pos])) {
                ++pos;
                continue;
            }
            if (source[pos] == ';') {
                while (pos < source.size() && source[pos] != '\n') ++pos;
                continue;
            }
            break;
        }
    }

    static std::string parseStringToken(const std::string& source, size_t& pos,
                                        std::vector<ParseDiagnostic>& diags) {
        std::string out;
        out.push_back('"');
        ++pos;
        bool escaped = false;
        while (pos < source.size()) {
            char c = source[pos++];
            out.push_back(c);
            if (escaped) {
                escaped = false;
                continue;
            }
            if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                return out;
            }
        }
        diags.push_back({1, 1, "Unterminated string in Scheme source", "error"});
        return out;
    }

    static std::string parseAtomToken(const std::string& source, size_t& pos) {
        size_t start = pos;
        while (pos < source.size() && !isDelimiter(source[pos])) ++pos;
        return source.substr(start, pos - start);
    }

    static SExpr makeQuotedForm(const SExpr& value) {
        SExpr q;
        q.isList = true;
        SExpr quoteSym;
        quoteSym.atom = "quote";
        q.list.push_back(quoteSym);
        q.list.push_back(value);
        return q;
    }

    static SExpr parseSingleForm(const std::string& source, size_t& pos,
                                 std::vector<ParseDiagnostic>& diags) {
        skipWhitespaceAndComments(source, pos);
        if (pos >= source.size()) return SExpr{};

        if (source[pos] == '(') {
            ++pos;
            SExpr list;
            list.isList = true;
            for (;;) {
                skipWhitespaceAndComments(source, pos);
                if (pos >= source.size()) {
                    diags.push_back({1, 1, "Unbalanced parentheses in Scheme source", "error"});
                    return list;
                }
                if (source[pos] == ')') {
                    ++pos;
                    return list;
                }
                list.list.push_back(parseSingleForm(source, pos, diags));
            }
        }

        if (source[pos] == ')') {
            diags.push_back({1, 1, "Unmatched closing parenthesis in Scheme source", "error"});
            ++pos;
            return SExpr{};
        }

        if (source[pos] == '\'' || source[pos] == '`') {
            ++pos;
            SExpr inner = parseSingleForm(source, pos, diags);
            return makeQuotedForm(inner);
        }

        if (source[pos] == '"') {
            SExpr atom;
            atom.atom = parseStringToken(source, pos, diags);
            return atom;
        }

        SExpr atom;
        atom.atom = parseAtomToken(source, pos);
        return atom;
    }

    static std::vector<SExpr> parseForms(const std::string& source,
                                         std::vector<ParseDiagnostic>& diags) {
        std::vector<SExpr> out;
        size_t pos = 0;
        while (pos < source.size()) {
            skipWhitespaceAndComments(source, pos);
            if (pos >= source.size()) break;
            out.push_back(parseSingleForm(source, pos, diags));
        }
        return out;
    }

    static void appendExpressionStatement(ASTNode* owner, const std::string& role, ASTNode* expr) {
        if (!expr) return;
        auto* stmt = new ExpressionStatement();
        stmt->id = IdGenerator::next("exprstmt");
        stmt->setChild("expression", expr);
        owner->addChild(role, stmt);
    }

    static ASTNode* convertExpression(const SExpr& expr);

    static ASTNode* convertIf(const SExpr& expr) {
        if (expr.list.size() < 3) return nullptr;
        auto* node = new IfStatement();
        node->id = IdGenerator::next("if");
        node->setChild("condition", convertExpression(expr.list[1]));
        appendExpressionStatement(node, "thenBranch", convertExpression(expr.list[2]));
        if (expr.list.size() > 3) {
            appendExpressionStatement(node, "elseBranch", convertExpression(expr.list[3]));
        }
        return node;
    }

    static ASTNode* convertCond(const SExpr& expr) {
        IfStatement* root = nullptr;
        IfStatement* cur = nullptr;
        for (size_t i = 1; i < expr.list.size(); ++i) {
            const auto& clause = expr.list[i];
            if (!clause.isList || clause.list.empty()) continue;
            auto* node = new IfStatement();
            node->id = IdGenerator::next("if");
            node->setChild("condition", convertExpression(clause.list[0]));
            if (clause.list.size() > 1) {
                appendExpressionStatement(node, "thenBranch", convertExpression(clause.list[1]));
            }
            if (!root) root = node;
            if (cur) cur->addChild("elseBranch", node);
            cur = node;
        }
        return root;
    }

    static ASTNode* convertLambda(const SExpr& expr) {
        auto* lambda = new LambdaExpression();
        lambda->id = IdGenerator::next("lambda");
        if (expr.list.size() > 1 && expr.list[1].isList) {
            for (const auto& p : expr.list[1].list) {
                if (p.isList || p.atom.empty()) continue;
                auto* param = new Parameter(IdGenerator::next("param"), p.atom);
                lambda->addChild("parameters", param);
            }
        }
        for (size_t i = 2; i < expr.list.size(); ++i) {
            appendExpressionStatement(lambda, "body", convertExpression(expr.list[i]));
        }
        return lambda;
    }

    static ASTNode* convertLet(const SExpr& expr) {
        auto* block = new Block();
        block->id = IdGenerator::next("blk");
        if (expr.list.size() > 1 && expr.list[1].isList) {
            for (const auto& binding : expr.list[1].list) {
                if (!binding.isList || binding.list.empty()) continue;
                std::string name = atomText(binding.list[0]);
                if (name.empty()) continue;
                auto* v = new Variable(IdGenerator::next("var"), name);
                if (binding.list.size() > 1) {
                    v->setChild("value", convertExpression(binding.list[1]));
                }
                block->addChild("statements", v);
            }
        }
        for (size_t i = 2; i < expr.list.size(); ++i) {
            appendExpressionStatement(block, "statements", convertExpression(expr.list[i]));
        }
        return block;
    }

    static ASTNode* convertDo(const SExpr& expr) {
        auto* loop = new ForLoop();
        loop->id = IdGenerator::next("for");
        loop->iteratorName = "do";

        if (expr.list.size() > 1 && expr.list[1].isList && !expr.list[1].list.empty()) {
            const auto& firstBinding = expr.list[1].list[0];
            if (firstBinding.isList && !firstBinding.list.empty()) {
                loop->iteratorName = atomText(firstBinding.list[0]);
                if (firstBinding.list.size() > 1) {
                    loop->setChild("iterable", convertExpression(firstBinding.list[1]));
                }
            }
        }

        size_t bodyStart = 3;
        if (expr.list.size() > 2 && expr.list[2].isList && expr.list[2].list.size() > 1) {
            appendExpressionStatement(loop, "body", convertExpression(expr.list[2].list[1]));
        }
        for (size_t i = bodyStart; i < expr.list.size(); ++i) {
            appendExpressionStatement(loop, "body", convertExpression(expr.list[i]));
        }
        return loop;
    }

    static ASTNode* convertBegin(const SExpr& expr) {
        auto* block = new Block();
        block->id = IdGenerator::next("blk");
        for (size_t i = 1; i < expr.list.size(); ++i) {
            appendExpressionStatement(block, "statements", convertExpression(expr.list[i]));
        }
        return block;
    }

    static ASTNode* convertCall(const SExpr& expr) {
        auto* call = new FunctionCall();
        call->id = IdGenerator::next("call");

        std::string head = headSymbol(expr);
        if (head == "call-with-current-continuation" || head == "call/cc") {
            call->functionName = head;
            auto* exec = new ExecAnnotation();
            exec->id = IdGenerator::next("anno");
            exec->mode = "continuation";
            call->addChild("annotations", exec);
        } else {
            call->functionName = atomText(expr.list[0]);
        }

        size_t start = 1;
        for (size_t i = start; i < expr.list.size(); ++i) {
            ASTNode* arg = convertExpression(expr.list[i]);
            if (arg) call->addChild("arguments", arg);
        }
        return call;
    }

    static ASTNode* convertValues(const SExpr& expr) {
        auto* call = new FunctionCall();
        call->id = IdGenerator::next("call");
        call->functionName = "values";
        for (size_t i = 1; i < expr.list.size(); ++i) {
            ASTNode* arg = convertExpression(expr.list[i]);
            if (arg) call->addChild("arguments", arg);
        }

        auto* contract = new ContractAnnotation();
        contract->id = IdGenerator::next("anno");
        contract->returnShape = "multiple-values";
        call->addChild("annotations", contract);
        return call;
    }

    static ASTNode* convertQuote(const SExpr& expr) {
        ASTNode* quoted = (expr.list.size() > 1) ? convertExpression(expr.list[1]) : nullptr;
        if (!quoted) quoted = new VariableReference(IdGenerator::next("ref"), "nil");
        auto* meta = new MetaAnnotation();
        meta->id = IdGenerator::next("anno");
        meta->state = "quoted";
        quoted->addChild("annotations", meta);
        return quoted;
    }

    static void convertDefineFunction(const SExpr& form, Module* module) {
        if (form.list.size() < 3 || !form.list[1].isList || form.list[1].list.empty()) return;
        std::string name = atomText(form.list[1].list[0]);
        auto* fn = new Function(IdGenerator::next("fn"), name);

        for (size_t i = 1; i < form.list[1].list.size(); ++i) {
            std::string pname = atomText(form.list[1].list[i]);
            if (pname.empty()) continue;
            fn->addChild("parameters", new Parameter(IdGenerator::next("param"), pname));
        }

        for (size_t i = 2; i < form.list.size(); ++i) {
            appendExpressionStatement(fn, "body", convertExpression(form.list[i]));
        }
        module->addChild("functions", fn);
    }

    static void convertDefineVariable(const SExpr& form, Module* module) {
        if (form.list.size() < 2) return;
        std::string name = atomText(form.list[1]);
        auto* var = new Variable(IdGenerator::next("var"), name);
        if (form.list.size() > 2) {
            var->setChild("value", convertExpression(form.list[2]));
        }
        module->addChild("variables", var);
    }

    static void convertDefineSyntax(const SExpr& form, Module* module) {
        if (form.list.size() < 2) return;
        auto* macro = new MacroDefinition(IdGenerator::next("mac"), atomText(form.list[1]));
        macro->isFunctionLike = true;
        macro->body = "syntax-rules";

        auto* meta = new MetaAnnotation();
        meta->id = IdGenerator::next("anno");
        meta->state = "hygienic";
        macro->addChild("annotations", meta);

        module->addChild("statements", macro);
    }

    static void convertTopLevelForm(const SExpr& form, Module* module) {
        if (!form.isList || form.list.empty()) return;
        std::string head = headSymbol(form);

        if (head == "define") {
            if (form.list.size() > 1 && form.list[1].isList) {
                convertDefineFunction(form, module);
            } else {
                convertDefineVariable(form, module);
            }
            return;
        }

        if (head == "define-syntax") {
            convertDefineSyntax(form, module);
            return;
        }

        appendExpressionStatement(module, "statements", convertExpression(form));
    }
};

inline ASTNode* SchemeParser::convertExpression(const SExpr& expr) {
    if (!expr.isList) {
        if (expr.atom.empty()) return nullptr;
        if (isInteger(expr.atom)) return new IntegerLiteral(IdGenerator::next("int"), std::stoi(expr.atom));
        if (isFloat(expr.atom)) return new FloatLiteral(IdGenerator::next("flt"), expr.atom);
        if (isStringToken(expr.atom)) {
            return new StringLiteral(IdGenerator::next("str"), expr.atom.substr(1, expr.atom.size() - 2));
        }
        if (expr.atom == "#t" || expr.atom == "t") return new BooleanLiteral(IdGenerator::next("bool"), true);
        if (expr.atom == "#f" || expr.atom == "nil") return new BooleanLiteral(IdGenerator::next("bool"), false);
        return new VariableReference(IdGenerator::next("ref"), expr.atom);
    }

    std::string head = headSymbol(expr);
    if (head == "if") return convertIf(expr);
    if (head == "cond") return convertCond(expr);
    if (head == "lambda") return convertLambda(expr);
    if (head == "let" || head == "let*" || head == "letrec") return convertLet(expr);
    if (head == "do") return convertDo(expr);
    if (head == "begin") return convertBegin(expr);
    if (head == "values") return convertValues(expr);
    if (head == "quote") return convertQuote(expr);
    return convertCall(expr);
}
