// Step 359: Find and Replace Improvements (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "panels/FindReplaceBar.h"

int main() {
    int passed = 0;
    const std::string text = "alpha beta\nbeta gamma\nbeta\n";

    // Test 1: Ctrl+F opens find bar
    {
        FindReplaceBar bar;
        bar.openFind();
        assert(bar.isVisible());
        assert(!bar.isReplaceMode());
        std::cout << "Test 1 PASSED: open find bar\n";
        passed++;
    }

    // Test 2: typing filters matches
    {
        FindReplaceBar bar;
        bar.openFind();
        bar.setQuery("beta", text);
        assert(bar.matchCount() == 3);
        std::cout << "Test 2 PASSED: typing filters matches\n";
        passed++;
    }

    // Test 3: match highlighting visible
    {
        FindReplaceBar bar;
        bar.openFind();
        bar.setQuery("beta", text);
        assert(bar.hasHighlights());
        std::cout << "Test 3 PASSED: highlights visible\n";
        passed++;
    }

    // Test 4: match count accurate
    {
        FindReplaceBar bar;
        bar.openFind();
        bar.setQuery("beta", text);
        assert(bar.matchCounterText() == "1 of 3");
        std::cout << "Test 4 PASSED: match count text\n";
        passed++;
    }

    // Test 5: Enter cycles through matches (next)
    {
        FindReplaceBar bar;
        bar.openFind();
        bar.setQuery("beta", text);
        int start = bar.currentIndex();
        bar.nextMatch();
        assert(bar.currentIndex() != start);
        std::cout << "Test 5 PASSED: next match cycle\n";
        passed++;
    }

    // Test 6: Ctrl+H shows replace field
    {
        FindReplaceBar bar;
        bar.openReplace();
        assert(bar.isVisible());
        assert(bar.isReplaceMode());
        std::cout << "Test 6 PASSED: replace mode open\n";
        passed++;
    }

    // Test 7: replace works
    {
        FindReplaceBar bar;
        std::string mutableText = text;
        bar.openReplace();
        bar.setQuery("gamma", mutableText);
        bar.setReplaceText("delta");
        bool ok = bar.replaceCurrent(mutableText);
        assert(ok);
        assert(mutableText.find("delta") != std::string::npos);
        std::cout << "Test 7 PASSED: replace current\n";
        passed++;
    }

    // Test 8: project-wide search returns results
    {
        FindReplaceBar bar;
        bar.openFind();
        bar.setQuery("beta", text);
        std::vector<std::pair<std::string, std::string>> files = {
            {"a.txt", text},
            {"b.txt", "zzz\nbeta\n"}
        };
        auto results = bar.searchProject(files);
        assert(results.size() == 4);
        std::cout << "Test 8 PASSED: project search results\n";
        passed++;
    }

    // Test 9: Escape closes find bar
    {
        FindReplaceBar bar;
        bar.openFind();
        bar.close();
        assert(!bar.isVisible());
        std::cout << "Test 9 PASSED: close find bar\n";
        passed++;
    }

    // Test 10: empty search clears highlights
    {
        FindReplaceBar bar;
        bar.openFind();
        bar.setQuery("beta", text);
        assert(bar.hasHighlights());
        bar.setQuery("", text);
        assert(!bar.hasHighlights());
        assert(bar.matchCount() == 0);
        std::cout << "Test 10 PASSED: empty query clears highlights\n";
        passed++;
    }

    // Test 11: Shift+Enter cycles previous match
    {
        FindReplaceBar bar;
        bar.openFind();
        bar.setQuery("beta", text);
        bar.nextMatch(); // index 1
        int idx = bar.currentIndex();
        bar.prevMatch();
        assert(bar.currentIndex() != idx);
        std::cout << "Test 11 PASSED: previous match cycle\n";
        passed++;
    }

    // Test 12: replace-all updates count and clears matches when fully replaced
    {
        FindReplaceBar bar;
        std::string mutableText = text;
        bar.openReplace();
        bar.setQuery("beta", mutableText);
        bar.setReplaceText("B");
        int replaced = bar.replaceAll(mutableText);
        assert(replaced == 3);
        assert(mutableText.find("beta") == std::string::npos);
        std::cout << "Test 12 PASSED: replace all\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
