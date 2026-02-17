#pragma once

#include "EditorState.h"
#include "StringUtils.h"

#include "imgui.h"
#include "SyntaxHighlighter.h"
#include "CodeEditorWidget.h"
#include "FileTree.h"
#include "WelcomeScreen.h"
#include "FileDialog.h"
#include "LSPClient.h"
#include "BufferManager.h"
#include "ast/Serialization.h"
#include "ast/Generator.h"
#include "ast/Annotation.h"

#include <string>
#include <vector>
#include <algorithm>
#include <climits>
#include <memory>

static int countLines(const std::string& text) {
    int lines = 1;
    for (char c : text) {
        if (c == '\n') ++lines;
    }
    return lines;
}

static std::string generateForLanguage(const Module* ast, const std::string& language) {
    if (!ast) return "";
    if (language == "python") {
        PythonGenerator gen;
        return gen.generate(ast);
    }
    if (language == "cpp") {
        CppGenerator gen;
        return gen.generate(ast);
    }
    if (language == "elisp") {
        ElispGenerator gen;
        return gen.generate(ast);
    }
    if (language == "javascript") {
        JavaScriptGenerator gen;
        return gen.generate(ast);
    }
    if (language == "typescript") {
        TypeScriptGenerator gen;
        return gen.generate(ast);
    }
    if (language == "java") {
        JavaGenerator gen;
        return gen.generate(ast);
    }
    if (language == "rust") {
        RustGenerator gen;
        return gen.generate(ast);
    }
    if (language == "go") {
        GoGenerator gen;
        return gen.generate(ast);
    }
    PythonGenerator gen;
    return gen.generate(ast);
}

static std::unique_ptr<Module> cloneModule(const Module* ast) {
    if (!ast) return nullptr;
    json j = toJson(ast);
    ASTNode* node = fromJson(j);
    if (!node || node->conceptType != "Module") return nullptr;
    return std::unique_ptr<Module>(static_cast<Module*>(node));
}

static bool isAnnotationNode(const ASTNode* node) {
    if (!node) return false;
    if (node->conceptType.find("Annotation") != std::string::npos) return true;
    if (node->conceptType == "DerefStrategy") return true;
    if (node->conceptType == "OptimizationLock") return true;
    if (node->conceptType == "LangSpecific") return true;
    return false;
}

static int countAnnotationNodes(const ASTNode* node) {
    if (!node) return 0;
    int count = isAnnotationNode(node) ? 1 : 0;
    for (auto* child : node->allChildren()) {
        count += countAnnotationNodes(child);
    }
    return count;
}

static std::string findOptimizationBlockReason(const ASTNode* node) {
    if (!node) return "";
    for (auto* anno : node->getChildren("annotations")) {
        if (anno->conceptType == "OwnerAnnotation") {
            auto* oa = static_cast<const OwnerAnnotation*>(anno);
            if (oa->strategy == "Single") {
                return "@Owner(Single) blocks optimization (aliasing risk)";
            }
        }
        if (anno->conceptType == "DeallocateAnnotation") {
            auto* da = static_cast<const DeallocateAnnotation*>(anno);
            if (da->strategy == "Explicit") {
                return "@Deallocate(Explicit) blocks optimization (reorder risk)";
            }
        }
        if (anno->conceptType == "AllocateAnnotation") {
            auto* aa = static_cast<const AllocateAnnotation*>(anno);
            if (aa->strategy == "Static") {
                return "@Allocate(Static) blocks optimization (dynamic alloc risk)";
            }
        }
    }
    for (auto* child : node->allChildren()) {
        std::string found = findOptimizationBlockReason(child);
        if (!found.empty()) return found;
    }
    return "";
}

static std::string findOptimizationLockWarning(const ASTNode* node) {
    if (!node) return "";
    if (node->conceptType == "OptimizationLock") {
        auto* lock = static_cast<const OptimizationLock*>(node);
        if (lock->lockLevel == "warning") {
            return "OptimizationLock: " + lock->lockReason +
                   " (locked by " + lock->lockedBy + ")";
        }
    }
    for (auto* child : node->allChildren()) {
        std::string found = findOptimizationLockWarning(child);
        if (!found.empty()) return found;
    }
    return "";
}

static ImVec4 transformColorForName(const std::string& name) {
    if (name == "constant-fold") return ImVec4(0.45f, 0.85f, 0.45f, 1.0f);
    if (name == "dead-code-elim") return ImVec4(0.90f, 0.45f, 0.45f, 1.0f);
    return ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
}


// ---------------------------------------------------------------------------
//  ImGui InputTextMultiline with std::string resize callback
// ---------------------------------------------------------------------------
struct InputTextCallbackData {
    std::string* str;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data) {
    auto* userData = static_cast<InputTextCallbackData*>(data->UserData);
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        userData->str->resize(data->BufTextLen);
        data->Buf = userData->str->data();
    }
    return 0;
}

static bool InputTextMultilineStr(const char* label, std::string* str,
                                   const ImVec2& size, ImGuiInputTextFlags flags = 0) {
    flags |= ImGuiInputTextFlags_CallbackResize;
    InputTextCallbackData cbData{str};
    // Ensure buffer has room
    if (str->capacity() < str->size() + 256)
        str->reserve(str->size() + 4096);
    return ImGui::InputTextMultiline(label, str->data(), str->capacity() + 1,
                                      size, flags, InputTextCallback, &cbData);
}

static bool InputTextStr(const char* label, std::string* str, ImGuiInputTextFlags flags = 0) {
    flags |= ImGuiInputTextFlags_CallbackResize;
    InputTextCallbackData cbData{str};
    if (str->capacity() < str->size() + 64)
        str->reserve(str->size() + 256);
    return ImGui::InputText(label, str->data(), str->capacity() + 1,
                            flags, InputTextCallback, &cbData);
}

#include "editor_utils/EditorUtilsPart1.h"
#include "editor_utils/EditorUtilsPart2.h"
#include "editor_utils/EditorUtilsPart3.h"
