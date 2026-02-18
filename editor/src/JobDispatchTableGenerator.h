#pragma once
// Step 642: Job dispatch table generator

#include <string>
#include <vector>

struct JobDispatchEntrySpec {
    std::string jobType;
    std::vector<std::string> requiredCaps;
    std::string payloadType;
    std::string executor;
};

struct JobDispatchTableOutput {
    bool success = false;
    std::string headerCode;
    std::vector<std::string> errors;
};

class JobDispatchTableGenerator {
public:
    static JobDispatchTableOutput generate(const std::vector<JobDispatchEntrySpec>& entries) {
        JobDispatchTableOutput out;
        if (entries.empty()) {
            out.errors.push_back("entries_required");
            return out;
        }

        out.success = true;
        out.headerCode = "#pragma once\n"
                         "#include <string>\n"
                         "#include <unordered_map>\n"
                         "#include <functional>\n\n"
                         "using ExecutorFn = std::function<bool(const std::string&)>;\n"
                         "inline std::unordered_map<std::string, ExecutorFn> makeDispatchTable() {\n"
                         "    std::unordered_map<std::string, ExecutorFn> table;\n";
        for (const auto& e : entries) {
            out.headerCode += "    table[\"" + e.jobType + "\"] = " + e.executor + ";\n";
            out.headerCode += "    // payload=" + e.payloadType + " caps=" + joinCaps(e.requiredCaps) + "\n";
        }
        out.headerCode += "    return table;\n}\n";
        return out;
    }

private:
    static std::string joinCaps(const std::vector<std::string>& caps) {
        if (caps.empty()) return "none";
        std::string out;
        for (std::size_t i = 0; i < caps.size(); ++i) {
            if (i) out += ",";
            out += caps[i];
        }
        return out;
    }
};
