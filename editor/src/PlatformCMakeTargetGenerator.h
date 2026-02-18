#pragma once
// Step 637: Cross-platform CMake target generator

#include <string>

struct PlatformBlockInput {
    std::string targetName;
    std::string processorPattern;
    std::string sourceFile;
    std::string annotationTarget;
};

class PlatformCMakeTargetGenerator {
public:
    static bool isSupportedAnnotation(const std::string& annotationTarget) {
        return !annotationTarget.empty();
    }

    static std::string generateCMakeGuard(const PlatformBlockInput& in) {
        if (in.targetName.empty() || in.processorPattern.empty() || in.sourceFile.empty()) {
            return "";
        }
        std::string cmake;
        cmake += "if(CMAKE_SYSTEM_PROCESSOR MATCHES \"" + in.processorPattern + "\")\n";
        cmake += "  target_sources(" + in.targetName + " PRIVATE " + in.sourceFile + ")\n";
        cmake += "  target_compile_definitions(" + in.targetName + " PRIVATE WHETSTONE_PLATFORM_MATCH=1)\n";
        cmake += "endif()\n";
        return cmake;
    }

    static std::string generateIfdef(const std::string& macroName,
                                     const std::string& code) {
        if (macroName.empty()) return "";
        return "#ifdef " + macroName + "\n" + code + "\n#endif\n";
    }

    static std::string macroForAnnotationTarget(const std::string& annotationTarget) {
        if (annotationTarget == "aarch64-apple-darwin") return "WHETSTONE_AARCH64_APPLE_DARWIN";
        if (annotationTarget == "x86_64-linux") return "WHETSTONE_X86_64_LINUX";
        if (annotationTarget == "x86_64-windows") return "WHETSTONE_X86_64_WINDOWS";
        return "WHETSTONE_PLATFORM_GENERIC";
    }
};
