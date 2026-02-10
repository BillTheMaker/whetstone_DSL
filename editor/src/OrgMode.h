#pragma once
#include "imgui.h"
#include "CodeEditorWidget.h"
#include "SyntaxHighlighter.h"
#include "EditorMode.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <functional>

enum class OrgBlockType {
    Heading,
    Paragraph,
    SourceBlock
};

struct OrgBlock {
    OrgBlockType type = OrgBlockType::Paragraph;
    int level = 0;
    std::string title;
    std::string text;
    std::string language;
    std::string code;
    std::string result;
};

struct OrgDocumentState {
    std::string source;
    std::vector<OrgBlock> blocks;
    std::vector<CodeEditorWidget> editors;
    std::vector<EditorMode> modes;
    std::vector<int> sourceBlockMap;
    bool dirty = true;
};

static inline std::string orgTrim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

static inline bool orgStartsWith(const std::string& line, const std::string& prefix) {
    return line.rfind(prefix, 0) == 0;
}

static std::vector<OrgBlock> parseOrgBlocks(const std::string& text) {
    std::vector<OrgBlock> blocks;
    std::vector<std::string> paragraphLines;
    std::vector<std::string> codeLines;
    bool inSrc = false;
    std::string srcLang;

    auto flushParagraph = [&]() {
        if (paragraphLines.empty()) return;
        OrgBlock block;
        block.type = OrgBlockType::Paragraph;
        std::ostringstream oss;
        for (size_t i = 0; i < paragraphLines.size(); ++i) {
            if (i) oss << "\n";
            oss << paragraphLines[i];
        }
        block.text = oss.str();
        blocks.push_back(std::move(block));
        paragraphLines.clear();
    };

    auto flushSource = [&]() {
        OrgBlock block;
        block.type = OrgBlockType::SourceBlock;
        block.language = srcLang.empty() ? "text" : srcLang;
        std::ostringstream oss;
        for (size_t i = 0; i < codeLines.size(); ++i) {
            if (i) oss << "\n";
            oss << codeLines[i];
        }
        block.code = oss.str();
        blocks.push_back(std::move(block));
        codeLines.clear();
        srcLang.clear();
    };

    std::istringstream ss(text);
    std::string line;
    while (std::getline(ss, line)) {
        if (inSrc) {
            if (orgStartsWith(orgTrim(line), "#+end_src")) {
                inSrc = false;
                flushSource();
            } else {
                codeLines.push_back(line);
            }
            continue;
        }

        std::string trimmed = orgTrim(line);
        if (trimmed.empty()) {
            flushParagraph();
            continue;
        }

        if (orgStartsWith(trimmed, "#+begin_src")) {
            flushParagraph();
            std::string rest = trimmed.substr(std::string("#+begin_src").size());
            srcLang = orgTrim(rest);
            inSrc = true;
            continue;
        }

        if (!trimmed.empty() && trimmed[0] == '*') {
            flushParagraph();
            size_t level = 0;
            while (level < trimmed.size() && trimmed[level] == '*') ++level;
            std::string title = orgTrim(trimmed.substr(level));
            OrgBlock block;
            block.type = OrgBlockType::Heading;
            block.level = (int)level;
            block.title = title;
            blocks.push_back(std::move(block));
            continue;
        }

        paragraphLines.push_back(line);
    }

    if (inSrc) {
        flushSource();
    }
    flushParagraph();

    return blocks;
}

static std::string buildOrgText(const std::vector<OrgBlock>& blocks) {
    std::ostringstream out;
    for (size_t i = 0; i < blocks.size(); ++i) {
        const auto& block = blocks[i];
        if (block.type == OrgBlockType::Heading) {
            out << std::string(std::max(1, block.level), '*') << " " << block.title << "\n";
        } else if (block.type == OrgBlockType::Paragraph) {
            out << block.text << "\n";
        } else if (block.type == OrgBlockType::SourceBlock) {
            out << "#+begin_src " << (block.language.empty() ? "text" : block.language) << "\n";
            out << block.code << "\n";
            out << "#+end_src\n";
            if (!block.result.empty()) {
                out << "#+RESULTS:\n" << block.result << "\n";
            }
        }
        if (i + 1 < blocks.size()) out << "\n";
    }
    return out.str();
}

static void updateOrgDocument(OrgDocumentState& state, const std::string& sourceText) {
    if (!state.dirty && sourceText == state.source) return;
    state.source = sourceText;
    state.blocks = parseOrgBlocks(sourceText);
    state.sourceBlockMap.clear();
    state.editors.clear();
    state.modes.clear();
    for (size_t i = 0; i < state.blocks.size(); ++i) {
        if (state.blocks[i].type == OrgBlockType::SourceBlock) {
            state.sourceBlockMap.push_back((int)i);
            state.editors.emplace_back();
            state.modes.emplace_back(state.blocks[i].language);
        }
    }
    state.dirty = false;
}

static bool renderOrgDocument(OrgDocumentState& state,
                              const std::string& sourceText,
                              bool showWhitespace,
                              ImFont* monoFont,
                              ImFont* uiFont,
                              const std::function<void(const std::string&)>& onOrgTextChanged,
                              const std::function<void(int, const std::string&, const std::string&)>& onBlockChanged,
                              const std::function<std::string(const std::string&, const std::string&)>& onRunBlock) {
    updateOrgDocument(state, sourceText);
    bool changed = false;

    int srcIndex = 0;
    for (size_t i = 0; i < state.blocks.size(); ++i) {
        auto& block = state.blocks[i];
        ImGui::PushID((int)i);
        if (block.type == OrgBlockType::Heading) {
            ImGui::PushFont(uiFont ? uiFont : ImGui::GetFont());
            ImGui::Text("%s%s", std::string(block.level, '#').c_str(), block.title.c_str());
            ImGui::PopFont();
        } else if (block.type == OrgBlockType::Paragraph) {
            ImGui::TextWrapped("%s", block.text.c_str());
        } else if (block.type == OrgBlockType::SourceBlock) {
            ImGui::PushFont(uiFont ? uiFont : ImGui::GetFont());
            ImGui::Text("Source (%s)", block.language.c_str());
            ImGui::PopFont();

            CodeEditorOptions opts;
            opts.readOnly = false;
            opts.showWhitespace = showWhitespace;
            opts.showLineNumbers = true;
            EditorMode& mode = state.modes[srcIndex];
            mode.setLanguage(block.language);
            opts.mode = &mode;

            auto spans = SyntaxHighlighter::highlight(block.code, block.language);
            CodeEditorResult result = state.editors[srcIndex].render(
                "##orgBlockEditor", block.code, spans, opts,
                ImVec2(0, 180), monoFont ? monoFont : ImGui::GetFont());

            if (result.changed) {
                changed = true;
                onBlockChanged((int)i, block.language, block.code);
            }

            if (ImGui::Button("Run Block")) {
                block.result = onRunBlock(block.language, block.code);
                changed = true;
            }
            if (!block.result.empty()) {
                ImGui::SameLine();
                ImGui::TextDisabled("Result:");
                ImGui::BeginChild("##orgResult", ImVec2(0, 80), true);
                ImGui::TextWrapped("%s", block.result.c_str());
                ImGui::EndChild();
            }
            srcIndex++;
        }
        ImGui::Spacing();
        ImGui::PopID();
    }

    if (changed) {
        std::string rebuilt = buildOrgText(state.blocks);
        onOrgTextChanged(rebuilt);
    }
    return changed;
}
