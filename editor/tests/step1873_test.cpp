// Step 1873 TDD: GR-005 — Prose paragraph capture in RequirementsParser
//
// Both parsers currently skip non-bullet lines in requirement sections.
// A spec with prose (non-bullet) requirements silently drops those requirements.
//
//  t1: prose lines in ## Requirements section are captured
//  t2: ## Features bullet list only → success and features captured
//  t3: ## Goals bullets + ## Requirements prose → both captured (end-to-end)
//  t4: very short prose lines (< 10 chars) are not captured
//  t5: code fence marker lines (starting with `) are not captured

#include "RequirementsParser.h"
#include "ArchitectIntakeProcessor.h"
#include <iostream>
#include <string>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(prose_lines_in_requirements_captured);
    std::string md =
        "## Requirements\n"
        "The system must authenticate users before allowing access.\n"
        "The API must be RESTful and support pagination.\n";
    auto items = RequirementsParser::parse(md);
    C(items.size() >= 2,
      "expected at least 2 prose requirements, got " + std::to_string(items.size()));
    bool foundAuth = false, foundRest = false;
    for (const auto& r : items) {
        if (r.sourceText.find("authenticate") != std::string::npos) foundAuth = true;
        if (r.sourceText.find("RESTful") != std::string::npos) foundRest = true;
    }
    C(foundAuth, "auth prose requirement not captured");
    C(foundRest, "RESTful prose requirement not captured");
    P();
}

void t2(){
    T(features_only_bullet_list_succeeds);
    std::string md =
        "## Features\n"
        "- REST API endpoints\n"
        "- rate limiting support\n";
    auto result = ArchitectIntakeProcessor::process(md);
    C(result.success, "expected success for features-only spec");
    C(result.requirementCount >= 2,
      "expected at least 2 features, got " + std::to_string(result.requirementCount));
    P();
}

void t3(){
    T(goals_bullets_plus_requirements_prose_both_captured);
    std::string md =
        "## Goals\n"
        "- Provide secure access\n"
        "- Support mobile clients\n"
        "## Requirements\n"
        "The system must implement OAuth2 for authentication.\n";
    auto result = ArchitectIntakeProcessor::process(md);
    C(result.success, "expected success");
    // 2 goals via normalized path + 1 prose req via parsedRequirements
    C(result.requirementCount >= 3,
      "expected at least 3 (2 goals + 1 prose req), got " + std::to_string(result.requirementCount));
    P();
}

void t4(){
    T(short_prose_lines_not_captured);
    std::string md =
        "## Requirements\n"
        "OK\n"
        "Yes\n"
        "- A valid bullet requirement to ensure the section works\n";
    auto items = RequirementsParser::parse(md);
    bool foundShort = false;
    for (const auto& r : items) {
        if (r.sourceText == "OK" || r.sourceText == "Yes") foundShort = true;
    }
    C(!foundShort, "short prose lines should not be captured as requirements");
    P();
}

void t5(){
    T(code_fence_marker_not_captured);
    std::string md =
        "## Requirements\n"
        "The system must support JSON output.\n"
        "```json\n"
        "{ \"key\": \"value\" }\n"
        "```\n";
    auto items = RequirementsParser::parse(md);
    bool foundFence = false;
    for (const auto& r : items) {
        if (r.sourceText.find("```") != std::string::npos) foundFence = true;
    }
    C(!foundFence, "code fence marker should not be captured as a requirement");
    P();
}

int main(){
    std::cout<<"Step 1873: GR-005 prose paragraph capture in RequirementsParser\n";
    t1();t2();t3();t4();t5();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
