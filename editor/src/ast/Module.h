#pragma once
#include "ASTNode.h"

class Module : public ASTNode {
public:
    std::string name;
    std::string targetLanguage;

    Module() { conceptType = "Module"; }
    Module(const std::string& id, const std::string& name, const std::string& lang)
        : name(name), targetLanguage(lang) {
        this->id = id;
        this->conceptType = "Module";
    }
};
