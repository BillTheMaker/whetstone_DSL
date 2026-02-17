#pragma once

// Step 488: Security-Preserving Translation
// Ensures security annotations are preserved or escalated for human review.

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

struct SecuritySourceAnnotation {
    std::string key;   // e.g. @InputValidation
    std::string value; // e.g. sanitized/raw/jwt/aes256
};

struct SecurityTranslationInput {
    std::string sourceLanguage;
    std::string targetLanguage;
    std::vector<SecuritySourceAnnotation> annotations;
};

struct SecurityMappedAnnotation {
    std::string sourceKey;
    std::string sourceValue;
    std::string mappedKey;
    std::string mappedValue;
    bool preserved = true;
    bool reviewRequired = false;
    std::string reviewReason;
};

struct SecurityTranslationOutput {
    std::vector<SecurityMappedAnnotation> mappings;
    bool hasBlockingReview = false;
    std::vector<std::string> notes;

    int preservedCount() const {
        int n = 0;
        for (const auto& m : mappings) if (m.preserved) ++n;
        return n;
    }
};

class SecurityPreservingTranslation {
public:
    static SecurityTranslationOutput translate(const SecurityTranslationInput& in) {
        SecurityTranslationOutput out;
        const std::string target = lower(in.targetLanguage);

        if (in.annotations.empty()) {
            out.notes.push_back("No security annotations found in source");
            return out;
        }

        for (const auto& ann : in.annotations) {
            auto mapped = mapAnnotation(ann, target);
            if (mapped.reviewRequired) out.hasBlockingReview = true;
            out.mappings.push_back(std::move(mapped));
        }

        out.notes.push_back("Security annotations preserved through translation");
        if (out.hasBlockingReview) {
            out.notes.push_back("One or more annotations require human review");
        }
        return out;
    }

private:
    static std::string lower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    static bool isInsecureCrypto(const std::string& value) {
        std::string v = lower(value);
        return v.find("md5") != std::string::npos || v.find("sha1") != std::string::npos;
    }

    static SecurityMappedAnnotation mapAnnotation(const SecuritySourceAnnotation& ann,
                                                  const std::string& targetLanguage) {
        SecurityMappedAnnotation out;
        out.sourceKey = ann.key;
        out.sourceValue = ann.value;
        out.mappedKey = ann.key;
        out.mappedValue = ann.value;

        if (ann.key == "@InputValidation") {
            if (targetLanguage == "rust") {
                out.mappedValue = "whetstone::input_validation(\"" + ann.value + "\")";
            } else if (targetLanguage == "python") {
                out.mappedValue = "@input_validation(level=\"" + ann.value + "\")";
            } else if (targetLanguage == "typescript" || targetLanguage == "javascript") {
                out.mappedValue = "validateInput(\"" + ann.value + "\")";
            } else {
                out.reviewRequired = true;
                out.reviewReason = "No deterministic @InputValidation mapping for target language";
            }
            return out;
        }

        if (ann.key == "@TrustBoundary") {
            out.mappedValue = "boundary:" + ann.value;
            return out;
        }

        if (ann.key == "@Auth") {
            if (targetLanguage == "rust") {
                out.mappedValue = "AuthGuard::" + ann.value;
            } else if (targetLanguage == "python") {
                out.mappedValue = "auth_required(method=\"" + ann.value + "\")";
            } else if (targetLanguage == "typescript" || targetLanguage == "javascript") {
                out.mappedValue = "@UseAuth(\"" + ann.value + "\")";
            } else {
                out.reviewRequired = true;
                out.reviewReason = "No deterministic @Auth mapping for target language";
            }
            return out;
        }

        if (ann.key == "@Encryption") {
            if (isInsecureCrypto(ann.value)) {
                out.reviewRequired = true;
                out.reviewReason = "Insecure crypto algorithm requires human review";
            }
            if (targetLanguage == "rust") {
                out.mappedValue = "ring::" + ann.value;
            } else if (targetLanguage == "python") {
                out.mappedValue = "cryptography." + ann.value;
            } else if (targetLanguage == "typescript" || targetLanguage == "javascript") {
                out.mappedValue = "crypto." + ann.value;
            } else {
                out.reviewRequired = true;
                if (out.reviewReason.empty()) {
                    out.reviewReason = "No deterministic @Encryption mapping for target language";
                }
            }
            return out;
        }

        if (ann.key == "@Secrets") {
            out.mappedValue = "secret_manager(policy=\"" + ann.value + "\")";
            out.reviewRequired = true;
            out.reviewReason = "Secrets handling always requires human review";
            return out;
        }

        if (ann.key == "@CORS") {
            if (targetLanguage == "python") {
                out.mappedValue = "configure_cors(\"" + ann.value + "\")";
            } else if (targetLanguage == "typescript" || targetLanguage == "javascript") {
                out.mappedValue = "cors({ policy: \"" + ann.value + "\" })";
            } else {
                out.reviewRequired = true;
                out.reviewReason = "No deterministic @CORS mapping for target language";
            }
            return out;
        }

        // Unknown security annotation: preserve by passthrough, require review.
        out.reviewRequired = true;
        out.reviewReason = "Unknown security annotation mapping; preserved for human review";
        return out;
    }
};
