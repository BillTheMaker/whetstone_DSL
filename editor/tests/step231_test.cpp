// Step 231: Session anonymizer.
#include "SessionAnonymizer.h"
#include <iostream>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    SessionAnonymizer anon;
    anon.setSeed(123);
    anon.setLevel(SessionAnonymizer::Level::Medium);

    json input = {
        {"path", "C:/Users/Bob/project/main.py"},
        {"source", "def foo(x):\n    # comment\n    return bar(x)\n"},
        {"apiKey", "SECRET-123"}
    };

    json out1 = anon.anonymize(input);
    json out2 = anon.anonymize(input);

    std::string path = out1.value("path", "");
    std::string src = out1.value("source", "");

    expect(path.find("/project/src/") == 0, "path anonymized", passed, failed);
    expect(path.find(".py") != std::string::npos, "path extension preserved", passed, failed);
    expect(src.find("comment") == std::string::npos, "comments stripped", passed, failed);
    expect(src.find("foo") == std::string::npos, "identifiers anonymized", passed, failed);
    expect(src.find("bar") == std::string::npos, "identifiers anonymized 2", passed, failed);
    expect(out1.value("apiKey", "") == "<redacted>", "secrets redacted", passed, failed);
    expect(out1.dump() == out2.dump(), "deterministic output", passed, failed);

    std::cout << "\n=== Step 231 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
