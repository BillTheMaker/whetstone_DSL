#pragma once
#include "imgui.h"
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>

class TerminalPanel {
public:
    static std::string buildCommandLine(const std::string& cwd, const std::string& command) {
        std::string workingDir = cwd.empty() ? "." : cwd;
#ifdef _WIN32
        return "cmd /C \"cd /d \\\"" + escapeForCmd(workingDir) + "\\\" && " +
               command + " 2>&1\"";
#else
        return "sh -c 'cd \"" + escapeForSh(workingDir) + "\" && " +
               command + " 2>&1'";
#endif
    }

    void render(const std::string& cwd, ImFont* monoFont) {
        ImGui::PushFont(monoFont ? monoFont : ImGui::GetFont());

        ImGui::PushItemWidth(-80.0f);
        bool runNow = inputText("##terminalCmd", commandBuf_,
                                ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Run") || runNow) {
            runCommand(cwd);
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            output_.clear();
        }

        ImGui::Separator();

        ImGui::BeginChild("##terminalOutput", ImVec2(0, 0), false);
        if (output_.empty()) {
            ImGui::TextDisabled("(no output)");
        } else {
            renderAnsiText(output_);
            if (autoScroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20.0f) {
                ImGui::SetScrollHereY(1.0f);
            }
        }
        ImGui::EndChild();

        ImGui::PopFont();
    }

    void append(const std::string& text) {
        output_ += text;
    }

    int runAndAppend(const std::string& cwd, const std::string& command) {
        std::string out;
        int code = runProcess(cwd, command, out);
        output_ += "> " + command + "\n";
        output_ += out;
        return code;
    }

    const std::string& getOutput() const {
        return output_;
    }

private:
    std::string commandBuf_;
    std::string output_;
    bool autoScroll_ = true;

    struct InputTextCallbackData {
        std::string* str;
    };

    static int inputTextCallback(ImGuiInputTextCallbackData* data) {
        auto* userData = static_cast<InputTextCallbackData*>(data->UserData);
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
            userData->str->resize(data->BufTextLen);
            data->Buf = userData->str->data();
        }
        return 0;
    }

    static bool inputText(const char* label, std::string& str, ImGuiInputTextFlags flags) {
        flags |= ImGuiInputTextFlags_CallbackResize;
        InputTextCallbackData cbData{&str};
        if (str.capacity() < str.size() + 64) str.reserve(str.size() + 256);
        return ImGui::InputText(label, str.data(), str.capacity() + 1,
                                flags, inputTextCallback, &cbData);
    }

    static std::string escapeForCmd(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '"') out += "\\\"";
            else out += c;
        }
        return out;
    }

    static std::string escapeForSh(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '\'') out += "'\\''";
            else out += c;
        }
        return out;
    }

    void runCommand(const std::string& cwd) {
        if (commandBuf_.empty()) return;

        std::string out;
        int code = runProcess(cwd, commandBuf_, out);
        output_ += "> " + commandBuf_ + "\n";
        output_ += out;
        output_ += "[exit code: " + std::to_string(code) + "]\n";
    }

    static int runProcess(const std::string& cwd, const std::string& command, std::string& out) {
        std::string fullCmd = buildCommandLine(cwd, command);
#ifdef _WIN32
        FILE* pipe = _popen(fullCmd.c_str(), "r");
#else
        FILE* pipe = popen(fullCmd.c_str(), "r");
#endif
        if (!pipe) {
            out += "[terminal] Failed to launch command.\n";
            return -1;
        }

        char buffer[4096];
        while (!feof(pipe)) {
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                out += buffer;
            }
        }
#ifdef _WIN32
        int code = _pclose(pipe);
#else
        int code = pclose(pipe);
#endif
        return code;
    }

    static ImVec4 ansiColor(int code, const ImVec4& base) {
        switch (code) {
            case 30: return ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
            case 31: return ImVec4(0.85f, 0.2f, 0.2f, 1.0f);
            case 32: return ImVec4(0.2f, 0.75f, 0.3f, 1.0f);
            case 33: return ImVec4(0.85f, 0.65f, 0.2f, 1.0f);
            case 34: return ImVec4(0.35f, 0.55f, 0.95f, 1.0f);
            case 35: return ImVec4(0.8f, 0.35f, 0.8f, 1.0f);
            case 36: return ImVec4(0.2f, 0.75f, 0.75f, 1.0f);
            case 37: return ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
            case 90: return ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
            case 91: return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            case 92: return ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
            case 93: return ImVec4(1.0f, 0.9f, 0.4f, 1.0f);
            case 94: return ImVec4(0.5f, 0.7f, 1.0f, 1.0f);
            case 95: return ImVec4(1.0f, 0.6f, 1.0f, 1.0f);
            case 96: return ImVec4(0.5f, 1.0f, 1.0f, 1.0f);
            case 97: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            case 39: return base;
            default: return base;
        }
    }

    static void renderAnsiText(const std::string& text) {
        ImVec4 baseColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        ImVec4 currentColor = baseColor;
        std::string buffer;
        bool lineStart = true;

        auto emit = [&](const std::string& chunk) {
            if (chunk.empty()) return;
            if (!lineStart) ImGui::SameLine(0.0f, 0.0f);
            ImGui::TextColored(currentColor, "%s", chunk.c_str());
            lineStart = false;
        };

        for (size_t i = 0; i < text.size(); ) {
            if (text[i] == '\x1b' && i + 1 < text.size() && text[i + 1] == '[') {
                emit(buffer);
                buffer.clear();
                i += 2;
                std::string codeStr;
                while (i < text.size() && text[i] != 'm') {
                    codeStr += text[i++];
                }
                if (i < text.size() && text[i] == 'm') i++;
                if (codeStr.empty()) {
                    currentColor = baseColor;
                    continue;
                }
                std::stringstream ss(codeStr);
                std::string part;
                while (std::getline(ss, part, ';')) {
                    int code = 0;
                    try { code = std::stoi(part); } catch (...) { code = 0; }
                    if (code == 0) {
                        currentColor = baseColor;
                    } else {
                        currentColor = ansiColor(code, currentColor);
                    }
                }
                continue;
            }
            if (text[i] == '\n') {
                emit(buffer);
                buffer.clear();
                ImGui::NewLine();
                lineStart = true;
                ++i;
                continue;
            }
            buffer.push_back(text[i++]);
        }
        emit(buffer);
    }
};
