// Step 825: Feedback-to-adapter tuning hooks (8 tests)
#include "governance/FeedbackAdapterHooks.h"
#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if (!(c)) { F(m); return; }

static FeedbackSignal makeSig(const std::string& id, const std::string& target, int w = 5) {
    FeedbackSignal s; s.signalId = id; s.adapterTarget = target;
    s.suggestion = "use smart ptrs"; s.weight = w; s.sourceIssue = "ISS-01";
    return s;
}

void t1() {
    T(add_signal);
    FeedbackAdapterHooks h; std::string err;
    C(h.addSignal(makeSig("S1", "cpp"), &err), "add");
    P();
}
void t2() {
    T(missing_signal_id);
    FeedbackAdapterHooks h; std::string err;
    FeedbackSignal s; s.adapterTarget = "cpp"; s.suggestion = "x";
    C(!h.addSignal(s, &err) && err == "signal_id_missing", "err");
    P();
}
void t3() {
    T(missing_target);
    FeedbackAdapterHooks h; std::string err;
    FeedbackSignal s; s.signalId = "S3"; s.suggestion = "x";
    C(!h.addSignal(s, &err) && err == "adapter_target_missing", "err");
    P();
}
void t4() {
    T(missing_suggestion);
    FeedbackAdapterHooks h; std::string err;
    FeedbackSignal s; s.signalId = "S4"; s.adapterTarget = "cpp";
    C(!h.addSignal(s, &err) && err == "suggestion_missing", "err");
    P();
}
void t5() {
    T(build_hint_for_target);
    FeedbackAdapterHooks h; std::string err;
    h.addSignal(makeSig("S5a", "cpp", 3), &err);
    h.addSignal(makeSig("S5b", "cpp", 7), &err);
    h.addSignal(makeSig("S5c", "rust", 5), &err);
    auto hint = h.buildHint("cpp");
    C(hint.signals.size() == 2, "signals");
    C(hint.aggregateWeight == 10, "weight");
    P();
}
void t6() {
    T(top_suggestion_is_highest_weight);
    FeedbackAdapterHooks h; std::string err;
    auto s1 = makeSig("S6a", "cpp", 2); s1.suggestion = "low_weight";
    auto s2 = makeSig("S6b", "cpp", 9); s2.suggestion = "high_weight";
    h.addSignal(s1, &err); h.addSignal(s2, &err);
    auto hint = h.buildHint("cpp");
    C(hint.topSuggestion == "high_weight", "top");
    P();
}
void t7() {
    T(adapter_targets_list);
    FeedbackAdapterHooks h; std::string err;
    h.addSignal(makeSig("S7a", "cpp"), &err);
    h.addSignal(makeSig("S7b", "rust"), &err);
    h.addSignal(makeSig("S7c", "cpp"), &err);
    auto targets = h.adapterTargets();
    C(targets.size() == 2, "two_targets");
    P();
}
void t8() {
    T(to_json_hint);
    FeedbackAdapterHooks h; std::string err;
    h.addSignal(makeSig("S8", "go", 4), &err);
    auto hint = h.buildHint("go");
    auto j = FeedbackAdapterHooks::toJson(hint);
    C(j.value("adapter_target", "") == "go", "target");
    C(j.value("aggregate_weight", 0) == 4, "weight");
    P();
}

int main() {
    std::cout << "Step 825: Feedback-to-adapter tuning hooks\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << p << "/" << (p+f) << " passed\n";
    return f ? 1 : 0;
}
