#pragma once
// Step 66: Cross-language memory projection
//
// CrossLanguageProjector: deep-copies an AST Module, changing its
// targetLanguage while preserving (and optionally adapting) memory
// annotations so that code generators can emit language-appropriate
// constructs (e.g. shared_ptr for @Reclaim(Tracing) in C++).

#include <memory>
#include <string>
#include <vector>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Type.h"
#include "ast/Import.h"
#include "ast/Annotation.h"

class CrossLanguageProjector {
public:
    // Project an AST from source language to target language.
    // Returns a new Module with annotations adapted for the target.
    std::unique_ptr<Module> project(const Module* source,
                                    const std::string& targetLanguage) const {
        if (!source) return nullptr;

        auto mod = std::make_unique<Module>(
            source->id + "_proj", source->name, targetLanguage);

        // Copy imports
        for (auto* child : source->getChildren("imports")) {
            if (child->conceptType == "Import") {
                auto* srcImp = static_cast<const Import*>(child);
                auto* imp = new Import(srcImp->id + "_proj", srcImp->moduleName,
                                       srcImp->importKind, srcImp->alias);
                mod->addChild("imports", imp);
            }
        }

        // Copy functions with annotations and body
        for (auto* child : source->getChildren("functions")) {
            if (child->conceptType == "Function") {
                auto* srcFn = static_cast<const Function*>(child);
                Function* newFn = cloneFunction(srcFn, targetLanguage);
                mod->addChild("functions", newFn);
            }
        }

        // Copy module-level variables
        for (auto* child : source->getChildren("variables")) {
            if (child->conceptType == "Variable") {
                auto* srcVar = static_cast<const Variable*>(child);
                Variable* newVar = new Variable(srcVar->id + "_proj", srcVar->name);
                auto* typeChild = srcVar->getChild("type");
                if (typeChild) {
                    ASTNode* t = cloneNode(typeChild, targetLanguage);
                    if (t) newVar->setChild("type", t);
                }
                auto* init = srcVar->getChild("initializer");
                if (init) {
                    ASTNode* i = cloneNode(init, targetLanguage);
                    if (i) newVar->setChild("initializer", i);
                }
                mod->addChild("variables", newVar);
            }
        }

        // Copy module-level annotations
        for (auto* anno : source->getChildren("annotations")) {
            ASTNode* newAnno = cloneAnnotation(anno, targetLanguage);
            if (newAnno) mod->addChild("annotations", newAnno);
        }

        return mod;
    }

    // Check if all annotation types from the original survive in the projected AST.
    bool annotationsPreserved(const Module* original,
                              const Module* projected) const {
        auto origTypes = collectAnnotationTypes(original);
        auto projTypes = collectAnnotationTypes(projected);

        for (const auto& type : origTypes) {
            bool found = false;
            for (const auto& pt : projTypes) {
                if (pt == type) { found = true; break; }
            }
            if (!found) return false;
        }
        return true;
    }

private:
    Function* cloneFunction(const Function* src,
                            const std::string& targetLanguage) const {
        Function* fn = new Function(src->id + "_proj", src->name);

        // Copy annotations (preserve all, even non-native to target)
        for (auto* anno : src->getChildren("annotations")) {
            ASTNode* newAnno = cloneAnnotation(anno, targetLanguage);
            if (newAnno) fn->addChild("annotations", newAnno);
        }

        // Copy body nodes
        for (auto* child : src->getChildren("body")) {
            ASTNode* newChild = cloneNode(child, targetLanguage);
            if (newChild) fn->addChild("body", newChild);
        }

        // Copy parameters
        for (auto* child : src->getChildren("parameters")) {
            ASTNode* newChild = cloneNode(child, targetLanguage);
            if (newChild) fn->addChild("parameters", newChild);
        }

        // Copy return type
        auto* retType = src->getChild("returnType");
        if (retType) {
            ASTNode* newRet = cloneNode(retType, targetLanguage);
            if (newRet) fn->setChild("returnType", newRet);
        }

        return fn;
    }

    ASTNode* cloneAnnotation(const ASTNode* anno,
                             const std::string& targetLanguage) const {
        if (!anno) return nullptr;

        const auto& ct = anno->conceptType;

        if (ct == "ReclaimAnnotation") {
            auto* src = static_cast<const ReclaimAnnotation*>(anno);
            std::string strategy = adaptReclaimStrategy(src->strategy, targetLanguage);
            auto* a = new ReclaimAnnotation(src->id + "_proj", src->strategy);
            a->strategy = strategy;
            a->reclaimPattern = src->reclaimPattern;
            return a;
        }
        if (ct == "DeallocateAnnotation") {
            auto* src = static_cast<const DeallocateAnnotation*>(anno);
            auto* a = new DeallocateAnnotation(src->id + "_proj", src->strategy);
            a->deallocateLocation = src->deallocateLocation;
            a->owner = src->owner;
            return a;
        }
        if (ct == "LifetimeAnnotation") {
            auto* src = static_cast<const LifetimeAnnotation*>(anno);
            auto* a = new LifetimeAnnotation(src->id + "_proj", src->strategy);
            a->lifetimeScope = src->lifetimeScope;
            return a;
        }
        if (ct == "OwnerAnnotation") {
            auto* src = static_cast<const OwnerAnnotation*>(anno);
            auto* a = new OwnerAnnotation(src->id + "_proj", src->strategy);
            a->ownerType = src->ownerType;
            return a;
        }
        if (ct == "AllocateAnnotation") {
            auto* src = static_cast<const AllocateAnnotation*>(anno);
            auto* a = new AllocateAnnotation(src->id + "_proj", src->strategy);
            a->allocationPattern = src->allocationPattern;
            return a;
        }
        if (ct == "DerefStrategy") {
            auto* src = static_cast<const DerefStrategy*>(anno);
            auto* a = new DerefStrategy(src->id + "_proj", src->strategy);
            a->derefLocation = src->derefLocation;
            a->owner = src->owner;
            return a;
        }
        if (ct == "OptimizationLock") {
            auto* a = new OptimizationLock();
            auto* src = static_cast<const OptimizationLock*>(anno);
            a->id = src->id + "_proj";
            a->lockedBy = src->lockedBy;
            a->lockReason = src->lockReason;
            a->lockLevel = src->lockLevel;
            return a;
        }
        if (ct == "LangSpecific") {
            auto* a = new LangSpecific();
            auto* src = static_cast<const LangSpecific*>(anno);
            a->id = src->id + "_proj";
            a->language = src->language;
            a->idiomType = src->idiomType;
            a->rawSyntax = src->rawSyntax;
            return a;
        }

        return nullptr;
    }

    ASTNode* cloneNode(const ASTNode* node,
                       const std::string& targetLanguage) const {
        if (!node) return nullptr;

        const auto& ct = node->conceptType;

        if (ct == "Variable") {
            auto* src = static_cast<const Variable*>(node);
            auto* v = new Variable(src->id + "_proj", src->name);
            // Clone variable annotations
            for (auto* anno : src->getChildren("annotations")) {
                ASTNode* a = cloneAnnotation(anno, targetLanguage);
                if (a) v->addChild("annotations", a);
            }
            // Clone type child
            auto* typeChild = src->getChild("type");
            if (typeChild) {
                ASTNode* t = cloneNode(typeChild, targetLanguage);
                if (t) v->setChild("type", t);
            }
            // Clone initializer
            auto* init = src->getChild("initializer");
            if (init) {
                ASTNode* i = cloneNode(init, targetLanguage);
                if (i) v->setChild("initializer", i);
            }
            return v;
        }
        if (ct == "Parameter") {
            auto* src = static_cast<const Parameter*>(node);
            auto* p = new Parameter(src->id + "_proj", src->name);
            auto* typeChild = src->getChild("type");
            if (typeChild) {
                ASTNode* t = cloneNode(typeChild, targetLanguage);
                if (t) p->setChild("type", t);
            }
            auto* defaultValue = src->getChild("defaultValue");
            if (defaultValue) {
                ASTNode* d = cloneNode(defaultValue, targetLanguage);
                if (d) p->setChild("defaultValue", d);
            }
            return p;
        }
        if (ct == "PrimitiveType") {
            auto* src = static_cast<const PrimitiveType*>(node);
            return new PrimitiveType(src->id + "_proj", src->kind);
        }
        if (ct == "ListType") {
            auto* t = new ListType();
            t->id = node->id + "_proj";
            auto* elem = node->getChild("elementType");
            if (elem) {
                ASTNode* e = cloneNode(elem, targetLanguage);
                if (e) t->setChild("elementType", e);
            }
            return t;
        }
        if (ct == "SetType") {
            auto* t = new SetType();
            t->id = node->id + "_proj";
            auto* elem = node->getChild("elementType");
            if (elem) {
                ASTNode* e = cloneNode(elem, targetLanguage);
                if (e) t->setChild("elementType", e);
            }
            return t;
        }
        if (ct == "MapType") {
            auto* t = new MapType();
            t->id = node->id + "_proj";
            auto* key = node->getChild("keyType");
            auto* val = node->getChild("valueType");
            if (key) {
                ASTNode* k = cloneNode(key, targetLanguage);
                if (k) t->setChild("keyType", k);
            }
            if (val) {
                ASTNode* v = cloneNode(val, targetLanguage);
                if (v) t->setChild("valueType", v);
            }
            return t;
        }
        if (ct == "TupleType") {
            auto* t = new TupleType();
            t->id = node->id + "_proj";
            for (auto* child : node->getChildren("elementTypes")) {
                ASTNode* c = cloneNode(child, targetLanguage);
                if (c) t->addChild("elementTypes", c);
            }
            return t;
        }
        if (ct == "ArrayType") {
            auto* t = new ArrayType();
            t->id = node->id + "_proj";
            auto* elem = node->getChild("elementType");
            if (elem) {
                ASTNode* e = cloneNode(elem, targetLanguage);
                if (e) t->setChild("elementType", e);
            }
            auto* size = node->getChild("size");
            if (size) {
                ASTNode* s = cloneNode(size, targetLanguage);
                if (s) t->setChild("size", s);
            }
            return t;
        }
        if (ct == "OptionalType") {
            auto* t = new OptionalType();
            t->id = node->id + "_proj";
            auto* inner = node->getChild("innerType");
            if (inner) {
                ASTNode* i = cloneNode(inner, targetLanguage);
                if (i) t->setChild("innerType", i);
            }
            return t;
        }
        if (ct == "CustomType") {
            auto* src = static_cast<const CustomType*>(node);
            return new CustomType(src->id + "_proj", src->typeName);
        }
        if (ct == "Assignment") {
            auto* src = static_cast<const Assignment*>(node);
            auto* a = new Assignment();
            a->id = src->id + "_proj";
            auto* target = src->getChild("target");
            auto* value = src->getChild("value");
            if (target) {
                ASTNode* t = cloneNode(target, targetLanguage);
                if (t) a->setChild("target", t);
            }
            if (value) {
                ASTNode* v = cloneNode(value, targetLanguage);
                if (v) a->setChild("value", v);
            }
            return a;
        }
        if (ct == "Return") {
            auto* src = static_cast<const Return*>(node);
            auto* r = new Return();
            r->id = src->id + "_proj";
            auto* val = src->getChild("value");
            if (val) {
                ASTNode* v = cloneNode(val, targetLanguage);
                if (v) r->setChild("value", v);
            }
            return r;
        }
        if (ct == "BinaryOperation") {
            auto* src = static_cast<const BinaryOperation*>(node);
            auto* b = new BinaryOperation();
            b->id = src->id + "_proj";
            b->op = src->op;
            auto* left = src->getChild("left");
            auto* right = src->getChild("right");
            if (left) {
                ASTNode* l = cloneNode(left, targetLanguage);
                if (l) b->setChild("left", l);
            }
            if (right) {
                ASTNode* r = cloneNode(right, targetLanguage);
                if (r) b->setChild("right", r);
            }
            return b;
        }
        if (ct == "UnaryOperation") {
            auto* src = static_cast<const UnaryOperation*>(node);
            auto* u = new UnaryOperation();
            u->id = src->id + "_proj";
            u->op = src->op;
            auto* operand = src->getChild("operand");
            if (operand) {
                ASTNode* o = cloneNode(operand, targetLanguage);
                if (o) u->setChild("operand", o);
            }
            return u;
        }
        if (ct == "VariableReference") {
            auto* src = static_cast<const VariableReference*>(node);
            return new VariableReference(src->id + "_proj", src->variableName);
        }
        if (ct == "IntegerLiteral") {
            auto* src = static_cast<const IntegerLiteral*>(node);
            return new IntegerLiteral(src->id + "_proj", src->value);
        }
        if (ct == "FloatLiteral") {
            auto* src = static_cast<const FloatLiteral*>(node);
            return new FloatLiteral(src->id + "_proj", src->value);
        }
        if (ct == "StringLiteral") {
            auto* src = static_cast<const StringLiteral*>(node);
            return new StringLiteral(src->id + "_proj", src->value);
        }
        if (ct == "BooleanLiteral") {
            auto* src = static_cast<const BooleanLiteral*>(node);
            return new BooleanLiteral(src->id + "_proj", src->value);
        }
        if (ct == "NullLiteral") {
            auto* n = new NullLiteral();
            n->id = node->id + "_proj";
            return n;
        }
        if (ct == "IfStatement") {
            auto* src = static_cast<const IfStatement*>(node);
            auto* s = new IfStatement();
            s->id = src->id + "_proj";
            auto* cond = src->getChild("condition");
            if (cond) {
                ASTNode* c = cloneNode(cond, targetLanguage);
                if (c) s->setChild("condition", c);
            }
            for (auto* child : src->getChildren("thenBranch")) {
                ASTNode* c = cloneNode(child, targetLanguage);
                if (c) s->addChild("thenBranch", c);
            }
            for (auto* child : src->getChildren("elseBranch")) {
                ASTNode* c = cloneNode(child, targetLanguage);
                if (c) s->addChild("elseBranch", c);
            }
            return s;
        }
        if (ct == "WhileLoop") {
            auto* src = static_cast<const WhileLoop*>(node);
            auto* s = new WhileLoop();
            s->id = src->id + "_proj";
            auto* cond = src->getChild("condition");
            if (cond) {
                ASTNode* c = cloneNode(cond, targetLanguage);
                if (c) s->setChild("condition", c);
            }
            for (auto* child : src->getChildren("body")) {
                ASTNode* c = cloneNode(child, targetLanguage);
                if (c) s->addChild("body", c);
            }
            return s;
        }
        if (ct == "ForLoop") {
            auto* src = static_cast<const ForLoop*>(node);
            auto* s = new ForLoop();
            s->id = src->id + "_proj";
            s->iteratorName = src->iteratorName;
            auto* iter = src->getChild("iterable");
            if (iter) {
                ASTNode* i = cloneNode(iter, targetLanguage);
                if (i) s->setChild("iterable", i);
            }
            for (auto* child : src->getChildren("body")) {
                ASTNode* c = cloneNode(child, targetLanguage);
                if (c) s->addChild("body", c);
            }
            return s;
        }
        if (ct == "ExpressionStatement") {
            auto* src = static_cast<const ExpressionStatement*>(node);
            auto* s = new ExpressionStatement();
            s->id = src->id + "_proj";
            auto* expr = src->getChild("expression");
            if (expr) {
                ASTNode* e = cloneNode(expr, targetLanguage);
                if (e) s->setChild("expression", e);
            }
            return s;
        }
        if (ct == "FunctionCall") {
            auto* src = static_cast<const FunctionCall*>(node);
            auto* c = new FunctionCall();
            c->id = src->id + "_proj";
            c->functionName = src->functionName;
            for (auto* child : src->getChildren("arguments")) {
                ASTNode* a = cloneNode(child, targetLanguage);
                if (a) c->addChild("arguments", a);
            }
            return c;
        }
        if (ct == "Block") {
            auto* src = static_cast<const Block*>(node);
            auto* b = new Block();
            b->id = src->id + "_proj";
            for (auto* child : src->getChildren("statements")) {
                ASTNode* c = cloneNode(child, targetLanguage);
                if (c) b->addChild("statements", c);
            }
            return b;
        }
        if (ct == "ListLiteral") {
            auto* src = static_cast<const ListLiteral*>(node);
            auto* l = new ListLiteral();
            l->id = src->id + "_proj";
            for (auto* child : src->getChildren("elements")) {
                ASTNode* c = cloneNode(child, targetLanguage);
                if (c) l->addChild("elements", c);
            }
            return l;
        }
        if (ct == "IndexAccess") {
            auto* src = static_cast<const IndexAccess*>(node);
            auto* a = new IndexAccess();
            a->id = src->id + "_proj";
            auto* target = src->getChild("target");
            auto* index = src->getChild("index");
            if (target) {
                ASTNode* t = cloneNode(target, targetLanguage);
                if (t) a->setChild("target", t);
            }
            if (index) {
                ASTNode* i = cloneNode(index, targetLanguage);
                if (i) a->setChild("index", i);
            }
            return a;
        }
        if (ct == "MemberAccess") {
            auto* src = static_cast<const MemberAccess*>(node);
            auto* m = new MemberAccess();
            m->id = src->id + "_proj";
            m->memberName = src->memberName;
            auto* target = src->getChild("target");
            if (target) {
                ASTNode* t = cloneNode(target, targetLanguage);
                if (t) m->setChild("target", t);
            }
            return m;
        }

        // Fallback: annotations
        if (ct.find("Annotation") != std::string::npos ||
            ct == "DerefStrategy" || ct == "OptimizationLock" ||
            ct == "LangSpecific") {
            return cloneAnnotation(node, targetLanguage);
        }

        return nullptr;
    }

    static std::string adaptReclaimStrategy(const std::string& strategy,
                                            const std::string& targetLanguage) {
        if (targetLanguage == "go") {
            return "Escape";
        }
        if (targetLanguage == "rust" || targetLanguage == "cpp" ||
            targetLanguage == "java" || targetLanguage == "javascript" ||
            targetLanguage == "typescript" || targetLanguage == "python" ||
            targetLanguage == "elisp") {
            return "Tracing";
        }
        return strategy;
    }

    // Recursively collect all annotation conceptTypes from a tree
    std::vector<std::string> collectAnnotationTypes(const ASTNode* node) const {
        std::vector<std::string> types;
        if (!node) return types;
        collectAnnotationTypesImpl(node, types);
        return types;
    }

    void collectAnnotationTypesImpl(const ASTNode* node,
                                    std::vector<std::string>& types) const {
        if (!node) return;

        // If this is an annotation node, record its type
        const auto& ct = node->conceptType;
        if (ct == "ReclaimAnnotation" || ct == "DeallocateAnnotation" ||
            ct == "LifetimeAnnotation" || ct == "OwnerAnnotation" ||
            ct == "AllocateAnnotation" || ct == "DerefStrategy" ||
            ct == "OptimizationLock" || ct == "LangSpecific") {
            types.push_back(ct);
        }

        // Recurse into all children
        for (auto* child : node->allChildren()) {
            collectAnnotationTypesImpl(child, types);
        }
    }
};
