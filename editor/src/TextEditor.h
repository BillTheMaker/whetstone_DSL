#pragma once
// Step 53: Classical text editing with AST integration
//
// TextEditor provides a text buffer with edit operations that stay
// synchronized with the AST.  Every mutation (insert, delete, replace,
// replaceAll) pushes the previous text onto an undo stack, modifies the
// text buffer, then re-parses to update the AST.  Undo/redo restores
// the text and re-parses.

#include "ast/Parser.h"
#include "ast/Generator.h"
#include <string>
#include <memory>
#include <vector>

class TextEditor {
public:
    void setContent(const std::string& text, const std::string& language) {
        text_ = text;
        language_ = language;
        undoStack_.clear();
        redoStack_.clear();
        reParse();
    }

    std::string getContent() const { return text_; }

    Module* getAST() const { return module_.get(); }

    // --- Edit operations ---------------------------------------------------

    void insertText(int position, const std::string& text) {
        pushUndo();
        text_.insert(position, text);
        reParse();
    }

    void deleteText(int position, int length) {
        pushUndo();
        text_.erase(position, length);
        reParse();
    }

    void replaceText(int position, int length, const std::string& replacement) {
        pushUndo();
        text_.erase(position, length);
        text_.insert(position, replacement);
        reParse();
    }

    // --- Undo / Redo -------------------------------------------------------

    void undo() {
        if (undoStack_.empty()) return;
        redoStack_.push_back(text_);
        text_ = undoStack_.back();
        undoStack_.pop_back();
        reParse();
    }

    void redo() {
        if (redoStack_.empty()) return;
        undoStack_.push_back(text_);
        text_ = redoStack_.back();
        redoStack_.pop_back();
        reParse();
    }

    bool canUndo() const { return !undoStack_.empty(); }
    bool canRedo() const { return !redoStack_.empty(); }

    // --- Find / Replace ----------------------------------------------------

    int find(const std::string& query, int startPos = 0) const {
        size_t pos = text_.find(query, startPos);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }

    int replaceAll(const std::string& query, const std::string& replacement) {
        pushUndo();
        int count = 0;
        size_t pos = 0;
        while ((pos = text_.find(query, pos)) != std::string::npos) {
            text_.erase(pos, query.length());
            text_.insert(pos, replacement);
            pos += replacement.length();
            ++count;
        }
        if (count > 0) {
            reParse();
        } else {
            undoStack_.pop_back(); // no change — discard undo entry
        }
        return count;
    }

    // --- Selection ---------------------------------------------------------

    std::string getSelection(int start, int length) const {
        return text_.substr(start, length);
    }

private:
    void pushUndo() {
        undoStack_.push_back(text_);
        redoStack_.clear();
    }

    void reParse() {
        if (language_ == "python") {
            module_ = TreeSitterParser::parsePython(text_);
        } else if (language_ == "cpp") {
            module_ = TreeSitterParser::parseCpp(text_);
        } else if (language_ == "elisp") {
            module_ = TreeSitterParser::parseElisp(text_);
        }
    }

    std::string text_;
    std::string language_;
    std::unique_ptr<Module> module_;
    std::vector<std::string> undoStack_;
    std::vector<std::string> redoStack_;
};
