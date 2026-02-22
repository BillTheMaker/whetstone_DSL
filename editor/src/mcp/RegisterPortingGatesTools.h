// Sprint 50 porting gates MCP tool (Step 735)
// Included inside MCPServer class body.

    void registerPortingGatesTools() {
        tools_.push_back({"whetstone_run_porting_gates",
            "Run security/sanitizer/supply-chain/performance gates for a ported target.",
            {{"type", "object"}, {"properties", {
                {"security_findings", {{"type", "array"}}},
                {"sanitizer", {{"type", "object"}}},
                {"dependencies", {{"type", "array"}}},
                {"benchmarks", {{"type", "array"}}},
                {"thresholds", {{"type", "object"}}},
                {"waivers", {{"type", "object"}}}
            }}}
        });
        toolHandlers_["whetstone_run_porting_gates"] =
            [this](const nlohmann::json& args) { return runPortingGates(args); };
    }

    nlohmann::json runPortingGates(const nlohmann::json& args) {
        std::vector<SecurityFinding> findings;
        for (const auto& f : args.value("security_findings", nlohmann::json::array())) {
            findings.push_back({f.value("id", ""), f.value("severity", "low"), f.value("file", "")});
        }
        auto sec = SecurityScanOrchestrator::run(findings);

        SanitizerGateProfile sanitizerProfile;
        auto sanArgs = args.value("sanitizer", nlohmann::json::object());
        sanitizerProfile.requireAsan = sanArgs.value("require_asan", true);
        sanitizerProfile.requireUbsan = sanArgs.value("require_ubsan", true);
        auto san = SanitizerGateIntegration::evaluate(sanitizerProfile,
                                                      sanArgs.value("asan_clean", false),
                                                      sanArgs.value("ubsan_clean", false));

        std::vector<DependencyAudit> deps;
        for (const auto& d : args.value("dependencies", nlohmann::json::array())) {
            deps.push_back({d.value("name", ""), d.value("high_vulns", 0)});
        }
        auto supply = SupplyChainAuditPacketModel::build(deps);

        std::vector<PerfBenchmarkCase> benches;
        for (const auto& b : args.value("benchmarks", nlohmann::json::array())) {
            benches.push_back({b.value("name", ""), b.value("baseline_ms", 0.0), b.value("target_ms", 0.0)});
        }
        benches = PerfBenchmarkContract::normalize(std::move(benches));

        MigrationGateThresholds th;
        auto thArgs = args.value("thresholds", nlohmann::json::object());
        th.maxHighSeverityFindings = thArgs.value("max_high_severity_findings", 0);
        th.maxPerfRegressionPct = thArgs.value("max_perf_regression_pct", 10.0f);

        auto perf = PerfComparator::compare(benches, th.maxPerfRegressionPct);

        auto waivers = args.value("waivers", nlohmann::json::object());
        PortingGateEvidence ev;
        ev.securityHighFindings = sec.highCount;
        ev.sanitizerPass = san.pass;
        ev.supplyChainHighFindings = supply.totalHigh;
        ev.worstPerfRegressionPct = perf.worstRegressionPct;
        ev.hasSecurityWaiver = waivers.value("security", false);
        ev.hasPerfWaiver = waivers.value("performance", false);

        auto gate = MigrationAcceptanceContract::evaluatePortingGates(ev, th);

        return {
            {"success", true},
            {"security", SecurityScanOrchestrator::toJson(sec)},
            {"sanitizer", SanitizerGateIntegration::toJson(san)},
            {"supply_chain", SupplyChainAuditPacketModel::toJson(supply)},
            {"performance", PerfComparator::toJson(perf)},
            {"gate_result", {
                {"pass", gate.pass},
                {"failed", gate.failedGates},
                {"warnings", gate.warningGates},
                {"states", gate.gateStates},
                {"summary", gate.summary}
            }},
            {"severity", {
                {"security", GateSeverityPolicy::severityFor("security")},
                {"sanitizer", GateSeverityPolicy::severityFor("sanitizer")},
                {"supply_chain", GateSeverityPolicy::severityFor("supply_chain")},
                {"performance", GateSeverityPolicy::severityFor("performance")}
            }}
        };
    }
