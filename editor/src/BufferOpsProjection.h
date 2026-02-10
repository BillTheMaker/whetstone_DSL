#pragma once
// --- BufferOpsProjection.h ---
// Extracted from BufferOps.h (Sprint 8 cleanup).
// Projection and diff helpers.
// Included from BufferOps.h after the EditorState struct definition.

inline void EditorState::projectToLanguage(const std::string& targetLanguage) {
    if (!active()) return;
    Module* ast = activeAST();
    if (!ast) {
        notify(NotificationLevel::Error,
               "Project to " + targetLanguage + ": no AST available.");
        return;
    }

    CrossLanguageProjector projector;
    auto projected = projector.project(ast, targetLanguage);
    if (!projected) {
        notify(NotificationLevel::Error,
               "Project to " + targetLanguage + ": failed to project AST.");
        return;
    }

    const int srcAnnoCount = countAnnotationNodes(ast);
    const int projAnnoCount = countAnnotationNodes(projected.get());
    const bool preserved = projector.annotationsPreserved(ast, projected.get());
    std::string generated = generateForLanguage(projected.get(), targetLanguage);

    std::string baseName = "(untitled-projection:" + targetLanguage + ")";
    std::string projName = baseName;
    int suffix = 1;
    while (buffers.hasBuffer(projName)) {
        projName = baseName + "-" + std::to_string(suffix++);
    }

    createBuffer(projName, generated, targetLanguage, defaultBufferMode());
    if (active()) {
        active()->readOnly = true;
        active()->sync.setText(generated, targetLanguage);
        active()->sync.setAST(std::move(projected));
        active()->incrementalOptimizer.setRoot(active()->sync.getAST());
        active()->editBuf = generated;
        active()->editor.setContent(active()->editBuf, targetLanguage);
        active()->mode.setLanguage(targetLanguage);
        active()->highlightsDirty = true;
        active()->generatedLanguage = targetLanguage;
        active()->generatedMode.setLanguage(targetLanguage);
        active()->generatedHighlightsDirty = true;
        active()->modified = false;
    }

    notify(NotificationLevel::Success,
           "Projected to " + targetLanguage + " in " + projName +
           " (annotations " + std::to_string(srcAnnoCount) + " -> " +
           std::to_string(projAnnoCount) + ", types preserved: " +
           (preserved ? "yes" : "no") + ").");
}

inline void EditorState::refreshActiveTextFromAST() {
    if (!active()) return;
    if (!isStructured()) return;
    active()->editBuf = active()->sync.getText();
    active()->editor.setContent(active()->editBuf, active()->language);
    active()->highlightsDirty = true;
    active()->highlightRequestTime = ImGui::GetTime();
    active()->modified = true;
    queueLspDidChange();
    analysisPending = true;
    analysisLastChange = ImGui::GetTime();
}

inline void EditorState::openDiff(const std::string& beforeText,
    const std::string& afterText,
    bool preview,
    int action,
    const std::vector<std::string>& transformIds,
    const std::vector<BatchMutationAPI::Mutation>& batchMutations = {}) {
    diff.active = true;
    diff.preview = preview;
    diff.action = action;
    diff.batch = !batchMutations.empty();
    diff.beforeText = beforeText;
    diff.afterText = afterText;
    diff.transformIds = transformIds;
    diff.batchMutations = batchMutations;
    LineDiff lineDiff = buildLineDiff(beforeText, afterText);
    diff.beforeLines = std::move(lineDiff.beforeLines);
    diff.afterLines = std::move(lineDiff.afterLines);
}
