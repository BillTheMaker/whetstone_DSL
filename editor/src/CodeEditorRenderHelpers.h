#pragma once
// Included inside CodeEditorWidget (render helpers).
    static ImU32 colorFor(TokenCategory cat) {
        ImU32 fallback = IM_COL32(220, 220, 220, 255);
        switch (cat) {
            case TokenCategory::Keyword:     fallback = IM_COL32(196, 126, 220, 255); break;
            case TokenCategory::String:      fallback = IM_COL32(207, 138, 94, 255); break;
            case TokenCategory::Comment:     fallback = IM_COL32(107, 133, 89, 255); break;
            case TokenCategory::Number:      fallback = IM_COL32(181, 204, 140, 255); break;
            case TokenCategory::Function:    fallback = IM_COL32(220, 220, 140, 255); break;
            case TokenCategory::Parameter:   fallback = IM_COL32(153, 199, 229, 255); break;
            case TokenCategory::Type:        fallback = IM_COL32(77, 179, 174, 255); break;
            case TokenCategory::Operator:    fallback = IM_COL32(220, 220, 220, 255); break;
            case TokenCategory::Punctuation: fallback = IM_COL32(160, 160, 160, 255); break;
            case TokenCategory::Builtin:     fallback = IM_COL32(77, 179, 229, 255); break;
            case TokenCategory::Identifier:  fallback = IM_COL32(160, 200, 230, 255); break;
            default: break;
        }
        return ThemeEngine::instance().syntaxColor(cat, fallback);
    }

    static int lineFromPos(int pos, const std::vector<int>& lineStarts) {
        if (lineStarts.empty()) return 0;
        int line = 0;
        for (int i = 0; i < (int)lineStarts.size(); ++i) {
            if (lineStarts[i] <= pos) line = i;
            else break;
        }
        return line;
    }

    static bool isBracketChar(char c) {
        return c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}';
    }

    static int lineFromMouseY(float mouseY, float baseY, float lineHeight, int lineCount) {
        int line = (int)((mouseY - baseY) / lineHeight);
        line = std::max(0, std::min(line, lineCount - 1));
        return line;
    }

    static int positionFromMouse(const ImVec2& mouse,
                                 const ImVec2& base,
                                 const std::vector<int>& lineStarts,
                                 const std::string& text,
                                 float charAdvance,
                                 float lineHeight) {
        int line = (int)((mouse.y - base.y) / lineHeight);
        line = std::max(0, std::min(line, (int)lineStarts.size() - 1));

        int start = lineStarts[line];
        int end = (line + 1 < (int)lineStarts.size()) ? lineStarts[line + 1] - 1 : (int)text.size();
        int len = std::max(0, end - start);

        int col = (int)((mouse.x - base.x) / charAdvance);
        col = std::max(0, std::min(col, len));
        return start + col;
    }

    static float calcGutterWidth(int lineCount, ImFont* font, float charAdvance) {
        int digits = 1;
        int n = std::max(1, lineCount);
        while (n >= 10) { n /= 10; ++digits; }
        const float pad = 6.0f;
        float digitsWidth = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1.0f,
                                                std::string(digits, '0').c_str()).x;
        float width = digitsWidth + pad * 2.0f;
        return std::max(width, charAdvance * 2.0f + pad * 2.0f);
    }

    const FoldRegion* findFoldAtLine(int line) const {
        for (const auto& f : folds_) {
            if (f.startLine == line) return &f;
        }
        return nullptr;
    }

    static void drawTriangle(ImDrawList* drawList, const ImVec2& center, ImU32 color, bool down) {
        float s = 4.0f;
        if (down) {
            drawList->AddTriangleFilled(
                ImVec2(center.x - s, center.y - s * 0.6f),
                ImVec2(center.x + s, center.y - s * 0.6f),
                ImVec2(center.x, center.y + s),
                color);
        } else {
            drawList->AddTriangleFilled(
                ImVec2(center.x - s, center.y - s),
                ImVec2(center.x - s, center.y + s),
                ImVec2(center.x + s, center.y),
                color);
        }
    }

    static bool lineIn(const std::vector<int>* lines, int line) {
        if (!lines) return false;
        return std::find(lines->begin(), lines->end(), line) != lines->end();
    }

    static bool annotationAtLine(const std::vector<AnnotationMarker>& markers,
                                 int line,
                                 AnnotationMarker& out) {
        for (const auto& m : markers) {
            if (m.line == line) {
                out = m;
                return true;
            }
        }
        return false;
    }

    static bool suggestionAtLine(const std::vector<SuggestionMarker>& markers,
                                 int line,
                                 SuggestionMarker& out) {
        for (const auto& m : markers) {
            if (m.line == line) {
                out = m;
                return true;
            }
        }
        return false;
    }

    static bool conflictAtLine(const std::vector<AnnotationConflictMarker>& markers,
                               int line,
                               AnnotationConflictMarker& out) {
        for (const auto& m : markers) {
            if (m.childLine == line || m.parentLine == line) {
                out = m;
                return true;
            }
        }
        return false;
    }

    static std::string diagnosticMessageAtLine(const std::vector<DiagnosticRange>& diags, int line) {
        for (const auto& d : diags) {
            if (line >= d.startLine && line <= d.endLine) return d.message;
        }
        return "";
    }

    static bool securityDiagnosticAtLine(const std::vector<DiagnosticRange>& diags,
                                         int line,
                                         DiagnosticRange& out) {
        for (const auto& d : diags) {
            if (line < d.startLine || line > d.endLine) continue;
            if (d.message.find("[Security]") != std::string::npos) {
                out = d;
                return true;
            }
        }
        return false;
    }

    static void renderSquiggles(const std::vector<DiagnosticRange>& diags,
                                int line,
                                float y,
                                float lineHeight,
                                float charAdvance,
                                float textBaseX,
                                int lineLen,
                                ImDrawList* drawList) {
        float baseY = y + lineHeight - 2.0f;
        for (const auto& d : diags) {
            if (line < d.startLine || line > d.endLine) continue;
            int startCol = (line == d.startLine) ? d.startCol : 0;
            int endCol = (line == d.endLine) ? d.endCol : lineLen;
            startCol = std::max(0, startCol);
            endCol = std::max(startCol, endCol);
            float xStart = textBaseX + startCol * charAdvance;
            float xEnd = textBaseX + endCol * charAdvance;
            ImU32 color = (d.severity == 1)
                ? ThemeEngine::instance().editorColor("diag_error", IM_COL32(220, 80, 80, 255))
                : (d.severity == 2)
                    ? ThemeEngine::instance().editorColor("diag_warning", IM_COL32(220, 160, 60, 255))
                    : ThemeEngine::instance().editorColor("diag_info", IM_COL32(90, 160, 220, 255));
            float x = xStart;
            float amp = 2.0f;
            bool up = true;
            int segment = 0;
            bool dotted = (d.severity != 1 && d.severity != 2);
            while (x < xEnd) {
                if (dotted) {
                    drawList->AddCircleFilled(ImVec2(x, baseY), 1.3f, color);
                    x += 4.0f;
                    continue;
                }
                float x2 = std::min(x + 4.0f, xEnd);
                float y1 = baseY + (up ? -amp : amp);
                float y2 = baseY + (up ? amp : -amp);
                bool drawSegment = true;
                if (d.severity == 2) {
                    drawSegment = (segment % 2 == 0);
                }
                if (drawSegment) {
                    drawList->AddLine(ImVec2(x, y1), ImVec2(x2, y2), color, 1.0f);
                }
                up = !up;
                x = x2;
                segment++;
            }
        }
    }

    static std::string diagnosticMessageAtPoint(const std::vector<DiagnosticRange>& diags,
                                                int line,
                                                const ImVec2& mouse,
                                                float y,
                                                float lineHeight,
                                                float charAdvance,
                                                float textBaseX) {
        if (mouse.y < y || mouse.y > y + lineHeight) return "";
        for (const auto& d : diags) {
            if (line < d.startLine || line > d.endLine) continue;
            int startCol = (line == d.startLine) ? d.startCol : 0;
            int endCol = (line == d.endLine) ? d.endCol : startCol + 1;
            float xStart = textBaseX + startCol * charAdvance;
            float xEnd = textBaseX + endCol * charAdvance;
            if (mouse.x >= xStart && mouse.x <= xEnd) return d.message;
        }
        return "";
    }


    void updateFolds(const std::string& text, const std::string& language) {
        if (text == lastFoldText_ && language == lastFoldLang_) return;
        lastFoldText_ = text;
        lastFoldLang_ = language;

        std::vector<FoldRegion> newFolds;
        if (text.empty()) { folds_.clear(); return; }

        const TSLanguage* lang = nullptr;
        if (language == "python") lang = tree_sitter_python();
        else if (language == "cpp") lang = tree_sitter_cpp();
        else if (language == "elisp") lang = tree_sitter_elisp();
        if (!lang) { folds_.clear(); return; }

        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, lang);
        TSTree* tree = ts_parser_parse_string(parser, nullptr, text.c_str(), (uint32_t)text.size());
        TSNode root = ts_tree_root_node(tree);

        collectFoldNodes(root, language, newFolds);

        // Preserve folded state by startLine
        for (auto& f : newFolds) {
            for (const auto& old : folds_) {
                if (old.startLine == f.startLine && old.endLine == f.endLine) {
                    f.folded = old.folded;
                    break;
                }
            }
        }

        folds_ = std::move(newFolds);
        if (applyDesiredFoldState_) {
            for (auto& f : folds_) {
                f.folded = std::find(desiredFoldedLines_.begin(),
                                     desiredFoldedLines_.end(),
                                     f.startLine) != desiredFoldedLines_.end();
            }
            applyDesiredFoldState_ = false;
        }

        ts_tree_delete(tree);
        ts_parser_delete(parser);
    }

    static bool isFoldNodeType(const std::string& language, const char* type) {
        if (language == "python") {
            return strcmp(type, "function_definition") == 0 ||
                   strcmp(type, "class_definition") == 0 ||
                   strcmp(type, "if_statement") == 0 ||
                   strcmp(type, "for_statement") == 0 ||
                   strcmp(type, "while_statement") == 0 ||
                   strcmp(type, "with_statement") == 0 ||
                   strcmp(type, "try_statement") == 0;
        } else if (language == "cpp") {
            return strcmp(type, "function_definition") == 0 ||
                   strcmp(type, "class_specifier") == 0 ||
                   strcmp(type, "struct_specifier") == 0 ||
                   strcmp(type, "namespace_definition") == 0 ||
                   strcmp(type, "if_statement") == 0 ||
                   strcmp(type, "for_statement") == 0 ||
                   strcmp(type, "while_statement") == 0 ||
                   strcmp(type, "compound_statement") == 0;
        } else if (language == "elisp") {
            return strcmp(type, "function_definition") == 0 ||
                   strcmp(type, "lambda_expression") == 0 ||
                   strcmp(type, "if_expression") == 0 ||
                   strcmp(type, "while_expression") == 0 ||
                   strcmp(type, "cond_expression") == 0;
        }
        return false;
    }

    static void collectFoldNodes(TSNode node, const std::string& language, std::vector<FoldRegion>& out) {
        if (ts_node_is_null(node)) return;
        const char* type = ts_node_type(node);
        if (isFoldNodeType(language, type)) {
            TSPoint start = ts_node_start_point(node);
            TSPoint end = ts_node_end_point(node);
            if (end.row > start.row) {
                FoldRegion f;
                f.startLine = (int)start.row;
                f.endLine = (int)end.row;
                f.folded = false;
                out.push_back(f);
            }
        }
        uint32_t count = ts_node_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            collectFoldNodes(ts_node_child(node, i), language, out);
        }
    }
