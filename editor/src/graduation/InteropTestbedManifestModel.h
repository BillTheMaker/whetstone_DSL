#pragma once
// Step 1149: Interop testbed manifest model.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct InteropTestbedManifestModel {
    std::string manifestId;
    std::string migrationClass;
    std::vector<std::string> languages;
    int testCaseCount = 0;
    bool requiresReferenceImpl = false;
    bool valid = false;
};

class InteropTestbedManifestModelFactory {
public:
    static InteropTestbedManifestModel make(const std::string& manifestId,
                                            const std::string& migrationClass,
                                            const std::vector<std::string>& languages,
                                            int testCaseCount,
                                            bool requiresReferenceImpl) {
        bool valid = !manifestId.empty() && !migrationClass.empty() &&
                     languages.size() >= 2 && testCaseCount > 0;
        return {manifestId, migrationClass, languages, testCaseCount, requiresReferenceImpl, valid};
    }

    static nlohmann::json toJson(const InteropTestbedManifestModel& m) {
        return {{"manifest_id", m.manifestId},
                {"migration_class", m.migrationClass},
                {"languages", m.languages},
                {"test_case_count", m.testCaseCount},
                {"requires_reference_impl", m.requiresReferenceImpl},
                {"valid", m.valid}};
    }
};
