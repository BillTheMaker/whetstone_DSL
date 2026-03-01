// Step 1875 TDD: GR-015 — CrossArtifactConsistencyEngine checks intent-vs-goal alignment
//
// CrossArtifactConsistencyChecker was a data stub (enabled=false, no logic).
// CrossArtifactConsistencyEngine is the actual evaluation: checks that each
// spec goal's meaningful keywords appear in at least one taskitem intent.
//
//  t1: all goals covered by intents → consistent=true, inconsistencyCount=0
//  t2: one goal not covered by any intent → inconsistencyCount=1, driftedGoals has it
//  t3: empty goals list → consistent=true (nothing to check)
//  t4: goal with only stop words / short tokens → not flagged (no meaningful tokens)
//  t5: goal covered by partial/substring match in intent → covered=true

#include "graduation/CrossArtifactConsistencyEngine.h"
#include <iostream>
#include <string>

static int p=0,f=0;
#define T(n) { std::cout<<"  "<<#n<<"... "; }
#define P() { std::cout<<"PASS\n"; ++p; }
#define F(m) { std::cout<<"FAIL: "<<m<<"\n"; ++f; }
#define C(c,m) if(!(c)){F(m);return;}

void t1(){
    T(all_goals_covered_consistent);
    std::vector<std::string> goals = {
        "secure user authentication",
        "support pagination for API responses"
    };
    std::vector<std::string> intents = {
        "implement OAuth2 authentication flow",
        "add pagination to list endpoints",
        "configure rate limiting"
    };
    auto result = CrossArtifactConsistencyEngine::check(goals, intents);
    C(result.consistent, "expected consistent=true when all goals are covered");
    C(result.inconsistencyCount == 0,
      "expected inconsistencyCount=0, got " + std::to_string(result.inconsistencyCount));
    P();
}

void t2(){
    T(uncovered_goal_flagged_as_drift);
    std::vector<std::string> goals = {
        "secure user authentication",
        "export data to CSV format"  // nothing in intents covers this
    };
    std::vector<std::string> intents = {
        "implement OAuth2 authentication flow",
        "configure rate limiting"
    };
    auto result = CrossArtifactConsistencyEngine::check(goals, intents);
    C(!result.consistent, "expected consistent=false when a goal is uncovered");
    C(result.inconsistencyCount == 1,
      "expected inconsistencyCount=1, got " + std::to_string(result.inconsistencyCount));
    C(!result.driftedGoals.empty(), "expected driftedGoals to be non-empty");
    bool foundCsv = false;
    for (const auto& g : result.driftedGoals) {
        if (g.find("CSV") != std::string::npos || g.find("export") != std::string::npos) foundCsv = true;
    }
    C(foundCsv, "expected CSV export goal in driftedGoals");
    P();
}

void t3(){
    T(empty_goals_consistent);
    std::vector<std::string> goals;
    std::vector<std::string> intents = {"implement something"};
    auto result = CrossArtifactConsistencyEngine::check(goals, intents);
    C(result.consistent, "empty goals should be consistent");
    C(result.inconsistencyCount == 0, "inconsistencyCount should be 0 for empty goals");
    P();
}

void t4(){
    T(stop_word_only_goal_not_flagged);
    std::vector<std::string> goals = {
        "the and must with for"  // all stop words — no meaningful tokens
    };
    std::vector<std::string> intents = {"completely unrelated task"};
    auto result = CrossArtifactConsistencyEngine::check(goals, intents);
    // Goal has no meaningful tokens (all stop words or < 4 chars) → should not be flagged
    C(result.consistent, "stop-word-only goal should not be flagged as inconsistent");
    P();
}

void t5(){
    T(substring_match_covers_goal);
    std::vector<std::string> goals = {
        "implement authentication"
    };
    std::vector<std::string> intents = {
        "OAuth2 authentication setup and session management"
        // "authentication" token from goal is a substring of intent
    };
    auto result = CrossArtifactConsistencyEngine::check(goals, intents);
    C(result.consistent,
      "goal should be covered when key token appears as substring in an intent");
    P();
}

int main(){
    std::cout<<"Step 1875: GR-015 CrossArtifactConsistencyEngine intent-vs-goal alignment\n";
    t1();t2();t3();t4();t5();
    std::cout<<"\nResults: "<<p<<"/"<<(p+f)<<" passed\n";
    return f?1:0;
}
