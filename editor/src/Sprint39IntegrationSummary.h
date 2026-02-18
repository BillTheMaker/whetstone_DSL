#pragma once
// Step 653: Sprint 39 integration + summary

#include "HiveMindAutoUpdateModel.h"
#include "LanguageGeneratorPluginManifest.h"
#include "PackageScriptGenerator.h"
#include "Phase39aIntegration.h"
#include "LocalTelemetryModel.h"

struct Sprint39IntegrationResult {
    bool selfHostingReady = false;
    bool packagingReady = false;
    bool autoUpdateReady = false;
    bool pluginReady = false;
    bool telemetryReady = false;
    bool releaseReady = false;
};

class Sprint39IntegrationSummary {
public:
    static Sprint39IntegrationResult run() {
        Sprint39IntegrationResult out;
        auto phase39a = Phase39aIntegration::run();
        out.selfHostingReady = phase39a.integrationReady;

        auto pkg = PackageScriptGenerator::generate("whetstone", "1.0.0");
        out.packagingReady = pkg.success;

        auto update = HiveMindAutoUpdateModel::execute({"1.0.0", "https://apiary/whetstone.AppImage", "abc"}, "abc");
        out.autoUpdateReady = update.accepted && update.hashVerified && update.restartScheduled;

        PluginManifest manifest{"RustGen", "whetstone-plugin-rust.so", "RustGenerator", {"rust"}};
        out.pluginReady = LanguageGeneratorPluginManifest::isValid(manifest) &&
                          LanguageGeneratorPluginManifest::isPluginFile(manifest.sharedObject);

        TelemetryCounters counters;
        LocalTelemetryModel::recordSessionMinutes(&counters, 10);
        LocalTelemetryModel::recordToolCall(&counters);
        out.telemetryReady = LocalTelemetryModel::weeklyReport(counters).find("session_minutes=10") != std::string::npos;

        out.releaseReady = out.selfHostingReady && out.packagingReady && out.autoUpdateReady &&
                           out.pluginReady && out.telemetryReady;
        return out;
    }
};
