#pragma once
#include "ProjectionGenerator.h"
#include "Import.h"
#include "ClassDeclaration.h"
#include "GenericType.h"
#include "AsyncNodes.h"
#include "../SemannoAnnotationImpl.h"
#include <map>
#include <unordered_map>

class JavaGenerator : public ProjectionGenerator, public SemannoAnnotationImpl<JavaGenerator> {
public:
    std::string commentPrefix() const { return "// "; }

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "// Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;

        auto imports = module->getChildren("imports");
        for (const auto* impNode : imports) {
            if (impNode->conceptType != "Import") continue;
            oss << emitImport(static_cast<const Import*>(impNode)) << "\n";
        }
        if (!imports.empty()) oss << "\n";

        // Group functions and variables by class prefix.
        std::map<std::string, std::vector<const Function*>> classMethods;
        std::map<std::string, std::vector<const Variable*>> classFields;
        std::vector<const Function*> topLevelMethods;
        std::vector<const Variable*> topLevelFields;

        auto functions = module->getChildren("functions");
        for (const auto* fnNode : functions) {
            if (fnNode->conceptType != "Function") continue;
            const Function* fn = static_cast<const Function*>(fnNode);
            auto pos = fn->name.find('.');
            if (pos != std::string::npos && pos > 0 && pos + 1 < fn->name.size()) {
                std::string className = fn->name.substr(0, pos);
                classMethods[className].push_back(fn);
            } else {
                topLevelMethods.push_back(fn);
            }
        }

        auto variables = module->getChildren("variables");
        for (const auto* varNode : variables) {
            if (varNode->conceptType != "Variable") continue;
            const Variable* var = static_cast<const Variable*>(varNode);
            auto pos = var->name.find('.');
            if (pos != std::string::npos && pos > 0 && pos + 1 < var->name.size()) {
                std::string className = var->name.substr(0, pos);
                classFields[className].push_back(var);
            } else {
                topLevelFields.push_back(var);
            }
        }

        std::string moduleClass = module->name.empty() ? "Main" : module->name;
        if (!topLevelMethods.empty() || !topLevelFields.empty()) {
            emitClass(oss, moduleClass, topLevelFields, topLevelMethods);
            if (!classMethods.empty() || !classFields.empty()) oss << "\n";
        }

        for (const auto& [className, methods] : classMethods) {
            std::vector<const Variable*> fields;
            auto it = classFields.find(className);
            if (it != classFields.end()) fields = it->second;
            emitClass(oss, className, fields, methods);
            oss << "\n";
        }

        if (!classFields.empty() && classMethods.empty()) {
            for (const auto& [className, fields] : classFields) {
                emitClass(oss, className, fields, {});
                oss << "\n";
            }
        }

        return oss.str();
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;
        emitAnnotations(oss, function->getChildren("annotations"), "");

        std::string methodName = function->name;
        std::string returnType = "void";
        auto retType = function->getChild("returnType");
        if (retType) returnType = generate(retType);

        oss << returnType << " " << methodName << "(";
        emitParameters(oss, function->getChildren("parameters"));
        oss << ") {\n";
        emitBody(oss, function->getChildren("body"), "    ");
        oss << "}\n";
        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        std::ostringstream oss;
        emitAnnotations(oss, variable->getChildren("annotations"), "");
        std::string typeStr = "Object";
        auto type = variable->getChild("type");
        if (type) typeStr = generate(type);
        std::string name = stripClassPrefix(variable->name);
        oss << typeStr << " " << name;
        auto initializer = variable->getChild("initializer");
        if (initializer) {
            oss << " = " << generate(initializer);
        }
        oss << ";";
        return oss.str();
    }

    std::string visitParameter(const Parameter* parameter) override {
        std::ostringstream oss;
        std::string typeStr = "Object";
        auto type = parameter->getChild("type");
        if (type) typeStr = generate(type);
        oss << typeStr << " " << parameter->name;
        return oss.str();
    }

    std::string visitAssignment(const Assignment* assignment) override {
        std::ostringstream oss;
        auto target = assignment->getChild("target");
        auto value = assignment->getChild("value");
        if (target) {
            oss << generate(target);
        } else {
            oss << "/* missing target */";
        }
        oss << " = ";
        if (value) {
            oss << generate(value);
        } else {
            oss << "null";
        }
        oss << ";";
        return oss.str();
    }

    std::string visitReturn(const Return* ret) override {
        std::ostringstream oss;
        oss << "return";
        auto value = ret->getChild("value");
        if (value) {
            oss << " " << generate(value);
        }
        oss << ";";
        return oss.str();
    }

    std::string visitBinaryOperation(const BinaryOperation* binOp) override {
        std::ostringstream oss;
        auto left = binOp->getChild("left");
        auto right = binOp->getChild("right");
        if (left) {
            oss << generate(left);
        } else {
            oss << "/* missing left */";
        }
        oss << " " << binOp->op << " ";
        if (right) {
            oss << generate(right);
        } else {
            oss << "/* missing right */";
        }
        return oss.str();
    }

    std::string visitVariableReference(const VariableReference* varRef) override {
        return varRef->variableName;
    }

    std::string visitIntegerLiteral(const IntegerLiteral* lit) override {
        return std::to_string(lit->value);
    }

    std::string visitFloatLiteral(const FloatLiteral* lit) override {
        return lit->value;
    }

    std::string visitStringLiteral(const StringLiteral* lit) override {
        if (isQuotedLiteral(lit->value)) return lit->value;
        return "\"" + lit->value + "\"";
    }

    std::string visitBooleanLiteral(const BooleanLiteral* lit) override {
        return lit->value ? "true" : "false";
    }

    std::string visitNullLiteral(const NullLiteral*) override {
        return "null";
    }

    std::string visitIfStatement(const IfStatement* stmt) override {
        std::ostringstream oss;
        auto condition = stmt->getChild("condition");
        auto thenBranch = stmt->getChildren("thenBranch");
        auto elseBranch = stmt->getChildren("elseBranch");

        oss << "if (";
        if (condition) {
            oss << generate(condition);
        } else {
            oss << "true";
        }
        oss << ") {\n";
        emitBody(oss, thenBranch, "    ");
        oss << "}";
        if (!elseBranch.empty()) {
            oss << " else {\n";
            emitBody(oss, elseBranch, "    ");
            oss << "}";
        }
        return oss.str();
    }

    std::string visitWhileLoop(const WhileLoop* loop) override {
        std::ostringstream oss;
        auto condition = loop->getChild("condition");
        auto body = loop->getChildren("body");
        oss << "while (";
        if (condition) {
            oss << generate(condition);
        } else {
            oss << "true";
        }
        oss << ") {\n";
        emitBody(oss, body, "    ");
        oss << "}";
        return oss.str();
    }

    std::string visitForLoop(const ForLoop* loop) override {
        std::ostringstream oss;
        auto iterable = loop->getChild("iterable");
        auto body = loop->getChildren("body");
        std::string iterName = loop->iteratorName.empty() ? "item" : loop->iteratorName;
        oss << "for (var " << iterName << " : ";
        if (iterable) {
            oss << generate(iterable);
        } else {
            oss << "java.util.List.of()";
        }
        oss << ") {\n";
        emitBody(oss, body, "    ");
        oss << "}";
        return oss.str();
    }

    std::string visitExpressionStatement(const ExpressionStatement* stmt) override {
        std::ostringstream oss;
        auto expr = stmt->getChild("expression");
        if (expr) {
            oss << generate(expr);
        }
        oss << ";";
        return oss.str();
    }

    std::string visitUnaryOperation(const UnaryOperation* unOp) override {
        std::ostringstream oss;
        std::string op = unOp->op;
        bool wordOp = !op.empty() && std::isalpha(static_cast<unsigned char>(op[0]));
        oss << op;
        if (wordOp) oss << " ";
        auto operand = unOp->getChild("operand");
        if (operand) {
            oss << generate(operand);
        } else {
            oss << "null";
        }
        return oss.str();
    }

    std::string visitFunctionCall(const FunctionCall* call) override {
        std::ostringstream oss;
        oss << call->functionName << "(";
        auto arguments = call->getChildren("arguments");
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(arguments[i]);
        }
        oss << ")";
        return oss.str();
    }

    std::string visitBlock(const Block* block) override {
        std::ostringstream oss;
        auto statements = block->getChildren("statements");
        oss << "{\n";
        emitBody(oss, statements, "    ");
        oss << "}";
        return oss.str();
    }

    std::string visitListLiteral(const ListLiteral* lit) override {
        std::ostringstream oss;
        oss << "java.util.List.of(";
        auto elements = lit->getChildren("elements");
        for (size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(elements[i]);
        }
        oss << ")";
        return oss.str();
    }

    std::string visitIndexAccess(const IndexAccess* access) override {
        std::ostringstream oss;
        auto target = access->getChild("target");
        auto index = access->getChild("index");
        if (target) {
            oss << generate(target);
        } else {
            oss << "/* missing target */";
        }
        oss << "[";
        if (index) {
            oss << generate(index);
        } else {
            oss << "0";
        }
        oss << "]";
        return oss.str();
    }

    std::string visitMemberAccess(const MemberAccess* access) override {
        std::ostringstream oss;
        auto target = access->getChild("target");
        if (target) {
            oss << generate(target);
        } else {
            oss << "/* missing target */";
        }
        oss << "." << access->memberName;
        return oss.str();
    }

    std::string visitPrimitiveType(const PrimitiveType* type) override {
        std::string kind = type->kind;
        if (kind == "string") return "String";
        if (kind == "bool") return "boolean";
        if (kind == "float") return "float";
        if (kind == "double") return "double";
        if (kind == "int" || kind == "long" || kind == "short" || kind == "byte") return kind;
        if (kind == "char") return "char";
        if (kind == "void") return "void";
        return kind;
    }

    std::string visitListType(const ListType* type) override {
        std::ostringstream oss;
        oss << "java.util.List<";
        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "Object";
        }
        oss << ">";
        return oss.str();
    }

    std::string visitSetType(const SetType* type) override {
        std::ostringstream oss;
        oss << "java.util.Set<";
        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "Object";
        }
        oss << ">";
        return oss.str();
    }

    std::string visitMapType(const MapType* type) override {
        std::ostringstream oss;
        oss << "java.util.Map<";
        auto keyType = type->getChild("keyType");
        auto valueType = type->getChild("valueType");
        if (keyType) {
            oss << generate(keyType);
        } else {
            oss << "Object";
        }
        oss << ", ";
        if (valueType) {
            oss << generate(valueType);
        } else {
            oss << "Object";
        }
        oss << ">";
        return oss.str();
    }

    std::string visitTupleType(const TupleType* type) override {
        std::ostringstream oss;
        oss << "java.util.List<";
        auto elementTypes = type->getChildren("elementTypes");
        if (!elementTypes.empty()) {
            oss << generate(elementTypes[0]);
        } else {
            oss << "Object";
        }
        oss << ">";
        return oss.str();
    }

    std::string visitArrayType(const ArrayType* type) override {
        std::ostringstream oss;
        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "Object";
        }
        oss << "[]";
        return oss.str();
    }

    std::string visitOptionalType(const OptionalType* type) override {
        std::ostringstream oss;
        oss << "java.util.Optional<";
        auto inner = type->getChild("innerType");
        if (inner) {
            oss << generate(inner);
        } else {
            oss << "Object";
        }
        oss << ">";
        return oss.str();
    }

    std::string visitCustomType(const CustomType* type) override {
        return type->typeName;
    }

    // --- New AST node visitors (Step 308) ---

    std::string visitClassDeclaration(const ASTNode* node) override {
        auto* cls = static_cast<const ClassDeclaration*>(node);
        std::ostringstream oss;
        auto annotations = cls->getChildren("annotations");
        for (const auto* a : annotations) oss << generate(a) << "\n";
        if (cls->isAbstract) oss << "abstract ";
        oss << "class " << cls->name;
        auto bases = cls->getBases();
        if (!bases.empty()) {
            // First base → extends, rest → implements (Java adaptation)
            oss << " extends " << bases[0].name;
            if (bases.size() > 1) {
                oss << " implements ";
                for (size_t i = 1; i < bases.size(); ++i) {
                    if (i > 1) oss << ", ";
                    oss << bases[i].name;
                }
            }
        }
        auto interfaces = cls->getChildren("interfaces");
        if (!interfaces.empty()) {
            oss << (bases.size() > 1 ? ", " : " implements ");
            for (size_t i = 0; i < interfaces.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << generate(interfaces[i]);
            }
        }
        oss << " {\n";
        auto fields = cls->getChildren("fields");
        for (const auto* f : fields)
            oss << "    " << generate(f) << "\n";
        if (!fields.empty() && !cls->getChildren("methods").empty()) oss << "\n";
        auto methods = cls->getChildren("methods");
        for (const auto* m : methods) oss << generate(m) << "\n";
        oss << "}\n";
        return oss.str();
    }

    std::string visitInterfaceDeclaration(const ASTNode* node) override {
        auto* iface = static_cast<const InterfaceDeclaration*>(node);
        std::ostringstream oss;
        oss << "interface " << iface->name << " {\n";
        auto methods = iface->getChildren("methods");
        for (const auto* m : methods) {
            auto* meth = static_cast<const MethodDeclaration*>(m);
            oss << "    void " << meth->name << "();\n";
        }
        oss << "}\n";
        return oss.str();
    }

    std::string visitMethodDeclaration(const ASTNode* node) override {
        auto* meth = static_cast<const MethodDeclaration*>(node);
        std::ostringstream oss;
        auto annotations = meth->getChildren("annotations");
        for (const auto* a : annotations) oss << "    " << generate(a) << "\n";
        oss << "    ";
        if (!meth->visibility.empty()) oss << meth->visibility << " ";
        if (meth->isStatic) oss << "static ";
        std::string returnType = "void";
        auto retType = meth->getChild("returnType");
        if (retType) returnType = generate(retType);
        oss << returnType << " " << meth->name << "(";
        emitParameters(oss, meth->getChildren("parameters"));
        oss << ")";
        if (meth->isOverride) oss << " /* @Override */";
        auto body = meth->getChildren("body");
        if (body.empty()) {
            oss << " {}\n";
        } else {
            oss << " {\n";
            emitBody(oss, body, "        ");
            oss << "    }\n";
        }
        return oss.str();
    }

    std::string visitGenericType(const ASTNode* node) override {
        auto* gen = static_cast<const GenericType*>(node);
        std::ostringstream oss;
        oss << gen->baseName << "<";
        auto params = gen->getChildren("typeParameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(params[i]);
        }
        oss << ">";
        return oss.str();
    }

    std::string visitTypeParameter(const ASTNode* node) override {
        auto* tp = static_cast<const TypeParameter*>(node);
        if (!tp->constraint.empty()) return tp->name + " extends " + tp->constraint;
        return tp->name;
    }

    std::string visitAsyncFunction(const ASTNode* node) override {
        auto* af = static_cast<const AsyncFunction*>(node);
        std::ostringstream oss;
        auto annotations = af->getChildren("annotations");
        for (const auto* a : annotations) oss << generate(a) << "\n";
        oss << "CompletableFuture<Void> " << af->name << "(";
        emitParameters(oss, af->getChildren("parameters"));
        oss << ") {\n";
        emitBody(oss, af->getChildren("body"), "    ");
        oss << "}\n";
        return oss.str();
    }

    std::string visitAwaitExpression(const ASTNode* node) override {
        auto* aw = static_cast<const AwaitExpression*>(node);
        auto* expr = aw->getChild("expression");
        return (expr ? generate(expr) : "/* missing */") + ".get()";
    }

    std::string visitLambdaExpression(const ASTNode* node) override {
        auto* lam = static_cast<const LambdaExpression*>(node);
        std::ostringstream oss;
        oss << "(";
        auto params = lam->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << static_cast<const Parameter*>(params[i])->name;
        }
        oss << ") -> ";
        auto body = lam->getChildren("body");
        if (body.size() == 1) {
            oss << generate(body[0]);
        } else if (body.empty()) {
            oss << "{}";
        } else {
            oss << "{ ";
            for (const auto* s : body) oss << generate(s) << "; ";
            oss << "}";
        }
        return oss.str();
    }

    std::string visitDecoratorAnnotation(const ASTNode* node) override {
        auto* dec = static_cast<const DecoratorAnnotation*>(node);
        std::ostringstream oss;
        oss << "@" << dec->name;
        auto args = dec->getChildren("arguments");
        if (!args.empty()) {
            oss << "(";
            for (size_t i = 0; i < args.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << generate(args[i]);
            }
            oss << ")";
        }
        return oss.str();
    }

    std::string visitDerefStrategy(const DerefStrategy* annotation) override {
        return "// @deref(" + annotation->strategy + ")";
    }

    std::string visitOptimizationLock(const OptimizationLock* annotation) override {
        return "// @lock(" + annotation->lockedBy + ") - " + annotation->lockReason;
    }

    std::string visitLangSpecific(const LangSpecific* annotation) override {
        if (annotation->language == "java" && !annotation->rawSyntax.empty()) {
            return "// " + annotation->rawSyntax;
        }
        return "// @lang_specific(" + annotation->language + ", " + annotation->idiomType + ")";
    }

    std::string visitDeallocateAnnotation(const DeallocateAnnotation* annotation) override {
        return "// @dealloc(" + annotation->strategy + ")";
    }

    std::string visitLifetimeAnnotation(const LifetimeAnnotation* annotation) override {
        return "// @lifetime(" + annotation->strategy + ")";
    }

    std::string visitReclaimAnnotation(const ReclaimAnnotation* annotation) override {
        return "// @reclaim(" + annotation->strategy + ") - GC-managed";
    }

    std::string visitOwnerAnnotation(const OwnerAnnotation* annotation) override {
        if (annotation->strategy == "Single") {
            return "// @owner(Single) - AutoCloseable/try-with-resources";
        }
        return "// @owner(" + annotation->strategy + ")";
    }

    std::string visitAllocateAnnotation(const AllocateAnnotation* annotation) override {
        return "// @allocate(" + annotation->strategy + ")";
    }

    std::string visitHotColdAnnotation(const HotColdAnnotation* annotation) override {
        return "// @" + annotation->hint;
    }

    std::string visitInlineAnnotation(const InlineAnnotation* annotation) override {
        return "// @inline(" + annotation->mode + ")";
    }

    std::string visitPureAnnotation(const PureAnnotation*) override {
        return "// @pure";
    }

    std::string visitConstExprAnnotation(const ConstExprAnnotation*) override {
        return "// @constexpr";
    }

    // --- Preprocessor/Enum/Namespace visitors (Step 340) ---

    std::string visitIncludeDirective(const ASTNode* node) override {
        auto* inc = static_cast<const IncludeDirective*>(node);
        std::string mod = inc->path;
        auto dot = mod.rfind('.');
        if (dot != std::string::npos) mod = mod.substr(0, dot);
        for (auto& c : mod) { if (c == '/' || c == '\\') c = '.'; }
        return "import " + mod + ";";
    }

    std::string visitPragmaDirective(const ASTNode* node) override {
        auto* prag = static_cast<const PragmaDirective*>(node);
        return "// pragma " + prag->directive;
    }

    std::string visitMacroDefinition(const ASTNode* node) override {
        auto* mac = static_cast<const MacroDefinition*>(node);
        return "public static final var " + mac->name + " = " + (mac->body.empty() ? "null" : mac->body) + ";";
    }

    std::string visitEnumDeclaration(const ASTNode* node) override {
        auto* e = static_cast<const EnumDeclaration*>(node);
        std::ostringstream oss;
        oss << "enum " << e->name << " {\n";
        auto members = e->getChildren("members");
        for (size_t i = 0; i < members.size(); ++i) {
            auto* m = static_cast<const EnumMember*>(members[i]);
            oss << "    " << m->name;
            if (i + 1 < members.size()) oss << ",";
            oss << "\n";
        }
        oss << "}\n";
        return oss.str();
    }

    std::string visitNamespaceDeclaration(const ASTNode* node) override {
        auto* ns = static_cast<const NamespaceDeclaration*>(node);
        std::ostringstream oss;
        oss << "package " << ns->name << ";\n";
        auto body = ns->getChildren("body");
        for (const auto* child : body)
            oss << generate(child) << "\n";
        return oss.str();
    }

    std::string visitTypeAlias(const ASTNode* node) override {
        auto* ta = static_cast<const TypeAlias*>(node);
        return "// type alias: " + ta->aliasName + " = " + ta->targetType;
    }

private:
    static std::string emitImport(const Import* imp) {
        std::string moduleName = imp->moduleName.empty() ? "module" : imp->moduleName;
        if (imp->importKind == "require") {
            return "// require " + moduleName;
        }
        return "import " + moduleName + ";";
    }

    static void emitAnnotations(std::ostringstream& oss,
                                const std::vector<ASTNode*>& annotations,
                                const std::string& indent) {
        for (const auto* annotation : annotations) {
            if (!annotation) continue;
            std::string code;
            if (annotation->conceptType == "LangSpecific") {
                auto* lang = static_cast<const LangSpecific*>(annotation);
                if (lang->language == "java" && !lang->rawSyntax.empty()) {
                    code = "// " + lang->rawSyntax;
                }
            }
            if (code.empty()) {
                JavaGenerator gen;
                code = gen.generate(annotation);
            }
            if (!code.empty()) {
                oss << indent << code << "\n";
            }
        }
    }

    void emitParameters(std::ostringstream& oss,
                        const std::vector<ASTNode*>& params) {
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << visitParameter(static_cast<const Parameter*>(params[i]));
        }
    }

    void emitBody(std::ostringstream& oss,
                  const std::vector<ASTNode*>& body,
                  const std::string& indent) {
        if (body.empty()) {
            oss << indent << "// TODO: implement\n";
            return;
        }
        for (const auto* stmt : body) {
            std::string stmtCode = generate(stmt);
            appendIndented(oss, stmtCode, indent);
            oss << "\n";
        }
    }

    void emitClass(std::ostringstream& oss,
                   const std::string& className,
                   const std::vector<const Variable*>& fields,
                   const std::vector<const Function*>& methods) {
        oss << "class " << className << " {\n";
        for (const auto* field : fields) {
            oss << "    " << visitVariable(field) << "\n";
        }
        if (!fields.empty() && !methods.empty()) oss << "\n";
        for (const auto* method : methods) {
            emitMethod(oss, className, method);
            oss << "\n";
        }
        oss << "}";
    }

    void emitMethod(std::ostringstream& oss,
                    const std::string& className,
                    const Function* method) {
        std::string methodName = methodNameFromQualified(method->name, className);
        bool isConstructor = (methodName == "constructor" || methodName == className);

        emitAnnotations(oss, method->getChildren("annotations"), "    ");
        oss << "    ";
        if (!isConstructor) {
            std::string returnType = "void";
            auto retType = method->getChild("returnType");
            if (retType) returnType = generate(retType);
            oss << returnType << " ";
            oss << methodName;
        } else {
            oss << className;
        }
        oss << "(";
        emitParameters(oss, method->getChildren("parameters"));
        oss << ") {\n";
        emitBody(oss, method->getChildren("body"), "        ");
        oss << "    }";
    }

    static void appendIndented(std::ostringstream& oss,
                               const std::string& code,
                               const std::string& indent) {
        size_t pos = 0;
        while (pos < code.length()) {
            size_t newlinePos = code.find('\n', pos);
            if (newlinePos == std::string::npos) {
                oss << indent << code.substr(pos);
                break;
            }
            oss << indent << code.substr(pos, newlinePos - pos) << "\n";
            pos = newlinePos + 1;
            if (pos >= code.length()) break;
        }
    }

    static std::string methodNameFromQualified(const std::string& name,
                                               const std::string& className) {
        if (name.rfind(className + ".", 0) == 0) {
            return name.substr(className.size() + 1);
        }
        auto pos = name.find('.');
        if (pos != std::string::npos && pos + 1 < name.size()) {
            return name.substr(pos + 1);
        }
        return name;
    }

    static std::string stripClassPrefix(const std::string& name) {
        auto pos = name.find('.');
        if (pos != std::string::npos && pos + 1 < name.size()) {
            return name.substr(pos + 1);
        }
        return name;
    }

    static bool isQuotedLiteral(const std::string& text) {
        if (text.size() < 2) return false;
        char first = text.front();
        char last = text.back();
        if ((first == '\'' && last == '\'') || (first == '"' && last == '"')) {
            return true;
        }
        return false;
    }
};
