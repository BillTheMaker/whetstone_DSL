#pragma once
#include <string>
#include <vector>
#include <memory>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"
#include "ast/Parser.h"
#include "ast/Generator.h"
#include "MemoryStrategyInference.h"
#include "AnnotationValidator.h"
#include "TransformEngine.h"
#include "StrategyValidator.h"
#include "CrossLanguageProjector.h"

class Pipeline {
public:
    struct PipelineResult {
        std::unique_ptr<Module> ast;
        std::vector<MemoryStrategyInference::Suggestion> suggestions;
        std::vector<AnnotationValidator::Diagnostic> validationDiags;
        std::vector<StrategyValidator::Violation> violations;
        TransformEngine::TransformResult foldResult;
        TransformEngine::TransformResult dceResult;
        std::string generatedCode;
        std::vector<ParseDiagnostic> parseDiags;
        bool success = false;
    };

    // Full pipeline: parse → infer → validate → optimize → generate
    PipelineResult run(const std::string& source,
                       const std::string& sourceLanguage,
                       const std::string& targetLanguage) {
        PipelineResult result;

        // 1. Parse
        result.ast = parse(source, sourceLanguage, result.parseDiags);
        if (!result.ast) return result;

        // 2. Infer memory annotations
        MemoryStrategyInference inferrer;
        result.suggestions = inferrer.inferAnnotations(result.ast.get());

        // 3. Apply top suggestion as annotation (if any)
        applyTopSuggestion(result.ast.get(), result.suggestions);

        // 4. Validate annotations
        AnnotationValidator annoValidator;
        result.validationDiags = annoValidator.validate(result.ast.get());

        // 5. Optimize (constant folding + DCE)
        TransformEngine engine;
        engine.setRoot(result.ast.get());
        result.foldResult = engine.constantFolding();
        result.dceResult = engine.deadCodeElimination();

        // 6. Validate post-optimization invariants
        StrategyValidator stratValidator;
        result.violations = stratValidator.validateInvariants(result.ast.get());

        // 7. Project to target language if different
        Module* genSource = result.ast.get();
        std::unique_ptr<Module> projected;
        if (sourceLanguage != targetLanguage) {
            CrossLanguageProjector projector;
            projected = projector.project(result.ast.get(), targetLanguage);
            genSource = projected.get();
        }

        // 8. Generate code
        result.generatedCode = generate(genSource, targetLanguage);
        result.success = true;
        return result;
    }

    // Parse-only step (exposed for partial pipeline use)
    std::unique_ptr<Module> parse(const std::string& source,
                                  const std::string& language,
                                  std::vector<ParseDiagnostic>& diags) {
        if (language == "python") {
            auto pr = TreeSitterParser::parsePythonWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "cpp") {
            auto pr = TreeSitterParser::parseCppWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "elisp") {
            auto pr = TreeSitterParser::parseElispWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "javascript") {
            auto pr = TreeSitterParser::parseJavaScriptWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "typescript") {
            auto pr = TreeSitterParser::parseTypeScriptWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "java") {
            auto pr = TreeSitterParser::parseJavaWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "rust") {
            auto pr = TreeSitterParser::parseRustWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "go") {
            auto pr = TreeSitterParser::parseGoWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "kotlin") {
            auto pr = KotlinParser::parseKotlinWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "csharp") {
            auto pr = CSharpParser::parseCSharpWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "fsharp" || language == "f#" || language == "fs") {
            auto pr = FSharpParser::parseFSharpWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "vbnet" || language == "vb" || language == "vb.net") {
            auto pr = VBNetParser::parseVBNetWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "postgresql" || language == "postgres") {
            auto pr = PostgreSQLParser::parsePostgreSQLWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "c") {
            auto pr = CParser::parseCWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "wat" || language == "wasm") {
            auto pr = WatParser::parseWatWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "common-lisp" || language == "commonlisp" ||
                   language == "lisp" || language == "cl") {
            auto pr = CommonLispParser::parseCommonLispWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        } else if (language == "scheme" || language == "scm") {
            auto pr = SchemeParser::parseSchemeWithDiagnostics(source);
            diags = std::move(pr.diagnostics);
            return std::move(pr.module);
        }
        return nullptr;
    }

    // Generate-only step
    std::string generate(const ASTNode* ast, const std::string& language) {
        if (!ast) return "";
        if (language == "python") {
            PythonGenerator gen;
            return gen.generate(ast);
        } else if (language == "cpp") {
            CppGenerator gen;
            return gen.generate(ast);
        } else if (language == "elisp") {
            ElispGenerator gen;
            return gen.generate(ast);
        } else if (language == "javascript") {
            JavaScriptGenerator gen;
            return gen.generate(ast);
        } else if (language == "typescript") {
            TypeScriptGenerator gen;
            return gen.generate(ast);
        } else if (language == "java") {
            JavaGenerator gen;
            return gen.generate(ast);
        } else if (language == "rust") {
            RustGenerator gen;
            return gen.generate(ast);
        } else if (language == "go") {
            GoGenerator gen;
            return gen.generate(ast);
        } else if (language == "kotlin") {
            KotlinGenerator gen;
            return gen.generate(ast);
        } else if (language == "csharp") {
            CSharpGenerator gen;
            return gen.generate(ast);
        } else if (language == "fsharp" || language == "f#" || language == "fs") {
            FSharpGenerator gen;
            return gen.generate(ast);
        } else if (language == "vbnet" || language == "vb" || language == "vb.net") {
            VBNetGenerator gen;
            return gen.generate(ast);
        } else if (language == "postgresql" || language == "postgres") {
            PostgreSQLGenerator gen;
            return gen.generate(ast);
        } else if (language == "c") {
            CGenerator gen;
            return gen.generate(ast);
        } else if (language == "wat" || language == "wasm") {
            WatGenerator gen;
            return gen.generate(ast);
        } else if (language == "common-lisp" || language == "commonlisp" ||
                   language == "lisp" || language == "cl") {
            CommonLispGenerator gen;
            return gen.generate(ast);
        } else if (language == "scheme" || language == "scm") {
            SchemeGenerator gen;
            return gen.generate(ast);
        }
        return "";
    }

private:
    void applyTopSuggestion(Module* mod,
                            const std::vector<MemoryStrategyInference::Suggestion>& suggestions) {
        if (suggestions.empty()) return;

        // Apply the module-level suggestion with highest confidence
        for (const auto& s : suggestions) {
            if (s.nodeId == mod->id && s.confidence >= 0.5) {
                Annotation* anno = createAnnotation(s);
                if (anno) {
                    mod->addChild("annotations", anno);
                }
                break;
            }
        }
    }

    Annotation* createAnnotation(const MemoryStrategyInference::Suggestion& s) {
        if (s.annotationType == "ReclaimAnnotation") {
            auto* a = new ReclaimAnnotation();
            a->id = "inferred_" + s.nodeId;
            a->strategy = s.strategy;
            return a;
        } else if (s.annotationType == "LifetimeAnnotation") {
            auto* a = new LifetimeAnnotation();
            a->id = "inferred_" + s.nodeId;
            a->strategy = s.strategy;
            return a;
        } else if (s.annotationType == "DeallocateAnnotation") {
            auto* a = new DeallocateAnnotation();
            a->id = "inferred_" + s.nodeId;
            a->strategy = s.strategy;
            return a;
        } else if (s.annotationType == "OwnerAnnotation") {
            auto* a = new OwnerAnnotation();
            a->id = "inferred_" + s.nodeId;
            a->strategy = s.strategy;
            return a;
        }
        return nullptr;
    }
};
