// Step 664-665: MCP wiring for codegen tools
//
// Registers two code generation tools:
//   whetstone_schema_to_cpp       (Step 634 — SchemaToCppGenerator)
//   whetstone_generate_dispatch_table (Step 642 — JobDispatchTableGenerator)
//
// This file is #included inside the MCPServer class body.
// Pattern follows RegisterArchitectIntakeTools.h (Sprint 36).

#include "../SchemaToCppGenerator.h"
#include "../JobDispatchTableGenerator.h"

    void registerCodegenTools() {
        // ---------------------------------------------------------------
        //  whetstone_schema_to_cpp
        // ---------------------------------------------------------------
        tools_.push_back({"whetstone_schema_to_cpp",
            "Generate a typed C++ header struct with nlohmann JSON serializers "
            "from a JSON Schema object. Returns header_code (ready to write to a "
            ".h file) and a cmake_interface_target snippet.",
            {{"type", "object"}, {"properties", {
                {"schema",      {{"type", "object"},
                    {"description", "JSON Schema with 'title' and 'properties'."}}},
                {"header_name", {{"type", "string"},
                    {"description", "Output header filename, e.g. 'EnergyContext.h'."}}},
                {"target_name", {{"type", "string"},
                    {"description", "CMake INTERFACE target name, e.g. 'energy_context_types'."}}}
            }}, {"required", json::array({"schema", "header_name", "target_name"})}}
        });
        toolHandlers_["whetstone_schema_to_cpp"] =
            [this](const json& args) {
                return runSchemaToCpp(args);
            };

        // ---------------------------------------------------------------
        //  whetstone_generate_dispatch_table
        // ---------------------------------------------------------------
        tools_.push_back({"whetstone_generate_dispatch_table",
            "Generate a C++ dispatch table header (makeDispatchTable()) from a "
            "list of job type specs. Each entry maps a job_type string to a C++ "
            "executor callable.",
            {{"type", "object"}, {"properties", {
                {"entries", {{"type", "array"},
                    {"description", "Array of job dispatch entries."},
                    {"items", {{"type", "object"}, {"properties", {
                        {"job_type",      {{"type", "string"}}},
                        {"required_caps", {{"type", "array"}, {"items", {{"type", "string"}}}}}  ,
                        {"payload_type",  {{"type", "string"}}},
                        {"executor",      {{"type", "string"},
                            {"description", "C++ callable expression, e.g. 'handleCudaTask'"}}}
                    }}, {"required", json::array({"job_type", "executor"})}}}}}
            }}, {"required", json::array({"entries"})}}
        });
        toolHandlers_["whetstone_generate_dispatch_table"] =
            [this](const json& args) {
                return runGenerateDispatchTable(args);
            };
    }

    json runSchemaToCpp(const json& args) {
        if (!args.contains("schema") || !args["schema"].is_object()) {
            return {{"success", false}, {"errors", json::array({"schema_missing_or_invalid"})}};
        }
        if (!args.contains("header_name") || !args["header_name"].is_string()) {
            return {{"success", false}, {"errors", json::array({"header_name_missing"})}};
        }
        if (!args.contains("target_name") || !args["target_name"].is_string()) {
            return {{"success", false}, {"errors", json::array({"target_name_missing"})}};
        }

        auto out = SchemaToCppGenerator::generate(
            args["schema"],
            args["header_name"].get<std::string>(),
            args["target_name"].get<std::string>()
        );

        return {
            {"success",                out.success},
            {"header_code",            out.headerCode},
            {"cmake_interface_target", out.cmakeInterfaceTarget},
            {"errors",                 out.errors}
        };
    }

    json runGenerateDispatchTable(const json& args) {
        if (!args.contains("entries") || !args["entries"].is_array()) {
            return {{"success", false}, {"errors", json::array({"entries_missing_or_invalid"})}};
        }

        std::vector<JobDispatchEntrySpec> entries;
        for (const auto& e : args["entries"]) {
            if (!e.contains("job_type") || !e.contains("executor")) {
                return {{"success", false}, {"errors", json::array({"entry_missing_job_type_or_executor"})}};
            }
            JobDispatchEntrySpec spec;
            spec.jobType     = e["job_type"].get<std::string>();
            spec.executor    = e["executor"].get<std::string>();
            spec.payloadType = e.value("payload_type", "");
            if (e.contains("required_caps") && e["required_caps"].is_array()) {
                for (const auto& cap : e["required_caps"]) {
                    if (cap.is_string()) spec.requiredCaps.push_back(cap.get<std::string>());
                }
            }
            entries.push_back(spec);
        }

        auto out = JobDispatchTableGenerator::generate(entries);

        return {
            {"success",     out.success},
            {"header_code", out.headerCode},
            {"errors",      out.errors}
        };
    }
