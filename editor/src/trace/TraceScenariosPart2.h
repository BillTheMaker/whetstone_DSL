    // --- Scenario: Annotate All Subjects ---
    Trace generateAnnotateAllSubjects(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "annotate_all_subjects";
        trace.difficulty = "advanced";
        trace.language = params.language;
        const auto& sample = pickSample(params.language);
        trace.steps.push_back({"user",
            "Add annotations from all 8 subjects (memory, type system, concurrency, "
            "scope, shims, optimization, meta-programming, policy) to this code.", "", {}, {}});
        trace.steps.push_back({"assistant",
            "I'll analyze the code and apply annotations across all subject areas.", "", {}, {}});
        json astResult = simulateGetAST(sample.source, params.language);
        trace.steps.push_back({"tool_call", "", "whetstone_get_ast", json::object(), {}});
        trace.steps.push_back({"tool_result", "", "whetstone_get_ast", {}, astResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_get_ast");
        trace.steps.push_back({"assistant",
            "Applied annotations: @Reclaim(Tracing), @BitWidth(64), @Exec(sync), "
            "@Visibility(public), @TailCall, @Policy(strict). All 8 subjects covered.", "", {}, {}});
        return trace;
    }

    // --- Scenario: Validate and Fix ---
    Trace generateValidateAndFix(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "validate_and_fix";
        trace.difficulty = "intermediate";
        trace.language = params.language;
        trace.steps.push_back({"user",
            "Validate all annotations and fix any conflicts or errors.", "", {}, {}});
        trace.steps.push_back({"assistant",
            "I'll run validation and conflict detection, then fix issues.", "", {}, {}});
        json suggestResult = {
            {"scopeId", "mod1"},
            {"suggestions", json::array()},
            {"diagnostics", json::array({{
                {"severity", "error"},
                {"message", "E0700: @Pure conflicts with @Blocking"},
                {"nodeId", "fn1"}
            }})}
        };
        trace.steps.push_back({"tool_call", "", "whetstone_suggest_annotations", json::object(), {}});
        trace.steps.push_back({"tool_result", "", "whetstone_suggest_annotations", {}, suggestResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_suggest_annotations");
        trace.steps.push_back({"assistant",
            "Found conflict E0700: @Pure vs @Blocking. Removed @Blocking to resolve.", "", {}, {}});
        return trace;
    }

    // --- Scenario: Semanno Export ---
    Trace generateSemannoExport(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "semanno_export";
        trace.difficulty = "basic";
        trace.language = params.language;
        trace.steps.push_back({"user",
            "Export the annotated code with Semanno inline comments.", "", {}, {}});
        trace.steps.push_back({"assistant",
            "I'll generate code with @semanno inline comments for all annotations.", "", {}, {}});
        const auto& sample = pickSample(params.language);
        json pipelineResult = {
            {"success", true},
            {"generatedCode", "// @semanno:intent(summary=\"math helpers\")\n" + sample.source},
            {"parseDiagnostics", json::array()},
            {"validationDiagnostics", json::array()},
            {"violations", json::array()},
            {"suggestions", json::array()},
            {"foldCount", 0}, {"dceCount", 0}
        };
        trace.steps.push_back({"tool_call", "", "whetstone_run_pipeline",
            {{"source", sample.source}, {"sourceLanguage", params.language},
             {"targetLanguage", params.language}}, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_run_pipeline", {}, pipelineResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_run_pipeline");
        trace.steps.push_back({"assistant",
            "Exported code with Semanno comments. All annotations are now "
            "embedded as parseable inline comments.", "", {}, {}});
        return trace;
    }

    // --- Scenario: Cross-Language Annotated ---
    Trace generateCrossLanguageAnnotated(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "cross_language_annotated";
        trace.difficulty = "advanced";
        trace.language = params.language;
        std::string target = (params.language == "python") ? "kotlin" :
                             (params.language == "kotlin") ? "csharp" : "python";
        trace.steps.push_back({"user",
            "Project this annotated " + params.language + " code to " + target +
            ", preserving all semantic annotations.", "", {}, {}});
        trace.steps.push_back({"assistant",
            "I'll project to " + target + " while ensuring annotations are preserved.", "", {}, {}});
        json pipelineResult = {
            {"success", true},
            {"generatedCode", "// Generated annotated " + target + " code"},
            {"parseDiagnostics", json::array()},
            {"validationDiagnostics", json::array()},
            {"violations", json::array()},
            {"suggestions", json::array()},
            {"foldCount", 0}, {"dceCount", 0}
        };
        trace.steps.push_back({"tool_call", "", "whetstone_run_pipeline",
            {{"source", pickSample(params.language).source},
             {"sourceLanguage", params.language}, {"targetLanguage", target}}, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_run_pipeline", {}, pipelineResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_run_pipeline");
        trace.steps.push_back({"assistant",
            "Successfully projected to " + target + " with all annotations preserved. "
            "Semanno comments are language-appropriate.", "", {}, {}});
        return trace;
    }

    // Simulate getAST result for a code sample
    json simulateGetAST(const std::string& source, const std::string& language) {
        // Use real parser if possible, fall back to mock
        Pipeline pipeline;
        std::vector<ParseDiagnostic> diags;
        auto mod = pipeline.parse(source, language, diags);
        if (mod) {
            return {
                {"ast", toJson(mod.get())},
                {"annotationCount", 0},
                {"diagnostics", json::array()}
            };
        }
        // Mock fallback
        return {
            {"ast", {{"conceptType", "Module"}, {"id", "mod1"}, {"name", "parsed"}}},
            {"annotationCount", 0},
            {"diagnostics", json::array()}
        };
    }
};
