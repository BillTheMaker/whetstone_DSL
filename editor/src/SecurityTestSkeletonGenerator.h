#pragma once

// Step 491: Security Testing Skeleton Generation
// Generates security-focused test stubs and routing hints.

#include <string>
#include <vector>

enum class SecurityTestKind {
    InputValidationFuzz,
    AuthBypass,
    SqlInjectionProbe,
    XssPayload,
    AccessControlMatrix
};

struct SecurityTestCaseSpec {
    SecurityTestKind kind = SecurityTestKind::InputValidationFuzz;
    std::string testName;
    std::string intentAnnotation;
    std::string routeTo;   // slm | human
    std::string code;
    bool humanReviewRequired = false;
};

struct SecuritySkeletonRequest {
    std::string language;
    std::string moduleName;
    std::string entityName;
    bool includeComplexPenTest = true;
};

struct SecuritySkeletonOutput {
    std::vector<SecurityTestCaseSpec> tests;
    std::vector<std::string> notes;
};

class SecurityTestSkeletonGenerator {
public:
    static SecuritySkeletonOutput generate(const SecuritySkeletonRequest& req) {
        SecuritySkeletonOutput out;
        const std::string base = req.moduleName.empty() ? "module" : req.moduleName;
        const std::string entity = req.entityName.empty() ? "Resource" : req.entityName;

        out.tests.push_back(makeInputValidation(base, entity));
        out.tests.push_back(makeAuthBypass(base, entity));
        out.tests.push_back(makeSqlProbe(base, entity));
        out.tests.push_back(makeXssProbe(base, entity));
        out.tests.push_back(makeAccessMatrix(base, entity));

        if (!req.includeComplexPenTest) {
            for (auto& t : out.tests) {
                if (t.kind == SecurityTestKind::SqlInjectionProbe ||
                    t.kind == SecurityTestKind::XssPayload ||
                    t.kind == SecurityTestKind::AuthBypass) {
                    t.humanReviewRequired = true;
                    t.routeTo = "human";
                }
            }
            out.notes.push_back("Complex penetration tests forced to human review");
        }

        out.notes.push_back("Security test skeletons generated with @Intent annotations");
        return out;
    }

private:
    static SecurityTestCaseSpec makeInputValidation(const std::string& module,
                                                    const std::string& entity) {
        SecurityTestCaseSpec t;
        t.kind = SecurityTestKind::InputValidationFuzz;
        t.testName = "test_" + module + "_input_validation_fuzz";
        t.intentAnnotation = "@Intent(fuzz boundary validation for " + entity + " inputs)";
        t.routeTo = "slm";
        t.code =
            "void " + t.testName + "() {\n"
            "  // fuzz input boundaries and assert rejection of malformed payloads\n"
            "}\n";
        return t;
    }

    static SecurityTestCaseSpec makeAuthBypass(const std::string& module,
                                               const std::string& entity) {
        SecurityTestCaseSpec t;
        t.kind = SecurityTestKind::AuthBypass;
        t.testName = "test_" + module + "_auth_bypass";
        t.intentAnnotation = "@Intent(check auth bypass vectors for " + entity + ")";
        t.routeTo = "human";
        t.humanReviewRequired = true;
        t.code =
            "void " + t.testName + "() {\n"
            "  // attempt unauthorized access and assert denial\n"
            "}\n";
        return t;
    }

    static SecurityTestCaseSpec makeSqlProbe(const std::string& module,
                                             const std::string& entity) {
        SecurityTestCaseSpec t;
        t.kind = SecurityTestKind::SqlInjectionProbe;
        t.testName = "test_" + module + "_sql_injection_probe";
        t.intentAnnotation = "@Intent(probe SQL injection paths for " + entity + ")";
        t.routeTo = "human";
        t.humanReviewRequired = true;
        t.code =
            "void " + t.testName + "() {\n"
            "  // submit payload: \"' OR 1=1 --\" and assert query safety\n"
            "}\n";
        return t;
    }

    static SecurityTestCaseSpec makeXssProbe(const std::string& module,
                                             const std::string& entity) {
        SecurityTestCaseSpec t;
        t.kind = SecurityTestKind::XssPayload;
        t.testName = "test_" + module + "_xss_payload";
        t.intentAnnotation = "@Intent(probe XSS payload handling for " + entity + ")";
        t.routeTo = "human";
        t.humanReviewRequired = true;
        t.code =
            "void " + t.testName + "() {\n"
            "  // inject <script>alert(1)</script> and assert escaping\n"
            "}\n";
        return t;
    }

    static SecurityTestCaseSpec makeAccessMatrix(const std::string& module,
                                                 const std::string& entity) {
        SecurityTestCaseSpec t;
        t.kind = SecurityTestKind::AccessControlMatrix;
        t.testName = "test_" + module + "_access_control_matrix";
        t.intentAnnotation = "@Intent(validate role/resource access matrix for " + entity + ")";
        t.routeTo = "slm";
        t.code =
            "void " + t.testName + "() {\n"
            "  // verify allowed and denied role combinations\n"
            "}\n";
        return t;
    }
};
