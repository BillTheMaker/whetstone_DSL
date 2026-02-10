#pragma once
// Step 163: Telemetry + crash reporting (opt-in)

#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Telemetry {
public:
    void initialize(const std::string& root, bool optIn) {
        workspaceRoot_ = root;
        optIn_ = optIn;
        if (!workspaceRoot_.empty()) {
            telemetryPath_ = workspaceRoot_ + "/telemetry.log";
            crashPath_ = workspaceRoot_ + "/crash.log";
        }
        if (optIn_) {
            recordEvent("session_start", json::object());
        }
    }

    void setOptIn(bool optIn) { optIn_ = optIn; }
    bool isOptedIn() const { return optIn_; }

    void recordEvent(const std::string& name, const json& fields) {
        if (!optIn_) return;
        if (telemetryPath_.empty()) return;
        json payload = fields;
        payload["event"] = name;
        payload["timestamp"] = currentTimestamp();
        std::ofstream out(telemetryPath_, std::ios::app);
        if (out.is_open()) {
            out << payload.dump() << "\n";
        }
    }

    void installCrashHandler() {
        if (crashPath_.empty()) return;
        crashInstance_ = this;
        std::set_terminate(&Telemetry::onTerminate);
    }

    const std::string& telemetryPath() const { return telemetryPath_; }
    const std::string& crashPath() const { return crashPath_; }

private:
    static std::string currentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
        return buf;
    }

    std::string workspaceRoot_;
    std::string telemetryPath_;
    std::string crashPath_;
    bool optIn_ = false;

    inline static Telemetry* crashInstance_ = nullptr;

    static void onTerminate() {
        if (crashInstance_ && !crashInstance_->crashPath_.empty()) {
            std::ofstream out(crashInstance_->crashPath_, std::ios::app);
            if (out.is_open()) {
                out << "[crash] terminate called at "
                    << currentTimestamp() << "\n";
            }
        }
        std::abort();
    }
};
