#pragma once
// Step 51: Text-AST bidirectional synchronization
//
// TextASTSync bridges raw text editing and the structured AST:
//   setText() → stores text, marks parse pending (debounce)
//   syncNow() → parses text via TreeSitterParser → updates AST
//   getText() → regenerates text from current AST via generator
//   getAST()  → returns current Module*
//   setAST()  → replaces AST directly (structured edit path)

#include "ast/Parser.h"
#include "ast/Generator.h"
#include <string>
#include <memory>

class TextASTSync {
public:
    void setText(const std::string& text, const std::string& language) {
        text_ = text;
        language_ = language;
        parsePending_ = true;
    }

    Module* getAST() const {
        return module_.get();
    }

    void setAST(std::unique_ptr<Module> module) {
        module_ = std::move(module);
        parsePending_ = false;
    }

    std::string getText() const {
        if (!module_) return text_;
        if (language_ == "python") {
            PythonGenerator gen;
            return gen.generate(module_.get());
        } else if (language_ == "cpp") {
            CppGenerator gen;
            return gen.generate(module_.get());
        } else if (language_ == "elisp") {
            ElispGenerator gen;
            return gen.generate(module_.get());
        }
        return text_;
    }

    void syncNow() {
        if (!parsePending_) return;
        if (language_ == "python") {
            module_ = TreeSitterParser::parsePython(text_);
        } else if (language_ == "cpp") {
            module_ = TreeSitterParser::parseCpp(text_);
        } else if (language_ == "elisp") {
            module_ = TreeSitterParser::parseElisp(text_);
        } else if (language_ == "javascript") {
            module_ = TreeSitterParser::parseJavaScript(text_);
        } else if (language_ == "typescript") {
            module_ = TreeSitterParser::parseTypeScript(text_);
        }
        parsePending_ = false;
    }

    bool isParsePending() const { return parsePending_; }

    const std::string& getLanguage() const { return language_; }

private:
    std::string text_;
    std::string language_;
    std::unique_ptr<Module> module_;
    bool parsePending_ = false;
};
