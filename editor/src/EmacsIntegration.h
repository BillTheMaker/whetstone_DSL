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
        std::string cmd = emacsPath_ + " --daemon 2>&1";
        int result = runCommand(cmd);
        if (result == 0) {
            daemonRunning_ = true;
            return true;
        }
        // Even if command "fails", check if daemon is actually alive
        // (Emacs daemon may return non-zero but still be running)
        if (isDaemonAlive()) {
            daemonRunning_ = true;
            return true;
        }
        lastError_ = "Failed to start Emacs daemon";
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
    std::string sendCommand(const std::string& elispCommand) {
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

    bool isDaemonAlive() const { return daemonRunning_; }

    std::string sendCommand(const std::string& elispCommand) {
        lastSentCommand_ = elispCommand;

        // Simulate responses for known commands
        if (elispCommand.find("find-file") != std::string::npos) {
            return "t";  // success
        }
        if (elispCommand.find("save-buffer") != std::string::npos) {
            return "t";
        }
        if (elispCommand == "(+ 1 1)") {
            return "2";
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

private:
    std::string lastError_;
    std::string lastSentCommand_;
};
