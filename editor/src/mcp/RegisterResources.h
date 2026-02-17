    void registerWhetstoneResources() {
        resources_.push_back({"whetstone://ast", "Current AST",
            "The current Abstract Syntax Tree as JSON", "application/json"});
        resources_.push_back({"whetstone://diagnostics", "Diagnostics",
            "Current diagnostics (errors, warnings) from LSP and Whetstone", "application/json"});
        resources_.push_back({"whetstone://libraries", "Imported Libraries",
            "Currently imported libraries with available symbols", "application/json"});
        resources_.push_back({"whetstone://annotations", "Annotations",
            "All memory annotations in the current module", "application/json"});
        resources_.push_back({"whetstone://settings", "Editor Settings",
            "Current editor settings (read-only)", "application/json"});
    }

    // ---------------------------------------------------------------
    //  Step 211: Register prompts
    // ---------------------------------------------------------------
