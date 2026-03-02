// Step 1901: MCP wiring for whetstone_generate_ffi_glue
//
// Registers:
//   whetstone_generate_ffi_glue
//
// This file is #included inside the MCPServer class body.

    void registerFFIGlueTools() {
        tools_.push_back({"whetstone_generate_ffi_glue",
            "Generate FFI glue code for all cross-language boundaries in a polyglot project. "
            "Extracts boundary nodes from the AST, dispatches to the correct binding emitter "
            "(RustPython, GoCpp, RustGo, CppJS) based on language pair, emits a C ABI header, "
            "and produces DWARF annotations for the DAP orchestrator.",
            {{"type", "object"}, {"properties", {
                {"spec", {{"type", "object"},
                    {"description", "PolyglotProjectSpec as JSON: {projectName, sections:[{componentName,assignedLanguage}], interfaces:[{fromComponent,toComponent}]}"}}},
                {"ast", {{"type", "object"},
                    {"description", "Component AST map: {components: {compName: {nodes:[{name,kind,exported,...}]}}}"}}},
                {"project_name", {{"type", "string"},
                    {"description", "Project name used for the C header guard (default: project)"}}}
            }}, {"required", json::array({"spec", "ast"})}}
        });
        toolHandlers_["whetstone_generate_ffi_glue"] =
            [this](const json& args) -> json {
                return runGenerateFFIGlue(args);
            };
    }

    json runGenerateFFIGlue(const json& args) {
        if (!args.contains("spec") || !args.contains("ast"))
            return {{"success", false}, {"error", "spec and ast are required"}};

        // Reconstruct PolyglotProjectSpec from JSON
        whetstone::PolyglotProjectSpec spec;
        const auto& specJ = args["spec"];
        if (specJ.contains("projectName") && specJ["projectName"].is_string())
            spec.projectName = specJ["projectName"].get<std::string>();

        if (specJ.contains("sections") && specJ["sections"].is_array()) {
            for (const auto& s : specJ["sections"]) {
                whetstone::PolyglotSection sec;
                if (s.contains("componentName")) sec.componentName = s["componentName"].get<std::string>();
                if (s.contains("assignedLanguage")) sec.assignedLanguage = s["assignedLanguage"].get<std::string>();
                if (s.contains("explicitLanguage")) sec.explicitLanguage = s["explicitLanguage"].get<std::string>();
                spec.sections.push_back(sec);
            }
        }

        if (specJ.contains("interfaces") && specJ["interfaces"].is_array()) {
            for (const auto& i : specJ["interfaces"]) {
                whetstone::PolyglotInterface iface;
                if (i.contains("fromComponent")) iface.fromComponent = i["fromComponent"].get<std::string>();
                if (i.contains("toComponent"))   iface.toComponent   = i["toComponent"].get<std::string>();
                if (i.contains("description"))   iface.description   = i["description"].get<std::string>();
                spec.interfaces.push_back(iface);
            }
        }

        const auto& ast = args["ast"];
        std::string projectName = "project";
        if (args.contains("project_name") && args["project_name"].is_string())
            projectName = args["project_name"].get<std::string>();

        // Extract boundary nodes
        auto nodes = whetstone::ABIBoundaryExtractor::extract(spec, ast);

        // Generate all bindings
        auto result = whetstone::FFIGlueDispatcher::generateAll(nodes, projectName);

        return {{"success", true},
                {"node_count", (int)nodes.size()},
                {"bindings", result["bindings"]},
                {"c_header", result["c_header"]},
                {"dwarf_annotations", result["dwarf_annotations"]}};
    }
