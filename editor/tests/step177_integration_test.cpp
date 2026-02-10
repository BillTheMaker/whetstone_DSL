// Step 177: Search utils integration checks.

#include <cassert>
#include <string>
#include <vector>

#include "SearchUtils.h"

int main() {
    const std::string text = "foo bar foo\nFoo bar\nfoobar\n";
    SearchOptions opts;
    opts.matchCase = false;
    opts.wholeWord = true;
    opts.useRegex = false;

    auto matches = SearchUtils::collectMatches(text, "foo", opts, 0, (int)text.size());
    assert(matches.size() == 3);

    std::vector<std::string> groups;
    opts.useRegex = true;
    bool ok = SearchUtils::regexGroupsForFirstMatch("foobar", "(foo)(bar)", opts, 0, 6, groups);
    assert(ok);
    assert(groups.size() == 2);
    assert(groups[0] == "foo");
    assert(groups[1] == "bar");

    int count = 0;
    opts.useRegex = false;
    std::string replaced = SearchUtils::replaceAll(text, "bar", "baz", opts, 0, (int)text.size(), count);
    assert(count == 2);
    assert(replaced.find("baz") != std::string::npos);

    printf("step177_integration_test: all assertions passed\n");
    return 0;
}
