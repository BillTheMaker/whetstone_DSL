#pragma once
// Step 821: Waiver policy packet (temporary/permanent/scoped).

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class WaiverScope { Temporary, Permanent, Scoped };

struct WaiverPolicyPacket {
    std::string waiverId;
    std::string issueRef;
    WaiverScope scope = WaiverScope::Temporary;
    std::string rationale;
    std::string grantedBy;
    std::string expiresAt;  // ISO date string, empty = permanent
    std::string scopePath;  // for Scoped waivers
    bool active = true;
};

class WaiverPolicyPacketModel {
public:
    static WaiverPolicyPacket make(const std::string& id,
                                   const std::string& issueRef,
                                   WaiverScope scope,
                                   const std::string& rationale,
                                   const std::string& grantedBy) {
        WaiverPolicyPacket p;
        p.waiverId = id;
        p.issueRef = issueRef;
        p.scope = scope;
        p.rationale = rationale;
        p.grantedBy = grantedBy;
        p.active = true;
        return p;
    }

    static bool validate(const WaiverPolicyPacket& p, std::string* error) {
        if (!error) return false;
        error->clear();
        if (p.waiverId.empty()) { *error = "waiver_id_missing"; return false; }
        if (p.issueRef.empty()) { *error = "issue_ref_missing"; return false; }
        if (p.rationale.empty()) { *error = "rationale_missing"; return false; }
        if (p.grantedBy.empty()) { *error = "granted_by_missing"; return false; }
        if (p.scope == WaiverScope::Scoped && p.scopePath.empty()) {
            *error = "scope_path_missing"; return false;
        }
        if (p.scope == WaiverScope::Temporary && p.expiresAt.empty()) {
            *error = "expires_at_missing"; return false;
        }
        return true;
    }

    static bool revoke(WaiverPolicyPacket& p, std::string* error) {
        if (!error) return false;
        error->clear();
        if (!p.active) { *error = "already_inactive"; return false; }
        p.active = false;
        return true;
    }

    static std::string scopeStr(WaiverScope s) {
        switch (s) {
            case WaiverScope::Temporary: return "temporary";
            case WaiverScope::Permanent: return "permanent";
            case WaiverScope::Scoped:    return "scoped";
        }
        return "unknown";
    }

    static nlohmann::json toJson(const WaiverPolicyPacket& p) {
        return {{"waiver_id", p.waiverId}, {"issue_ref", p.issueRef},
                {"scope", scopeStr(p.scope)}, {"rationale", p.rationale},
                {"granted_by", p.grantedBy}, {"expires_at", p.expiresAt},
                {"scope_path", p.scopePath}, {"active", p.active}};
    }
};
