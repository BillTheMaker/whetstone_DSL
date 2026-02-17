    void registerWhetstonePrompts() {
        // --- Step 270: Semantic annotation prompts ---
        prompts_.push_back({"annotate_intent",
            "Annotate unannotated functions with intent summaries and categories. "
            "Uses whetstone_get_unannotated_nodes and whetstone_set_semantic_annotation.",
            {}
        });

        prompts_.push_back({"annotate_complexity",
            "Annotate functions with complexity estimates (time complexity, "
            "cognitive complexity, lines of logic).",
            {}
        });

        prompts_.push_back({"annotate_risk",
            "Assess modification risk for each function based on callers, "
            "complexity, and side effects. Uses call hierarchy for dependent counts.",
            {}
        });

        prompts_.push_back({"annotate_contracts",
            "Document preconditions, postconditions, return shapes, and "
            "side effects for each function.",
            {}
        });

        prompts_.push_back({"annotate_full",
            "Run a complete annotation pass: intent, complexity, risk, "
            "and contracts for all unannotated functions. Save the annotated "
            "AST sidecar when done.",
            {}
        });

        // --- Original prompts ---
        prompts_.push_back({"annotate_module",
            "Analyze the current module and suggest memory annotations for all "
            "unannotated functions with confidence scores.",
            {{"scope", "Which functions to annotate (all, unannotated, or a specific function name)", false}}
        });

        prompts_.push_back({"cross_language_projection",
            "Project the current code to a different target language, adapting "
            "annotations appropriately.",
            {{"targetLanguage", "Target language (cpp, rust, go, java, javascript, python, elisp)", true}}
        });

        prompts_.push_back({"security_audit",
            "Check all dependencies for known vulnerabilities and suggest "
            "safe alternatives or upgrades.",
            {}
        });

        prompts_.push_back({"refactor_memory",
            "Analyze memory strategy annotations and suggest improvements "
            "for safety and performance.",
            {}
        });
    }

    std::vector<MCPPromptMessage> generatePromptMessages(const std::string& name,
                                                          const json& args) {
        // --- Step 270: Semantic annotation prompt messages ---
        if (name == "annotate_intent") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "For each unannotated function, add an intent annotation:\n"
                    "1. Call whetstone_get_unannotated_nodes to find targets\n"
                    "2. Use whetstone_get_ast (compact:true) to understand each function\n"
                    "3. Call whetstone_set_semantic_annotation with type='intent', providing:\n"
                    "   - summary: 1-sentence description of what and why\n"
                    "   - category: one of 'validation', 'transformation', 'io',\n"
                    "     'coordination', 'computation', 'initialization'\n"
                    "4. Verify with whetstone_get_semantic_annotations"
                }}
            }};
        }
        if (name == "annotate_complexity") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "For each function, estimate complexity:\n"
                    "1. Call whetstone_get_unannotated_nodes with type='complexity'\n"
                    "2. Use whetstone_get_ast (compact:true) to read each function\n"
                    "3. Call whetstone_set_semantic_annotation with type='complexity':\n"
                    "   - timeComplexity: 'O(1)', 'O(n)', 'O(n^2)', etc.\n"
                    "   - cognitiveComplexity: 1-10 scale\n"
                    "   - linesOfLogic: count of logic statements"
                }}
            }};
        }
        if (name == "annotate_risk") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "For each function, assess modification risk:\n"
                    "1. Call whetstone_get_unannotated_nodes with type='risk'\n"
                    "2. Use whetstone_get_call_hierarchy to count callers/dependents\n"
                    "3. Consider complexity and side effects\n"
                    "4. Call whetstone_set_semantic_annotation with type='risk':\n"
                    "   - level: 'low', 'medium', 'high', 'critical'\n"
                    "   - reason: why this risk level\n"
                    "   - dependentCount: number of callers/consumers"
                }}
            }};
        }
        if (name == "annotate_contracts") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "For each function, document data contracts:\n"
                    "1. Call whetstone_get_unannotated_nodes with type='contract'\n"
                    "2. Use whetstone_get_ast (compact:true) to read each function\n"
                    "3. Call whetstone_set_semantic_annotation with type='contract':\n"
                    "   - preconditions: input requirements\n"
                    "   - postconditions: output guarantees\n"
                    "   - returnShape: human-readable type/shape\n"
                    "   - sideEffects: 'none', 'io', 'mutation', 'network'"
                }}
            }};
        }
        if (name == "annotate_full") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "Run a complete semantic annotation pass:\n"
                    "1. Call whetstone_get_unannotated_nodes to find all targets\n"
                    "2. For each function, add intent, complexity, risk, and contract\n"
                    "   annotations using whetstone_set_semantic_annotation\n"
                    "3. Use whetstone_get_call_hierarchy for caller counts\n"
                    "4. Save the annotated AST with whetstone_save_annotated_ast\n"
                    "5. Verify with whetstone_get_semantic_annotations"
                }}
            }};
        }
        if (name == "annotate_module") {
            std::string scope = args.value("scope", "all");
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "Analyze the current module's AST and suggest memory annotations for " +
                    scope + " functions. For each function:\n"
                    "1. Use whetstone_get_ast to read the current AST\n"
                    "2. Use whetstone_suggest_annotations for each unannotated function\n"
                    "3. Present suggestions with confidence scores\n"
                    "4. Apply approved annotations with whetstone_apply_annotation\n"
                    "5. Verify with whetstone_get_ast that annotations were applied correctly"
                }}
            }};
        }
        if (name == "cross_language_projection") {
            std::string lang = args.value("targetLanguage", "cpp");
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "Project the current code to " + lang + ":\n"
                    "1. Use whetstone_get_ast to read the current AST\n"
                    "2. Use whetstone_project_language with targetLanguage=\"" + lang + "\"\n"
                    "3. Review the generated code and annotation adaptations\n"
                    "4. Report any annotation issues or incompatibilities"
                }}
            }};
        }
        if (name == "security_audit") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "Perform a security audit of the project's dependencies:\n"
                    "1. Use whetstone_get_ast to identify imported libraries\n"
                    "2. Check each dependency for known vulnerabilities\n"
                    "3. Suggest safe alternatives or version upgrades\n"
                    "4. Report severity levels and recommended actions"
                }}
            }};
        }
        if (name == "refactor_memory") {
            return {{
                "user",
                {{"type", "text"}, {"text",
                    "Analyze and improve memory strategy annotations:\n"
                    "1. Use whetstone_get_ast to read the current AST\n"
                    "2. Use whetstone_suggest_annotations for each function\n"
                    "3. Identify annotation conflicts or suboptimal strategies\n"
                    "4. Suggest improvements for safety and performance\n"
                    "5. Apply approved changes with whetstone_apply_annotation"
                }}
            }};
        }
        return {};
    }

    // ---------------------------------------------------------------
    //  Step 247: Register file operation tools
    // ---------------------------------------------------------------
