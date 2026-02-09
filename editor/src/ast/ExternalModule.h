#pragma once
#include "ASTNode.h"

class ExternalModule : public ASTNode {
public:
    std::string name;
    std::string version;
    std::string language;

    ExternalModule() { conceptType = "ExternalModule"; }
    ExternalModule(const std::string& id, const std::string& name,
                   const std::string& language,
                   const std::string& version = "")
        : name(name), version(version), language(language) {
        this->id = id;
        this->conceptType = "ExternalModule";
    }
    // children: signatures (0..n) via addChild("signatures", TypeSignature*)
};
