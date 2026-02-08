// Step 57 TDD Test: Buffer management
//
// Tests that BufferManager:
// 1. Opens buffers and tracks them
// 2. Gets buffer content
// 3. Closes buffers
// 4. Tracks multiple buffers simultaneously
// 5. Switches between buffers
// 6. Closing one buffer leaves others intact
// 7. Buffer info includes language and modified flag

#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include "BufferManager.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Open a buffer ---
    {
        BufferManager mgr;
        bool ok = mgr.openBuffer("/test/foo.py", "def foo(): pass", "python");
        assert(ok && "Opening a buffer should succeed");
        assert(mgr.hasBuffer("/test/foo.py") && "Buffer should exist after opening");

        std::cout << "Test 1 PASS: Buffer opened and tracked" << std::endl;
        ++passed;
    }

    // --- Test 2: Get buffer content ---
    {
        BufferManager mgr;
        mgr.openBuffer("/test/foo.py", "def foo(): pass", "python");

        std::string content = mgr.getBufferContent("/test/foo.py");
        assert(content == "def foo(): pass" && "Buffer content should match what was opened");

        std::cout << "Test 2 PASS: Buffer content retrievable" << std::endl;
        ++passed;
    }

    // --- Test 3: Close buffer removes it ---
    {
        BufferManager mgr;
        mgr.openBuffer("/test/foo.py", "def foo(): pass", "python");
        assert(mgr.hasBuffer("/test/foo.py") && "Buffer should exist");

        bool closed = mgr.closeBuffer("/test/foo.py");
        assert(closed && "Close should succeed");
        assert(!mgr.hasBuffer("/test/foo.py") && "Buffer should not exist after closing");

        std::cout << "Test 3 PASS: Close buffer removes it" << std::endl;
        ++passed;
    }

    // --- Test 4: Multiple buffers tracked ---
    {
        BufferManager mgr;
        mgr.openBuffer("/test/a.py", "# a", "python");
        mgr.openBuffer("/test/b.cpp", "// b", "cpp");
        mgr.openBuffer("/test/c.el", ";; c", "elisp");

        auto buffers = mgr.getOpenBuffers();
        assert(buffers.size() == 3 && "Should have 3 open buffers");

        assert(mgr.hasBuffer("/test/a.py") && "Should have a.py");
        assert(mgr.hasBuffer("/test/b.cpp") && "Should have b.cpp");
        assert(mgr.hasBuffer("/test/c.el") && "Should have c.el");

        std::cout << "Test 4 PASS: Multiple buffers tracked simultaneously" << std::endl;
        ++passed;
    }

    // --- Test 5: Switch between buffers ---
    {
        BufferManager mgr;
        mgr.openBuffer("/test/a.py", "# a", "python");
        mgr.openBuffer("/test/b.cpp", "// b", "cpp");

        mgr.switchToBuffer("/test/a.py");
        assert(mgr.getActiveBufferPath() == "/test/a.py" && "Active should be a.py");

        mgr.switchToBuffer("/test/b.cpp");
        assert(mgr.getActiveBufferPath() == "/test/b.cpp" && "Active should be b.cpp");

        std::cout << "Test 5 PASS: Switch between buffers works" << std::endl;
        ++passed;
    }

    // --- Test 6: Close one buffer, others remain ---
    {
        BufferManager mgr;
        mgr.openBuffer("/test/a.py", "# a", "python");
        mgr.openBuffer("/test/b.cpp", "// b", "cpp");
        mgr.openBuffer("/test/c.el", ";; c", "elisp");

        mgr.closeBuffer("/test/b.cpp");

        auto buffers = mgr.getOpenBuffers();
        assert(buffers.size() == 2 && "Should have 2 buffers after closing one");
        assert(!mgr.hasBuffer("/test/b.cpp") && "Closed buffer should be gone");
        assert(mgr.hasBuffer("/test/a.py") && "Other buffers should remain");
        assert(mgr.hasBuffer("/test/c.el") && "Other buffers should remain");

        std::cout << "Test 6 PASS: Close one buffer, others remain" << std::endl;
        ++passed;
    }

    // --- Test 7: Buffer info includes language and modified flag ---
    {
        BufferManager mgr;
        mgr.openBuffer("/test/foo.py", "def foo(): pass", "python");

        auto info = mgr.getBufferInfo("/test/foo.py");
        assert(info.path == "/test/foo.py" && "Path should match");
        assert(info.language == "python" && "Language should be python");
        assert(!info.modified && "New buffer should not be modified");

        mgr.setModified("/test/foo.py", true);
        auto info2 = mgr.getBufferInfo("/test/foo.py");
        assert(info2.modified && "Buffer should be marked modified");

        std::cout << "Test 7 PASS: Buffer info has language and modified flag" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 57 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
