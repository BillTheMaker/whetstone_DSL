#pragma once
#include "ASTFeatureExtractor.h"
#include <string>
#include <vector>

namespace whetstone {

struct LanguageIdiomProfile {
    std::string language;
    float idealMutationRatio;
    ASTFeatures::RecursionShape idealRecursion;
    ASTFeatures::ConcurrencyPrimitive idealConcurrency;
    ASTFeatures::IOPattern idealIO;
    ASTFeatures::TypeComplexity idealTypeComplexity;

    static std::vector<LanguageIdiomProfile> allProfiles() {
        using RS = ASTFeatures::RecursionShape;
        using CP = ASTFeatures::ConcurrencyPrimitive;
        using IO = ASTFeatures::IOPattern;
        using TC = ASTFeatures::TypeComplexity;
        return {
            {"Python",     0.40f, RS::FlatLoop,      CP::None,        IO::Blocking,   TC::None          },
            {"Rust",       0.20f, RS::TreeRecursive, CP::SharedMemory, IO::Async,      TC::SimpleGenerics},
            {"Go",         0.30f, RS::FlatLoop,      CP::Channels,    IO::Blocking,   TC::None          },
            {"Haskell",    0.00f, RS::TailRecursive, CP::None,        IO::None,       TC::DependentTypes},
            {"C++",        0.30f, RS::TreeRecursive, CP::SharedMemory, IO::Blocking,  TC::SimpleGenerics},
            {"TypeScript", 0.30f, RS::FlatLoop,      CP::None,        IO::Async,      TC::SimpleGenerics},
            {"Elixir",     0.10f, RS::TailRecursive, CP::Actors,      IO::EventDriven,TC::None          },
            {"Lisp",       0.05f, RS::TailRecursive, CP::None,        IO::None,       TC::None          },
        };
    }
};

} // namespace whetstone
