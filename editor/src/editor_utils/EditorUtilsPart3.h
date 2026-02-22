
// ---------------------------------------------------------------------------
//  Syntax highlight color map
// ---------------------------------------------------------------------------
static ImVec4 tokenColor(TokenCategory cat) {
    switch (cat) {
        case TokenCategory::Keyword:     return ImVec4(0.77f, 0.49f, 0.86f, 1.0f); // purple
        case TokenCategory::String:      return ImVec4(0.81f, 0.54f, 0.37f, 1.0f); // orange
        case TokenCategory::Comment:     return ImVec4(0.42f, 0.52f, 0.35f, 1.0f); // green
        case TokenCategory::Number:      return ImVec4(0.71f, 0.80f, 0.55f, 1.0f); // light green
        case TokenCategory::Function:    return ImVec4(0.86f, 0.86f, 0.55f, 1.0f); // yellow
        case TokenCategory::Parameter:   return ImVec4(0.60f, 0.78f, 0.90f, 1.0f); // light blue
        case TokenCategory::Type:        return ImVec4(0.30f, 0.70f, 0.68f, 1.0f); // teal
        case TokenCategory::Operator:    return ImVec4(0.86f, 0.86f, 0.86f, 1.0f); // white
        case TokenCategory::Punctuation: return ImVec4(0.60f, 0.60f, 0.60f, 1.0f); // gray
        case TokenCategory::Builtin:     return ImVec4(0.30f, 0.70f, 0.90f, 1.0f); // blue
        case TokenCategory::Identifier:  return ImVec4(0.60f, 0.78f, 0.90f, 1.0f); // light blue
        default:                         return ImVec4(0.86f, 0.86f, 0.86f, 1.0f); // white
    }
}

// ---------------------------------------------------------------------------
//  Render syntax-highlighted text (read-only preview)
// ---------------------------------------------------------------------------
static void RenderHighlightedText(const std::string& text,
                                   const std::vector<HighlightSpan>& spans) {
    if (text.empty()) return;

    // Build a color for each byte position
    // Default = plain text color
    std::vector<TokenCategory> charCats(text.size(), TokenCategory::Plain);
    for (const auto& span : spans) {
        for (uint32_t i = span.start; i < span.end && i < charCats.size(); ++i) {
            charCats[i] = span.category;
        }
    }

    // Render line by line, span by span
    size_t pos = 0;
    int lineNum = 1;
    while (pos < text.size()) {
        // Find end of line
        size_t eol = text.find('\n', pos);
        if (eol == std::string::npos) eol = text.size();

        // Line number
        ImGui::TextColored(ImVec4(0.45f, 0.45f, 0.45f, 1.0f), "%4d ", lineNum);
        ImGui::SameLine(0.0f, 0.0f);

        // Render spans within this line
        size_t linePos = pos;
        while (linePos < eol) {
            // Find the extent of the current color
            TokenCategory cat = charCats[linePos];
            size_t spanEnd = linePos + 1;
            while (spanEnd < eol && charCats[spanEnd] == cat) ++spanEnd;

            std::string chunk = text.substr(linePos, spanEnd - linePos);
            ImGui::TextColored(tokenColor(cat), "%s", chunk.c_str());
            if (spanEnd < eol) ImGui::SameLine(0.0f, 0.0f);

            linePos = spanEnd;
        }

        if (linePos == pos) {
            // Empty line
            ImGui::TextUnformatted("");
        }

        pos = eol + 1;
        ++lineNum;
    }
}

// ---------------------------------------------------------------------------
//  File tree rendering
// ---------------------------------------------------------------------------
static void RenderFileTree(const FileNode& node, EditorState& state) {
    if (!node.isDir) {
        if (ImGui::Selectable(node.name.c_str())) {
            state.doOpen(node.path, state.defaultBufferMode());
        }
        return;
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (!state.workspaceRoot.empty() && node.path == state.workspaceRoot) {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }
    if (ImGui::TreeNodeEx(node.name.c_str(), flags)) {
        for (const auto& child : node.children) {
            RenderFileTree(child, state);
        }
        ImGui::TreePop();
    }
}

// ---------------------------------------------------------------------------
//  Welcome screen rendering
// ---------------------------------------------------------------------------
static void RenderWelcome(WelcomeScreen& welcome, EditorState& state, std::string& lastDialogPath) {
    ImGui::Text("%s", welcome.getTitle().c_str());
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", welcome.getSubtitle().c_str());
    ImGui::Separator();

    ImGui::Text("Quick Actions");
    if (ImGui::Button("New File")) {
        std::string lang = state.active() ? state.active()->language : "python";
        state.createBuffer(state.makeUntitledName(), "", lang, state.defaultBufferMode());
    }
    ImGui::SameLine();
    if (ImGui::Button("Open File")) {
        auto path = FileDialog::openFile({"Open File", {"*.py","*.cpp","*.h","*.el","*.js","*.ts","*.java","*.rs","*.go","*.org","*.md","*.txt","*.docx","*.*"}, lastDialogPath});
        if (!path.empty()) {
            lastDialogPath = path;
            state.doOpen(path, state.defaultBufferMode());
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Folder")) {
        auto path = FileDialog::openFolder({"Open Folder", state.workspaceRoot});
        if (!path.empty()) {
            std::string error;
            if (state.setWorkspaceRoot(path, &error)) {
                lastDialogPath = path;
            } else {
                state.notify(NotificationLevel::Error,
                             "Failed to open folder: " + error);
            }
        }
    }

    ImGui::Separator();
    ImGui::Text("Recent Files");
    for (const auto& rf : welcome.getRecentFiles()) {
        if (ImGui::Selectable(rf.displayName.c_str())) {
            state.doOpen(rf.path, bufferModeFromString(rf.mode));
        }
    }

    ImGui::Separator();
    ImGui::Text("Tip");
    ImGui::TextWrapped("%s", welcome.getTip(0).c_str());
}
