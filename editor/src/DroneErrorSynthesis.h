#pragma once
// Step 636: Error type synthesis for drone

#include <string>
#include <vector>

enum class DroneErrorKind {
    MqttError,
    NexusError,
    ExecutionError,
    CapabilityMismatch
};

struct DroneErrorSpec {
    DroneErrorKind kind = DroneErrorKind::ExecutionError;
    std::string message;
    int code = 0;
    bool retryable = false;
};

class DroneErrorSynthesis {
public:
    static std::string className(DroneErrorKind kind) {
        if (kind == DroneErrorKind::MqttError) return "MqttError";
        if (kind == DroneErrorKind::NexusError) return "NexusError";
        if (kind == DroneErrorKind::ExecutionError) return "ExecutionError";
        return "CapabilityMismatch";
    }

    static std::string toErrorCodeCategory(DroneErrorKind kind) {
        if (kind == DroneErrorKind::MqttError) return "mqtt";
        if (kind == DroneErrorKind::NexusError) return "nexus";
        if (kind == DroneErrorKind::ExecutionError) return "execution";
        return "capability";
    }

    static std::string expectedAlias(DroneErrorKind kind,
                                     const std::string& valueType = "void") {
        return "std::expected<" + valueType + ", " + className(kind) + ">";
    }

    static bool isRetryable(const DroneErrorSpec& spec) {
        if (spec.kind == DroneErrorKind::CapabilityMismatch) return false;
        if (spec.code < 0) return false;
        return spec.retryable;
    }

    static std::string synthesizeHeader(const std::string& ns = "whetstone") {
        if (ns.empty()) return "";
        std::string code;
        code += "#pragma once\n";
        code += "#include <expected>\n";
        code += "#include <string>\n\n";
        code += "namespace " + ns + " {\n";
        code += "struct MqttError { int code = 0; std::string message; };\n";
        code += "struct NexusError { int code = 0; std::string message; };\n";
        code += "struct ExecutionError { int code = 0; std::string message; };\n";
        code += "struct CapabilityMismatch { int code = 0; std::string message; };\n";
        code += "} // namespace " + ns + "\n";
        return code;
    }
};
