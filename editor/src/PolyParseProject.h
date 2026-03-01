#pragma once
#include "PolyglotProjectSpec.h"

namespace whetstone {

class PolyParseProject {
public:
    // Returns a fully-populated poly-parse PolyglotProjectSpec.
    // lexer         (FlatLoop/None/Blocking/SimpleGenerics, high mutation) → C++
    // ast-transform (TailRecursive/None/None/DependentTypes, no mutation)  → Haskell
    static PolyglotProjectSpec make() {
        PolyglotProjectSpec spec;
        spec.projectName = "poly-parse";

        PolyglotSection lexer;
        lexer.componentName                   = "lexer";
        lexer.features.mutationRatio          = 0.35f;
        lexer.features.recursionShape         = ASTFeatures::RecursionShape::TreeRecursive;
        lexer.features.concurrencyPrimitive   = ASTFeatures::ConcurrencyPrimitive::SharedMemory;
        lexer.features.ioPattern              = ASTFeatures::IOPattern::Blocking;
        lexer.features.typeComplexity         = ASTFeatures::TypeComplexity::SimpleGenerics;
        spec.sections.push_back(lexer);

        PolyglotSection astTransform;
        astTransform.componentName                   = "ast-transform";
        astTransform.features.mutationRatio          = 0.0f;
        astTransform.features.recursionShape         = ASTFeatures::RecursionShape::TailRecursive;
        astTransform.features.concurrencyPrimitive   = ASTFeatures::ConcurrencyPrimitive::None;
        astTransform.features.ioPattern              = ASTFeatures::IOPattern::None;
        astTransform.features.typeComplexity         = ASTFeatures::TypeComplexity::DependentTypes;
        spec.sections.push_back(astTransform);

        PolyglotInterface iface;
        iface.fromComponent = "lexer";
        iface.toComponent   = "ast-transform";
        iface.description   = "token stream";
        spec.interfaces.push_back(iface);

        return spec;
    }
};

} // namespace whetstone
