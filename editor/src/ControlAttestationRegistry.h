#pragma once
// Step 602: Control Attestation Registry

#include <map>
#include <string>
#include <vector>

#include "ValidationErrorUtil.h"

enum class AttestationStatus {
    Draft,
    Signed,
    Expired
};

struct ControlAttestation {
    std::string attestationId;
    std::string controlId;
    std::string owner;
    int daysUntilExpiry = 0;
    AttestationStatus status = AttestationStatus::Draft;
    std::string signer;
};

class ControlAttestationRegistry {
public:
    bool submit(const ControlAttestation& attestation, std::string* error) {
        if (!error) return false;
        error->clear();
        if (attestation.attestationId.empty()) return failWith(error, "attestation_id_missing");
        if (attestation.controlId.empty()) return failWith(error, "control_id_missing");
        if (attestation.owner.empty()) return failWith(error, "owner_missing");
        if (attestation.daysUntilExpiry < 0) return failWith(error, "expiry_days_invalid");
        if (attestation.status == AttestationStatus::Expired) return failWith(error, "status_invalid");
        if (items_.count(attestation.attestationId) != 0) return failWith(error, "attestation_duplicate");
        items_[attestation.attestationId] = attestation;
        order_.push_back(attestation.attestationId);
        return true;
    }

    bool sign(const std::string& attestationId,
              const std::string& signer,
              std::string* error) {
        if (!error) return false;
        error->clear();
        auto it = items_.find(attestationId);
        if (it == items_.end()) return failWith(error, "attestation_missing");
        if (signer.empty()) return failWith(error, "signer_missing");
        if (it->second.status == AttestationStatus::Expired) return failWith(error, "attestation_expired");
        it->second.status = AttestationStatus::Signed;
        it->second.signer = signer;
        return true;
    }

    bool expire(const std::string& attestationId, std::string* error) {
        if (!error) return false;
        error->clear();
        auto it = items_.find(attestationId);
        if (it == items_.end()) return failWith(error, "attestation_missing");
        it->second.status = AttestationStatus::Expired;
        return true;
    }

    std::vector<ControlAttestation> byStatus(AttestationStatus status) const {
        std::vector<ControlAttestation> out;
        for (const auto& id : order_) {
            const auto& attestation = items_.at(id);
            if (attestation.status == status) out.push_back(attestation);
        }
        return out;
    }

    int expiringWithin(int days) const {
        int count = 0;
        for (const auto& id : order_) {
            const auto& attestation = items_.at(id);
            if (attestation.status != AttestationStatus::Expired &&
                attestation.daysUntilExpiry <= days) {
                ++count;
            }
        }
        return count;
    }

private:
    std::map<std::string, ControlAttestation> items_;
    std::vector<std::string> order_;
};
