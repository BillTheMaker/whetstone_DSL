#pragma once
#include "PolyglotProjectSpec.h"

namespace whetstone {

class PolySortProject {
public:
    // Returns a fully-populated poly-sort PolyglotProjectSpec.
    // sort-core (TreeRecursive/SharedMemory/Async/SimpleGenerics) → Rust
    // data-gen  (FlatLoop/None/Blocking/None, high mutation)       → Python
    static PolyglotProjectSpec make() {
        PolyglotProjectSpec spec;
        spec.projectName = "poly-sort";

        PolyglotSection sortCore;
        sortCore.componentName                   = "sort-core";
        sortCore.features.mutationRatio          = 0.15f;
        sortCore.features.recursionShape         = ASTFeatures::RecursionShape::TreeRecursive;
        sortCore.features.concurrencyPrimitive   = ASTFeatures::ConcurrencyPrimitive::SharedMemory;
        sortCore.features.ioPattern              = ASTFeatures::IOPattern::Async;
        sortCore.features.typeComplexity         = ASTFeatures::TypeComplexity::SimpleGenerics;
        spec.sections.push_back(sortCore);

        PolyglotSection dataGen;
        dataGen.componentName                    = "data-gen";
        dataGen.features.mutationRatio           = 0.55f;
        dataGen.features.recursionShape          = ASTFeatures::RecursionShape::FlatLoop;
        dataGen.features.concurrencyPrimitive    = ASTFeatures::ConcurrencyPrimitive::None;
        dataGen.features.ioPattern               = ASTFeatures::IOPattern::Blocking;
        dataGen.features.typeComplexity          = ASTFeatures::TypeComplexity::None;
        spec.sections.push_back(dataGen);

        PolyglotInterface iface;
        iface.fromComponent = "data-gen";
        iface.toComponent   = "sort-core";
        iface.description   = "raw integer slice";
        spec.interfaces.push_back(iface);

        return spec;
    }
};

} // namespace whetstone
