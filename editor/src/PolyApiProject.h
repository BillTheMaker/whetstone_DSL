#pragma once
#include "PolyglotProjectSpec.h"

namespace whetstone {

class PolyApiProject {
public:
    // Returns a fully-populated poly-api PolyglotProjectSpec.
    // http-server (FlatLoop/Channels/Blocking/None)         → Go
    // api-client  (FlatLoop/None/Async/SimpleGenerics)      → TypeScript
    static PolyglotProjectSpec make() {
        PolyglotProjectSpec spec;
        spec.projectName = "poly-api";

        PolyglotSection httpServer;
        httpServer.componentName                   = "http-server";
        httpServer.features.mutationRatio          = 0.30f;
        httpServer.features.recursionShape         = ASTFeatures::RecursionShape::FlatLoop;
        httpServer.features.concurrencyPrimitive   = ASTFeatures::ConcurrencyPrimitive::Channels;
        httpServer.features.ioPattern              = ASTFeatures::IOPattern::Blocking;
        httpServer.features.typeComplexity         = ASTFeatures::TypeComplexity::None;
        spec.sections.push_back(httpServer);

        PolyglotSection apiClient;
        apiClient.componentName                    = "api-client";
        apiClient.features.mutationRatio           = 0.20f;
        apiClient.features.recursionShape          = ASTFeatures::RecursionShape::FlatLoop;
        apiClient.features.concurrencyPrimitive    = ASTFeatures::ConcurrencyPrimitive::None;
        apiClient.features.ioPattern               = ASTFeatures::IOPattern::Async;
        apiClient.features.typeComplexity          = ASTFeatures::TypeComplexity::SimpleGenerics;
        spec.sections.push_back(apiClient);

        PolyglotInterface iface;
        iface.fromComponent = "api-client";
        iface.toComponent   = "http-server";
        iface.description   = "JSON REST calls";
        spec.interfaces.push_back(iface);

        return spec;
    }
};

} // namespace whetstone
