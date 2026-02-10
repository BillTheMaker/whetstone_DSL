#pragma once
// TreeSitterParser TypeScript support.
public:
    //  TypeScript
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseTypeScript(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_typescript());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_ts_module";
        module->targetLanguage = "typescript";
        applySpan(module.get(), root);

        convertJavaScriptModule(root, source, module.get(), "typescript");

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseTypeScriptWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_typescript());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_ts_module";
        result.module->targetLanguage = "typescript";
        applySpan(result.module.get(), root);

        convertJavaScriptModule(root, source, result.module.get(), "typescript");
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
private:
