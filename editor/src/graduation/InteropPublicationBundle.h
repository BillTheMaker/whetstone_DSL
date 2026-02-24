#pragma once
// Step 1157: Interop publication bundle.
#include <string>
#include <nlohmann/json.hpp>

struct InteropPublicationBundle {
    std::string bundleId;
    std::string manifestId;
    std::string referenceHarnessId;
    std::string conformanceReportId;
    bool signedBundle = false;
    std::string status;
};

class InteropPublicationBundleFactory {
public:
    static InteropPublicationBundle make(const std::string& bundleId,
                                         const std::string& manifestId,
                                         const std::string& referenceHarnessId,
                                         const std::string& conformanceReportId,
                                         bool signedBundle) {
        bool complete = !bundleId.empty() && !manifestId.empty() &&
                        !referenceHarnessId.empty() && !conformanceReportId.empty();
        std::string status = complete && signedBundle ? "publishable" : "draft";
        return {bundleId, manifestId, referenceHarnessId, conformanceReportId, signedBundle, status};
    }

    static nlohmann::json toJson(const InteropPublicationBundle& b) {
        return {{"bundle_id", b.bundleId},
                {"manifest_id", b.manifestId},
                {"reference_harness_id", b.referenceHarnessId},
                {"conformance_report_id", b.conformanceReportId},
                {"signed_bundle", b.signedBundle},
                {"status", b.status}};
    }
};
