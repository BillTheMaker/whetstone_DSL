#pragma once
// Step 56: Robust Emacs command integration
//
// ElispCommandBuilder: builds valid Elisp expressions with proper escaping.
// EmacsConnection: manages Emacs daemon lifecycle (start, health check,
//   command execution, auto-restart).
//
// The connection talks to emacsclient --eval for command execution.
// On Windows, Emacs must be on PATH or configured via setEmacsPath().

#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <filesystem>

// ---------------------------------------------------------------------------
//  ElispCommandBuilder — produces valid Elisp command strings
// ---------------------------------------------------------------------------
class ElispCommandBuilder {
public:
    // Build an Elisp expression to open a file
    static std::string findFile(const std::string& path) {
        return "(find-file \"" + escapeString(path) + "\")";
    }

    // Build an Elisp expression to save current buffer
    static std::string saveBuffer() {
        return "(save-buffer)";
    }

    // Build an Elisp expression to evaluate arbitrary Elisp
    static std::string eval(const std::string& elisp) {
        return "(eval (read \"" + escapeString(elisp) + "\"))";
    }

    // Escape special characters in strings for Elisp embedding
    static std::string escapeString(const std::string& str) {
        std::string result;
        result.reserve(str.size() + 16);
        for (char c : str) {
            switch (c) {
                case '\\': result += "\\\\"; break;
                case '"':  result += "\\\""; break;
                case '\n': result += "\\n";  break;
                case '\t': result += "\\t";  break;
                case '\r': result += "\\r";  break;
                default:   result += c;      break;
            }
        }
        return result;
    }

    // Build an Elisp expression to set a variable
    static std::string setVariable(const std::string& name, const std::string& value) {
        return "(setq " + name + " \"" + escapeString(value) + "\")";
    }

    // Build a progn block from multiple expressions
    static std::string progn(const std::vector<std::string>& exprs) {
        std::string result = "(progn";
        for (const auto& e : exprs) {
            result += " " + e;
        }
        result += ")";
        return result;
    }

    // Build an Elisp expression to list functions matching a prefix/regex
    static std::string aproposFunctions(const std::string& prefixRegex) {
        std::string pattern = prefixRegex;
        return "(mapconcat #'symbol-name (apropos-internal \"" +
               escapeString(pattern) + "\" 'fboundp) \"\\n\")";
    }

    // Build an Elisp expression to describe a function (signature + doc)
    static std::string describeFunction(const std::string& name) {
        std::string sym = escapeString(name);
        return "(let* ((sym '" + sym + ") "
               "(args (if (fboundp 'help-function-arglist) "
               "          (help-function-arglist sym t) nil)) "
               "(doc (or (documentation sym t) \"\"))) "
               "(concat (if args (prin1-to-string args) \"\") \"\\n\" doc))";
    }

    // Build an Elisp expression to resolve a key binding to a command name
    static std::string keyBinding(const std::string& keySequence) {
        return "(let ((cmd (key-binding (kbd \"" + escapeString(keySequence) + "\")))) "
               "(if cmd (symbol-name cmd) \"\"))";
    }

    // Build an Elisp expression to call a command interactively
    static std::string callInteractive(const std::string& command) {
        return "(call-interactively '" + escapeString(command) + ")";
    }

    // Build an Elisp expression to run an extended command by name
    static std::string executeExtendedCommand(const std::string& command) {
        std::string sym = escapeString(command);
        return "(if (commandp '" + sym + ") "
               "(call-interactively '" + sym + ") "
               "(concat \"Unknown command: \" \"" + sym + "\"))";
    }

    // Build an Elisp expression for the current mode line
    static std::string modeLine() {
        return "(format-mode-line mode-line-format)";
    }

    // Build an Elisp expression to read a buffer for a given file
    static std::string bufferStringForFile(const std::string& path) {
        return "(with-current-buffer (find-file-noselect \"" +
               escapeString(path) + "\") (buffer-string))";
    }

    // Build an Elisp expression to replace a buffer's contents and save
    static std::string replaceBufferForFile(const std::string& path, const std::string& text) {
        return "(with-current-buffer (find-file-noselect \"" + escapeString(path) + "\") "
               "(erase-buffer) "
               "(insert \"" + escapeString(text) + "\") "
               "(save-buffer))";
    }
};

// ---------------------------------------------------------------------------
//  EmacsConnection — manages Emacs daemon lifecycle
// ---------------------------------------------------------------------------
class EmacsConnection {
public:
    EmacsConnection() = default;

    // Set custom path to emacs/emacsclient executables
    void setEmacsPath(const std::string& path) { emacsPath_ = path; }
    void setEmacsclientPath(const std::string& path) { emacsclientPath_ = path; }

    // Start Emacs daemon
    bool startDaemon() {
        std::string output;
        return startDaemonWithConfig("", output);
    }

    // Start Emacs daemon with an explicit init file path
    virtual bool startDaemonWithConfig(const std::string& initFilePath, std::string& outLog) {
        outLog.clear();
        std::string cmd = emacsPath_ + " --daemon";
        if (!initFilePath.empty()) {
            cmd += " --load \"" + initFilePath + "\"";
        }
        cmd += " 2>&1";
        FILE* pipe = openPipe(cmd.c_str(), "r");
        if (!pipe) {
            lastError_ = "Failed to start Emacs daemon";
            return false;
        }
        char buf[512];
        while (fgets(buf, sizeof(buf), pipe)) {
            outLog += buf;
        }
        int result = closePipe(pipe);
        if (result == 0) {
            daemonRunning_ = true;
            return true;
        }
        if (isDaemonAlive()) {
            daemonRunning_ = true;
            return true;
        }
        lastError_ = outLog.empty() ? "Failed to start Emacs daemon" : outLog;
        return false;
    }

    // Check if daemon is running
    bool isDaemonAlive() const {
        // Try to evaluate a simple expression
        std::string cmd = emacsclientPath_ + " --eval \"(+ 1 1)\" 2>&1";
        FILE* pipe = openPipe(cmd.c_str(), "r");
        if (!pipe) return false;

        char buf[256];
        std::string output;
        while (fgets(buf, sizeof(buf), pipe)) {
            output += buf;
        }
        int ret = closePipe(pipe);

        // If emacsclient returns 0 and outputs "2", daemon is alive
        return (ret == 0 && output.find("2") != std::string::npos);
    }

    // Send command to Emacs and get result
    virtual std::string sendCommand(const std::string& elispCommand) {
        lastError_.clear();

        if (!daemonRunning_ && !isDaemonAlive()) {
            lastError_ = "Emacs daemon is not running";
            return "";
        }

        std::string cmd = emacsclientPath_ + " --eval \"" +
                          ElispCommandBuilder::escapeString(elispCommand) + "\" 2>&1";
        FILE* pipe = openPipe(cmd.c_str(), "r");
        if (!pipe) {
            lastError_ = "Failed to execute emacsclient";
            return "";
        }

        char buf[4096];
        std::string output;
        while (fgets(buf, sizeof(buf), pipe)) {
            output += buf;
        }
        int ret = closePipe(pipe);

        // Trim trailing newline
        while (!output.empty() && (output.back() == '\n' || output.back() == '\r'))
            output.pop_back();

        if (ret != 0) {
            lastError_ = output.empty() ? "emacsclient returned error" : output;
            // Check if daemon died
            if (!isDaemonAlive()) {
                daemonRunning_ = false;
                lastError_ = "Emacs daemon crashed";
            }
        }

        return output;
    }

    std::string getBufferText(const std::string& path) {
        return sendCommand(ElispCommandBuilder::bufferStringForFile(path));
    }

    bool setBufferText(const std::string& path, const std::string& text) {
        std::string result = sendCommand(ElispCommandBuilder::replaceBufferForFile(path, text));
        if (!lastError_.empty() && result == "error") return false;
        return true;
    }

    bool openFileInEmacsFrame(const std::string& path) {
        lastError_.clear();
        std::string cmd = emacsclientPath_ + " -c -n \"" + path + "\"";
        int code = runCommand(cmd);
        if (code != 0) {
            lastError_ = "emacsclient failed to open frame";
            return false;
        }
        return true;
    }

    // Auto-restart daemon if it died
    bool ensureDaemon() {
        if (daemonRunning_ && isDaemonAlive()) return true;
        daemonRunning_ = false;
        return startDaemon();
    }

    // Get the last error
    std::string getLastError() const { return lastError_; }

    // Check if we believe the daemon is running
    bool isConnected() const { return daemonRunning_; }

protected:
    // Wrappers for popen/pclose to allow testing without real Emacs
    virtual FILE* openPipe(const char* cmd, const char* mode) const {
#ifdef _WIN32
        return _popen(cmd, mode);
#else
        return popen(cmd, mode);
#endif
    }

    virtual int closePipe(FILE* pipe) const {
#ifdef _WIN32
        return _pclose(pipe);
#else
        return pclose(pipe);
#endif
    }

    virtual int runCommand(const std::string& cmd) const {
        return std::system(cmd.c_str());
    }

    bool daemonRunning_ = false;

private:
    std::string emacsPath_       = "emacs";
    std::string emacsclientPath_ = "emacsclient";
    std::string lastError_;
};

// Resolve emacs config path to init.el
static inline std::string resolveEmacsInitPath(const std::string& configPath) {
    if (configPath.empty()) return "";
    std::filesystem::path p(configPath);
    if (std::filesystem::exists(p)) {
        if (std::filesystem::is_directory(p)) {
            return (p / "init.el").string();
        }
        return p.string();
    }
    if (p.extension().string().empty()) {
        return (p / "init.el").string();
    }
    return p.string();
}

// ---------------------------------------------------------------------------
//  MockEmacsConnection — for testing without a real Emacs installation
// ---------------------------------------------------------------------------
class MockEmacsConnection : public EmacsConnection {
public:
    MockEmacsConnection() {
        // Simulate daemon as running
        daemonRunning_ = true;
    }

    bool startDaemon() {
        daemonRunning_ = true;
        return true;
    }

    bool startDaemonWithConfig(const std::string& initFilePath, std::string& outLog) {
        lastInitPath_ = initFilePath;
        outLog = "Loaded " + initFilePath;
        daemonRunning_ = true;
        return true;
    }

    bool isDaemonAlive() const { return daemonRunning_; }

    std::string sendCommand(const std::string& elispCommand) override {
        lastSentCommand_ = elispCommand;

        // Simulate responses for known commands
        if (elispCommand.find("buffer-string") != std::string::npos) {
            return "mock-buffer";
        }
        if (elispCommand.find("erase-buffer") != std::string::npos) {
            return "t";
        }
        if (elispCommand.find("find-file") != std::string::npos) {
            return "t";  // success
        }
        if (elispCommand.find("save-buffer") != std::string::npos) {
            return "t";
        }
        if (elispCommand == "(+ 1 1)") {
            return "2";
        }
        if (elispCommand.find("package-alist") != std::string::npos) {
            return "use-package|2.4.1|Declarative package configuration\n"
                   "cl-lib|1.0|Common Lisp extensions for Emacs";
        }
        if (elispCommand.find("package-archive-contents") != std::string::npos) {
            return "magit|3.4.0|Git porcelain inside Emacs\n"
                   "projectile|2.7.0|Project interaction library";
        }
        if (elispCommand.find("(require '") != std::string::npos) {
            return "t";
        }
        if (elispCommand.find("apropos-internal") != std::string::npos) {
            if (elispCommand.find("use-package") != std::string::npos) {
                return "use-package\nuse-package-alias";
            }
            if (elispCommand.find("projectile") != std::string::npos) {
                return "projectile-find-file\nprojectile-switch-project";
            }
            if (elispCommand.find("cl-") != std::string::npos) {
                return "cl-loop\ncl-mapcar";
            }
            return "";
        }
        if (elispCommand.find("help-function-arglist") != std::string::npos) {
            if (elispCommand.find("use-package") != std::string::npos) {
                return "(name &rest args)\nConfigure package via use-package.";
            }
            if (elispCommand.find("projectile") != std::string::npos) {
                return "(project)\nProjectile project switching.";
            }
            return "()\nEmacs function.";
        }
        if (elispCommand.find("key-binding") != std::string::npos) {
            if (elispCommand.find("C-x C-s") != std::string::npos) {
                return "save-buffer";
            }
            if (elispCommand.find("C-x C-f") != std::string::npos) {
                return "find-file";
            }
            return "";
        }
        if (elispCommand.find("call-interactively") != std::string::npos) {
            return "t";
        }
        if (elispCommand.find("format-mode-line") != std::string::npos) {
            return "Emacs: Fundamental (Whetstone)";
        }
        // Unknown command — simulate error
        lastError_ = "void-function";
        return "error";
    }

    bool ensureDaemon() {
        daemonRunning_ = true;
        return true;
    }

    std::string getLastError() const { return lastError_; }

    // Test helper: get the last command sent
    std::string getLastSentCommand() const { return lastSentCommand_; }
    std::string getLastInitPath() const { return lastInitPath_; }
    std::string getLastRunCommand() const { return lastRunCommand_; }

protected:
    int runCommand(const std::string& cmd) const override {
        lastRunCommand_ = cmd;
        return 0;
    }

private:
    std::string lastError_;
    std::string lastSentCommand_;
    std::string lastInitPath_;
    mutable std::string lastRunCommand_;
};

