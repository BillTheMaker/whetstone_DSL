// -----------------------------------------------------------------------
//  Collectors: convert each diagnostic source to unified format
// -----------------------------------------------------------------------

inline std::vector<StructuredDiagnostic> collectParseDiagnostics(
        const std::vector<ParseDiagnostic>& diags) {
    std::vector<StructuredDiagnostic> out;
    out.reserve(diags.size());
    for (const auto& d : diags) {
        StructuredDiagnostic sd;
        sd.code     = parseErrorCode(d.message);
        sd.severity = severityFromStr(d.severity);
        sd.line     = d.line;
        sd.col      = d.column;
        sd.message  = d.message;
        sd.source   = "parser";
        out.push_back(std::move(sd));
    }
    return out;
}

inline std::vector<StructuredDiagnostic> collectAnnotationDiagnostics(
        const std::vector<AnnotationValidator::Diagnostic>& diags) {
    std::vector<StructuredDiagnostic> out;
    out.reserve(diags.size());
    for (const auto& d : diags) {
        StructuredDiagnostic sd;
        sd.code     = annotationErrorCode(d.message);
        sd.severity = severityFromStr(d.severity);
        sd.nodeId   = d.nodeId;
        sd.message  = d.message;
        sd.source   = "annotation";
        sd.fix      = buildAnnotationFix(d.nodeId, d.message);
        out.push_back(std::move(sd));
    }
    return out;
}

inline std::vector<StructuredDiagnostic> collectStrategyDiagnostics(
        const std::vector<StrategyValidator::Violation>& violations) {
    std::vector<StructuredDiagnostic> out;
    out.reserve(violations.size());
    for (const auto& v : violations) {
        StructuredDiagnostic sd;
        sd.code     = strategyErrorCode(v.category);
        sd.severity = severityFromStr(v.severity);
        sd.nodeId   = v.nodeId;
        sd.message  = v.message;
        sd.source   = "strategy";
        sd.fix      = buildStrategyFix(v.nodeId, v.category);
        out.push_back(std::move(sd));
    }
    return out;
}

// -----------------------------------------------------------------------
//  collectAllDiagnostics — run the full validation pipeline on an AST
// -----------------------------------------------------------------------
inline std::vector<StructuredDiagnostic> collectAllDiagnostics(
        Module* ast) {
    std::vector<StructuredDiagnostic> all;
    if (!ast) return all;

    // Annotation validation
    AnnotationValidator annoValidator;
    auto annoDiags = annoValidator.validate(ast);
    auto annoStructured = collectAnnotationDiagnostics(annoDiags);
    all.insert(all.end(), annoStructured.begin(), annoStructured.end());

    // Extended annotation validation (Subjects 2-8)
    AnnotationValidatorExtended extValidator;
    auto extDiags = extValidator.validate(ast);
    auto extStructured = collectAnnotationDiagnostics(extDiags);
    all.insert(all.end(), extStructured.begin(), extStructured.end());

    // Cross-type annotation conflict detection
    std::vector<CrossTypeConflict> crossConflicts;
    collectCrossTypeConflicts(ast, crossConflicts);
    for (const auto& c : crossConflicts) {
        StructuredDiagnostic sd;
        sd.code = "E0210";
        sd.severity = DiagnosticSeverity::Error;
        sd.nodeId = c.nodeId;
        sd.message = c.type1 + " + " + c.type2 + ": " + c.message;
        sd.source = "annotation";
        all.push_back(std::move(sd));
    }

    // Strategy validation (post-optimization invariants)
    StrategyValidator stratValidator;
    auto violations = stratValidator.validateInvariants(ast);
    auto stratStructured = collectStrategyDiagnostics(violations);
    all.insert(all.end(), stratStructured.begin(), stratStructured.end());

    return all;
}

// -----------------------------------------------------------------------
//  collectPipelineDiagnostics — from a PipelineResult
// -----------------------------------------------------------------------
inline std::vector<StructuredDiagnostic> collectPipelineDiagnostics(
        const Pipeline::PipelineResult& pr) {
    std::vector<StructuredDiagnostic> all;

    auto parseDiags = collectParseDiagnostics(pr.parseDiags);
    all.insert(all.end(), parseDiags.begin(), parseDiags.end());

    auto annoDiags = collectAnnotationDiagnostics(pr.validationDiags);
    all.insert(all.end(), annoDiags.begin(), annoDiags.end());

    auto stratDiags = collectStrategyDiagnostics(pr.violations);
    all.insert(all.end(), stratDiags.begin(), stratDiags.end());

    return all;
}

// -----------------------------------------------------------------------
//  diagnosticsToJson — convert array to JSON
// -----------------------------------------------------------------------
inline json diagnosticsToJson(
        const std::vector<StructuredDiagnostic>& diags) {
    json arr = json::array();
    for (const auto& d : diags)
        arr.push_back(diagnosticToJson(d));
    return arr;
}

// -----------------------------------------------------------------------
//  filterBySeverity — keep only diagnostics at or above threshold
// -----------------------------------------------------------------------
inline std::vector<StructuredDiagnostic> filterBySeverity(
        const std::vector<StructuredDiagnostic>& diags,
        DiagnosticSeverity maxSeverity) {
    std::vector<StructuredDiagnostic> out;
    for (const auto& d : diags) {
        if (static_cast<int>(d.severity) <=
            static_cast<int>(maxSeverity)) {
            out.push_back(d);
        }
    }
    return out;
}

// -----------------------------------------------------------------------
//  filterBySource — keep only diagnostics from a specific source
// -----------------------------------------------------------------------
inline std::vector<StructuredDiagnostic> filterBySource(
        const std::vector<StructuredDiagnostic>& diags,
        const std::string& source) {
    std::vector<StructuredDiagnostic> out;
    for (const auto& d : diags) {
        if (d.source == source) out.push_back(d);
    }
    return out;
}

