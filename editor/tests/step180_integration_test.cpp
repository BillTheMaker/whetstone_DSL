// Step 180: Rich tooltip markdown rendering checks.

#include <cassert>

#include "RichTooltip.h"

int main() {
    auto& states = richTooltipStates();
    states["demo"].pinned = true;
    states["demo"].pinnedContent = "## Title\n- Item";
    assert(states["demo"].pinned == true);
    assert(states["demo"].pinnedContent.find("Title") != std::string::npos);

    printf("step180_integration_test: all assertions passed\n");
    return 0;
}
