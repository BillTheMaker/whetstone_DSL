#pragma once
#include "Pipeline.h"
#include "AnnotationInference.h"
#include "SemannoFormat.h"
#include <string>
#include <vector>

struct TrainingPair {
    std::string rawCode;
    std::string annotatedCode;
    std::string language;
    std::vector<std::string> annotations;  // list of annotation types applied
};

class TrainingDataGenerator {
public:
    TrainingPair generate(const std::string& source,
                          const std::string& language) {
        TrainingPair pair;
        pair.rawCode = source;
        pair.language = language;

        Pipeline pipeline;
        std::vector<ParseDiagnostic> diags;
        auto mod = pipeline.parse(source, language, diags);
        if (!mod) {
            pair.annotatedCode = source;
            return pair;
        }

        // Infer annotations
        AnnotationInference inferrer;
        auto inferred = inferrer.inferAll(mod.get());

        // Apply inferred annotations to AST
        for (const auto& inf : inferred) {
            ASTNode* target = findNodeById(mod.get(), inf.nodeId);
            if (!target) continue;

            Annotation* anno = createAnnotation(inf);
            if (anno) {
                target->addChild("annotations", anno);
                pair.annotations.push_back(inf.annotationType);
            }
        }

        // Generate annotated code
        pair.annotatedCode = pipeline.generate(mod.get(), language);

        return pair;
    }

    std::vector<TrainingPair> generateBatch(
        const std::vector<std::pair<std::string, std::string>>& samples) {
        std::vector<TrainingPair> pairs;
        for (const auto& [source, lang] : samples) {
            pairs.push_back(generate(source, lang));
        }
        return pairs;
    }

private:
    Annotation* createAnnotation(const AnnotationInference::InferredAnnotation& inf) {
        if (inf.annotationType == "ExecAnnotation") {
            auto* a = new ExecAnnotation();
            a->id = "inferred_" + inf.nodeId;
            a->mode = inf.value;
            return a;
        }
        if (inf.annotationType == "PureAnnotation") {
            auto* a = new PureAnnotation();
            a->id = "inferred_" + inf.nodeId;
            return a;
        }
        if (inf.annotationType == "TailCallAnnotation") {
            auto* a = new TailCallAnnotation();
            a->id = "inferred_" + inf.nodeId;
            return a;
        }
        if (inf.annotationType == "VisibilityAnnotation") {
            auto* a = new VisibilityAnnotation();
            a->id = "inferred_" + inf.nodeId;
            a->level = inf.value;
            return a;
        }
        if (inf.annotationType == "ExceptionAnnotation") {
            auto* a = new ExceptionAnnotation();
            a->id = "inferred_" + inf.nodeId;
            a->style = inf.value;
            return a;
        }
        if (inf.annotationType == "BlockingAnnotation") {
            auto* a = new BlockingAnnotation();
            a->id = "inferred_" + inf.nodeId;
            a->kind = inf.value;
            return a;
        }
        if (inf.annotationType == "ParallelAnnotation") {
            auto* a = new ParallelAnnotation();
            a->id = "inferred_" + inf.nodeId;
            a->kind = inf.value;
            return a;
        }
        if (inf.annotationType == "LoopAnnotation") {
            auto* a = new LoopAnnotation();
            a->id = "inferred_" + inf.nodeId;
            a->hint = inf.value;
            return a;
        }
        if (inf.annotationType == "ComplexityAnnotation") {
            auto* a = new ComplexityAnnotation();
            a->id = "inferred_" + inf.nodeId;
            a->timeComplexity = inf.value;
            return a;
        }
        // Memory annotations - delegate
        if (inf.annotationType == "ReclaimAnnotation") {
            auto* a = new ReclaimAnnotation();
            a->id = "inferred_" + inf.nodeId;
            a->strategy = inf.value;
            return a;
        }
        if (inf.annotationType == "LifetimeAnnotation") {
            auto* a = new LifetimeAnnotation();
            a->id = "inferred_" + inf.nodeId;
            a->strategy = inf.value;
            return a;
        }
        if (inf.annotationType == "OwnerAnnotation") {
            auto* a = new OwnerAnnotation();
            a->id = "inferred_" + inf.nodeId;
            a->strategy = inf.value;
            return a;
        }
        if (inf.annotationType == "DeallocateAnnotation") {
            auto* a = new DeallocateAnnotation();
            a->id = "inferred_" + inf.nodeId;
            a->strategy = inf.value;
            return a;
        }
        return nullptr;
    }
};
