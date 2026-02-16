// Step 349: Icon System + Visual Indicators (12 tests)
// Tests icon constants, file/node/severity/workflow icons, fallbacks, completeness

#include <cassert>
#include <iostream>
#include <string>
#include <set>
#include "Icons.h"
#include "ThemeData.h"

int main() {
    int passed = 0;

    // Test 1: All file type icons are non-empty
    {
        assert(std::string(WhetstoneIcons::FileSource).size() > 0);
        assert(std::string(WhetstoneIcons::FileHeader).size() > 0);
        assert(std::string(WhetstoneIcons::FileConfig).size() > 0);
        assert(std::string(WhetstoneIcons::FileImage).size() > 0);
        assert(std::string(WhetstoneIcons::FileGeneric).size() > 0);
        assert(std::string(WhetstoneIcons::Folder).size() > 0);
        assert(std::string(WhetstoneIcons::FolderOpen).size() > 0);
        std::cout << "Test 1 PASSED: All file type icons non-empty\n";
        passed++;
    }

    // Test 2: Diagnostic severity icons distinct
    {
        std::set<std::string> icons;
        icons.insert(WhetstoneIcons::DiagError);
        icons.insert(WhetstoneIcons::DiagWarning);
        icons.insert(WhetstoneIcons::DiagInfo);
        icons.insert(WhetstoneIcons::DiagHint);
        assert(icons.size() == 4); // all different
        std::cout << "Test 2 PASSED: 4 distinct diagnostic icons\n";
        passed++;
    }

    // Test 3: Workflow status icons cover all states
    {
        auto pending = WhetstoneIcons::iconForWorkflowStatus("pending");
        auto progress = WhetstoneIcons::iconForWorkflowStatus("in-progress");
        auto complete = WhetstoneIcons::iconForWorkflowStatus("complete");
        auto rejected = WhetstoneIcons::iconForWorkflowStatus("rejected");
        auto review = WhetstoneIcons::iconForWorkflowStatus("review");
        assert(std::string(pending).size() > 0);
        assert(std::string(progress).size() > 0);
        assert(std::string(complete).size() > 0);
        assert(std::string(rejected).size() > 0);
        assert(std::string(review).size() > 0);
        std::cout << "Test 3 PASSED: Workflow status icons cover all 5 states\n";
        passed++;
    }

    // Test 4: AST node type icons
    {
        assert(std::string(WhetstoneIcons::iconForNodeType("Function")) == WhetstoneIcons::NodeFunction);
        assert(std::string(WhetstoneIcons::iconForNodeType("ClassDeclaration")) == WhetstoneIcons::NodeClass);
        assert(std::string(WhetstoneIcons::iconForNodeType("Variable")) == WhetstoneIcons::NodeVariable);
        assert(std::string(WhetstoneIcons::iconForNodeType("Module")) == WhetstoneIcons::NodeModule);
        std::cout << "Test 4 PASSED: AST node type icons\n";
        passed++;
    }

    // Test 5: Severity icon mapping
    {
        assert(std::string(WhetstoneIcons::iconForSeverity("error")) == WhetstoneIcons::DiagError);
        assert(std::string(WhetstoneIcons::iconForSeverity("warning")) == WhetstoneIcons::DiagWarning);
        assert(std::string(WhetstoneIcons::iconForSeverity("info")) == WhetstoneIcons::DiagInfo);
        assert(std::string(WhetstoneIcons::iconForSeverity("hint")) == WhetstoneIcons::DiagHint);
        std::cout << "Test 5 PASSED: Severity icon mapping\n";
        passed++;
    }

    // Test 6: Fallback text representations
    {
        assert(WhetstoneIcons::fallback(WhetstoneIcons::DiagError) == "[ERR]");
        assert(WhetstoneIcons::fallback(WhetstoneIcons::DiagWarning) == "[WARN]");
        assert(WhetstoneIcons::fallback(WhetstoneIcons::DiagInfo) == "[INFO]");
        assert(WhetstoneIcons::fallback(WhetstoneIcons::Complete) == "[OK]");
        assert(WhetstoneIcons::fallback(WhetstoneIcons::Rejected) == "[FAIL]");
        std::cout << "Test 6 PASSED: Fallback text representations\n";
        passed++;
    }

    // Test 7: Navigation icons defined
    {
        assert(std::string(WhetstoneIcons::ArrowRight).size() > 0);
        assert(std::string(WhetstoneIcons::ArrowDown).size() > 0);
        assert(std::string(WhetstoneIcons::Search).size() > 0);
        assert(std::string(WhetstoneIcons::Settings).size() > 0);
        std::cout << "Test 7 PASSED: Navigation icons defined\n";
        passed++;
    }

    // Test 8: Unknown node type returns generic icon
    {
        auto icon = WhetstoneIcons::iconForNodeType("UnknownNodeType");
        assert(std::string(icon).size() > 0); // should still return something
        std::cout << "Test 8 PASSED: Unknown node type returns generic icon\n";
        passed++;
    }

    // Test 9: Unknown severity returns generic icon
    {
        auto icon = WhetstoneIcons::iconForSeverity("unknown");
        assert(std::string(icon).size() > 0);
        std::cout << "Test 9 PASSED: Unknown severity returns generic icon\n";
        passed++;
    }

    // Test 10: Unknown workflow status returns generic icon
    {
        auto icon = WhetstoneIcons::iconForWorkflowStatus("unknown");
        assert(std::string(icon).size() > 0);
        std::cout << "Test 10 PASSED: Unknown workflow status returns generic\n";
        passed++;
    }

    // Test 11: Icon count — at least 20 distinct icon constants
    {
        std::set<std::string> all;
        all.insert(WhetstoneIcons::FileSource);
        all.insert(WhetstoneIcons::FileHeader);
        all.insert(WhetstoneIcons::FileConfig);
        all.insert(WhetstoneIcons::FileImage);
        all.insert(WhetstoneIcons::FileGeneric);
        all.insert(WhetstoneIcons::Folder);
        all.insert(WhetstoneIcons::FolderOpen);
        all.insert(WhetstoneIcons::DiagError);
        all.insert(WhetstoneIcons::DiagWarning);
        all.insert(WhetstoneIcons::DiagInfo);
        all.insert(WhetstoneIcons::DiagHint);
        all.insert(WhetstoneIcons::Pending);
        all.insert(WhetstoneIcons::InProgress);
        all.insert(WhetstoneIcons::Complete);
        all.insert(WhetstoneIcons::Rejected);
        all.insert(WhetstoneIcons::Review);
        all.insert(WhetstoneIcons::NodeFunction);
        all.insert(WhetstoneIcons::NodeClass);
        all.insert(WhetstoneIcons::NodeVariable);
        all.insert(WhetstoneIcons::NodeModule);
        all.insert(WhetstoneIcons::ArrowRight);
        all.insert(WhetstoneIcons::ArrowDown);
        all.insert(WhetstoneIcons::Search);
        all.insert(WhetstoneIcons::Settings);
        assert(all.size() >= 20);
        std::cout << "Test 11 PASSED: " << all.size() << " distinct icon constants\n";
        passed++;
    }

    // Test 12: Diagnostic icon colors match theme severity colors
    {
        auto theme = getDefaultDarkTheme();
        // Verify diagnostic colors are semantic: error=red, warning=amber, info=blue
        assert(theme.diagError.r > 0.5f);    // red-ish
        assert(theme.diagWarning.r > 0.5f && theme.diagWarning.g > 0.3f); // amber
        assert(theme.diagInfo.b > 0.5f);      // blue-ish
        assert(theme.diagHint.b > 0.5f || theme.diagHint.r > 0.5f); // purple
        std::cout << "Test 12 PASSED: Diagnostic colors match severity semantics\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
