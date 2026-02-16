#pragma once

#include "ASTNode.h"
#include "Module.h"
#include "Function.h"
#include "Variable.h"
#include "Parameter.h"
#include "Type.h"
#include "ClassDeclaration.h"
#include "Statement.h"
#include "Expression.h"
#include "AsyncNodes.h"
#include "PreprocessorNodes.h"
#include "Annotation.h"
#include "../ast/Parser.h"
#include <cctype>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class CommonLispParser {
public:
    static std::unique_ptr<Module> parseCommonLisp(const std::string& source) {
        ParseResult result = parseCommonLispWithDiagnostics(source);
        return std::move(result.module);
    }

    static ParseResult parseCommonLispWithDiagnostics(const std::string& source) {
        ParseResult result;
        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_common_lisp_module";
        result.module->targetLanguage = "common-lisp";

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
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    static bool isDelimiter(char c) {
        return isWhitespace(c) || c == '(' || c == ')' || c == '\'' || c == '`' || c == ';';
    }

    static std::string canonicalHead(const std::string& symbol) {
        size_t pos = symbol.rfind(':');
        if (pos == std::string::npos) return symbol;
        return symbol.substr(pos + 1);
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
        bool seenDigit = false;
        bool seenDot = false;
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

    static bool isEarmuff(const std::string& symbol) {
        return symbol.size() > 2 && symbol.front() == '*' && symbol.back() == '*';
    }

    static PrimitiveType* primitiveType(const std::string& name) {
        return new PrimitiveType(IdGenerator::next("type"), name);
    }

    static ASTNode* mapTypeName(const std::string& tname) {
        std::string t = canonicalHead(tname);
        if (t == "fixnum" || t == "integer") return primitiveType("int");
        if (t == "single-float" || t == "double-float" || t == "float") return primitiveType("float");
        if (t == "string") return primitiveType("string");
        if (t == "t") return primitiveType("any");
        if (t == "nil") return primitiveType("nil");
        return new CustomType(IdGenerator::next("type"), t);
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
        diags.push_back({1, 1, "Unterminated string in Common Lisp source", "error"});
        return out;
    }

    static std::string parseAtomToken(const std::string& source, size_t& pos) {
        size_t start = pos;
        while (pos < source.size() && !isDelimiter(source[pos])) ++pos;
        return source.substr(start, pos - start);
    }

    static SExpr makeQuotedForm(const SExpr& value) {
        SExpr quoted;
        quoted.isList = true;
        SExpr quoteSymbol;
        quoteSymbol.atom = "quote";
        quoted.list.push_back(quoteSymbol);
        quoted.list.push_back(value);
        return quoted;
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
                    diags.push_back({1, 1, "Unbalanced parentheses in Common Lisp source", "error"});
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
            diags.push_back({1, 1, "Unmatched closing parenthesis in Common Lisp source", "error"});
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

    static std::string atomText(const SExpr& expr) {
        return expr.isList ? "" : expr.atom;
    }

    static std::string headSymbol(const SExpr& expr) {
        if (!expr.isList || expr.list.empty() || expr.list[0].isList) return "";
        return canonicalHead(expr.list[0].atom);
    }

    static void appendExpressionStatement(ASTNode* owner, const std::string& role, ASTNode* expr) {
        if (!expr) return;
        auto* stmt = new ExpressionStatement();
        stmt->id = IdGenerator::next("exprstmt");
        stmt->setChild("expression", expr);
        owner->addChild(role, stmt);
    }

    static ASTNode* convertIfExpression(const SExpr& expr) {
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

    static ASTNode* convertCondExpression(const SExpr& expr) {
        if (expr.list.size() < 2) return nullptr;
        IfStatement* root = nullptr;
        IfStatement* current = nullptr;

        for (size_t i = 1; i < expr.list.size(); ++i) {
            const SExpr& clause = expr.list[i];
            if (!clause.isList || clause.list.empty()) continue;

            auto* node = new IfStatement();
            node->id = IdGenerator::next("if");
            node->setChild("condition", convertExpression(clause.list[0]));
            if (clause.list.size() >= 2) {
                appendExpressionStatement(node, "thenBranch", convertExpression(clause.list[1]));
            }

            if (!root) root = node;
            if (current) current->addChild("elseBranch", node);
            current = node;
        }

        return root;
    }

    static ASTNode* convertLambdaExpression(const SExpr& expr) {
        auto* lambda = new LambdaExpression();
        lambda->id = IdGenerator::next("lambda");

        if (expr.list.size() > 1 && expr.list[1].isList) {
            for (const auto& p : expr.list[1].list) {
                if (p.isList || p.atom.empty()) continue;
                std::string name = p.atom;
                if (!name.empty() && name[0] == '&') continue;
                auto* param = new Parameter(IdGenerator::next("param"), name);
                lambda->addChild("parameters", param);
            }
        }

        for (size_t i = 2; i < expr.list.size(); ++i) {
            appendExpressionStatement(lambda, "body", convertExpression(expr.list[i]));
        }
        return lambda;
    }

    static ASTNode* convertLetExpression(const SExpr& expr) {
        auto* block = new Block();
        block->id = IdGenerator::next("blk");

        if (expr.list.size() > 1 && expr.list[1].isList) {
            for (const auto& binding : expr.list[1].list) {
                if (!binding.isList || binding.list.empty()) continue;
                std::string name = atomText(binding.list[0]);
                if (name.empty()) continue;

                auto* var = new Variable(IdGenerator::next("var"), name);
                if (binding.list.size() > 1) {
                    var->setChild("value", convertExpression(binding.list[1]));
                }
                block->addChild("statements", var);
            }
        }

        for (size_t i = 2; i < expr.list.size(); ++i) {
            appendExpressionStatement(block, "statements", convertExpression(expr.list[i]));
        }
        return block;
    }

    static ASTNode* convertLoopForm(const SExpr& expr) {
        auto* loop = new ForLoop();
        loop->id = IdGenerator::next("for");
        std::string head = headSymbol(expr);

        if (head == "dotimes" && expr.list.size() > 1 && expr.list[1].isList && !expr.list[1].list.empty()) {
            loop->iteratorName = atomText(expr.list[1].list[0]);
            if (expr.list[1].list.size() > 1) {
                loop->setChild("iterable", convertExpression(expr.list[1].list[1]));
            }
        } else {
            loop->iteratorName = head;
        }

        size_t start = (head == "dotimes") ? 2 : 1;
        for (size_t i = start; i < expr.list.size(); ++i) {
            appendExpressionStatement(loop, "body", convertExpression(expr.list[i]));
        }
        return loop;
    }

    static void attachQuotedAnnotation(ASTNode* node) {
        if (!node) return;
        auto* meta = new MetaAnnotation();
        meta->id = IdGenerator::next("anno");
        meta->state = "quoted";
        meta->phase = "compile";
        node->addChild("annotations", meta);
    }

    static ASTNode* convertCallLike(const SExpr& expr) {
        if (!expr.isList || expr.list.empty()) return nullptr;
        auto* call = new FunctionCall();
        call->id = IdGenerator::next("call");

        std::string head = headSymbol(expr);
        if (head == "funcall" || head == "apply") {
            if (expr.list.size() > 1) call->functionName = atomText(expr.list[1]);
            for (size_t i = 2; i < expr.list.size(); ++i) {
                ASTNode* arg = convertExpression(expr.list[i]);
                if (arg) call->addChild("arguments", arg);
            }
            return call;
        }

        call->functionName = atomText(expr.list[0]);
        for (size_t i = 1; i < expr.list.size(); ++i) {
            ASTNode* arg = convertExpression(expr.list[i]);
            if (arg) call->addChild("arguments", arg);
        }
        return call;
    }

    static ASTNode* convertValuesExpression(const SExpr& expr) {
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

    static ASTNode* convertExpression(const SExpr& expr) {
        if (!expr.isList) {
            std::string atom = expr.atom;
            if (atom.empty()) return nullptr;
            if (isInteger(atom)) return new IntegerLiteral(IdGenerator::next("int"), std::stoi(atom));
            if (isFloat(atom)) return new FloatLiteral(IdGenerator::next("flt"), atom);
            if (isStringToken(atom)) return new StringLiteral(IdGenerator::next("str"), atom.substr(1, atom.size() - 2));
            if (atom == "t") return new BooleanLiteral(IdGenerator::next("bool"), true);
            if (atom == "nil") return new BooleanLiteral(IdGenerator::next("bool"), false);
            return new VariableReference(IdGenerator::next("ref"), atom);
        }

        std::string head = headSymbol(expr);
        if (head == "if") return convertIfExpression(expr);
        if (head == "cond") return convertCondExpression(expr);
        if (head == "lambda") return convertLambdaExpression(expr);
        if (head == "let" || head == "let*") return convertLetExpression(expr);
        if (head == "loop" || head == "do" || head == "dotimes") return convertLoopForm(expr);
        if (head == "quote") {
            ASTNode* quotedExpr = (expr.list.size() > 1) ? convertExpression(expr.list[1]) : nullptr;
            if (!quotedExpr) quotedExpr = new VariableReference(IdGenerator::next("ref"), "nil");
            attachQuotedAnnotation(quotedExpr);
            return quotedExpr;
        }
        if (head == "values") return convertValuesExpression(expr);
        return convertCallLike(expr);
    }

    static void parseDeclareTypeForms(
        const std::vector<SExpr>& body,
        std::unordered_map<std::string, std::string>& declaredTypes) {
        for (const auto& form : body) {
            if (!form.isList || headSymbol(form) != "declare") continue;
            for (size_t i = 1; i < form.list.size(); ++i) {
                const SExpr& decl = form.list[i];
                if (!decl.isList || decl.list.size() < 3) continue;
                if (headSymbol(decl) != "type") continue;
                std::string tname = atomText(decl.list[1]);
                for (size_t j = 2; j < decl.list.size(); ++j) {
                    std::string var = atomText(decl.list[j]);
                    if (!var.empty()) declaredTypes[var] = tname;
                }
            }
        }
    }

    static void addDynamicBindingAnnotationIfNeeded(ASTNode* node, const std::string& symbol) {
        if (!isEarmuff(symbol)) return;
        auto* binding = new BindingAnnotation();
        binding->id = IdGenerator::next("anno");
        binding->time = "dynamic";
        node->addChild("annotations", binding);
    }

    static void convertDefun(const SExpr& form, Module* module) {
        if (form.list.size() < 3) return;
        std::string name = atomText(form.list[1]);
        auto* fn = new Function(IdGenerator::next("fn"), name);

        if (form.list[2].isList) {
            for (const auto& p : form.list[2].list) {
                if (p.isList || p.atom.empty() || p.atom[0] == '&') continue;
                auto* param = new Parameter(IdGenerator::next("param"), p.atom);
                fn->addChild("parameters", param);
            }
        }

        std::vector<SExpr> body;
        for (size_t i = 3; i < form.list.size(); ++i) body.push_back(form.list[i]);

        std::unordered_map<std::string, std::string> declaredTypes;
        parseDeclareTypeForms(body, declaredTypes);
        for (ASTNode* p : fn->getChildren("parameters")) {
            auto* param = static_cast<Parameter*>(p);
            auto it = declaredTypes.find(param->name);
            if (it != declaredTypes.end()) {
                param->setChild("type", mapTypeName(it->second));
            }
        }

        for (const auto& expr : body) {
            if (expr.isList && headSymbol(expr) == "declare") continue;
            appendExpressionStatement(fn, "body", convertExpression(expr));
        }
        module->addChild("functions", fn);
    }

    static void convertDefmethod(const SExpr& form, Module* module) {
        if (form.list.size() < 3) return;
        std::string name = atomText(form.list[1]);
        auto* method = new MethodDeclaration(IdGenerator::next("method"), name);

        if (form.list[2].isList) {
            for (const auto& paramForm : form.list[2].list) {
                if (!paramForm.isList) {
                    std::string pname = atomText(paramForm);
                    if (pname.empty()) continue;
                    auto* p = new Parameter(IdGenerator::next("param"), pname);
                    method->addChild("parameters", p);
                    continue;
                }
                if (paramForm.list.empty()) continue;
                std::string pname = atomText(paramForm.list[0]);
                if (pname.empty()) continue;

                auto* p = new Parameter(IdGenerator::next("param"), pname);
                if (paramForm.list.size() > 1) {
                    std::string cls = atomText(paramForm.list[1]);
                    p->setChild("type", new CustomType(IdGenerator::next("type"), cls));
                    if (method->className.empty()) method->className = cls;
                }
                method->addChild("parameters", p);
            }
        }

        for (size_t i = 3; i < form.list.size(); ++i) {
            appendExpressionStatement(method, "body", convertExpression(form.list[i]));
        }
        module->addChild("functions", method);
    }

    static void convertDefclass(const SExpr& form, Module* module) {
        if (form.list.size() < 2) return;
        auto* cls = new ClassDeclaration(IdGenerator::next("cls"), atomText(form.list[1]));

        if (form.list.size() > 2 && form.list[2].isList) {
            for (const auto& base : form.list[2].list) {
                std::string b = atomText(base);
                if (!b.empty()) cls->addBase(b);
            }
        }

        if (form.list.size() > 3 && form.list[3].isList) {
            for (const auto& slot : form.list[3].list) {
                if (!slot.isList || slot.list.empty()) continue;
                std::string fieldName = atomText(slot.list[0]);
                if (fieldName.empty()) continue;
                auto* field = new Variable(IdGenerator::next("var"), fieldName);
                cls->addChild("fields", field);
            }
        }

        module->addChild("classes", cls);
    }

    static void convertDefvar(const SExpr& form, Module* module) {
        if (form.list.size() < 2) return;
        std::string name = atomText(form.list[1]);
        auto* var = new Variable(IdGenerator::next("var"), name);

        addDynamicBindingAnnotationIfNeeded(var, name);
        if (form.list.size() > 2) {
            var->setChild("value", convertExpression(form.list[2]));
        }
        module->addChild("variables", var);
    }

    static void convertDefmacro(const SExpr& form, Module* module) {
        if (form.list.size() < 2) return;
        auto* macro = new MacroDefinition(IdGenerator::next("mac"), atomText(form.list[1]));
        macro->isFunctionLike = true;

        if (form.list.size() > 2 && form.list[2].isList) {
            for (const auto& p : form.list[2].list) {
                if (!p.isList && !p.atom.empty()) macro->parameters.push_back(p.atom);
            }
        }

        std::string body = "(";
        for (size_t i = 3; i < form.list.size(); ++i) {
            if (i > 3) body += ' ';
            body += atomText(form.list[i]).empty() ? "form" : atomText(form.list[i]);
        }
        body += ")";
        macro->body = body;
        module->addChild("statements", macro);
    }

    static void convertTopLevelForm(const SExpr& form, Module* module) {
        if (!form.isList || form.list.empty()) return;
        std::string head = headSymbol(form);

        if (head == "defun") {
            convertDefun(form, module);
            return;
        }
        if (head == "defmethod") {
            convertDefmethod(form, module);
            return;
        }
        if (head == "defclass") {
            convertDefclass(form, module);
            return;
        }
        if (head == "defvar" || head == "defparameter") {
            convertDefvar(form, module);
            return;
        }
        if (head == "defmacro") {
            convertDefmacro(form, module);
            return;
        }

        appendExpressionStatement(module, "statements", convertExpression(form));
    }
};
