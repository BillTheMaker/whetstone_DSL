// Step 183: Buffer rename integration checks.

#include <cassert>

#include "BufferManager.h"

int main() {
    BufferManager mgr;
    mgr.openBuffer("(untitled)", "", "python");
    bool ok = mgr.renameBuffer("(untitled)", "(untitled-renamed)");
    assert(ok);
    assert(mgr.hasBuffer("(untitled-renamed)"));

    printf("step183_integration_test: all assertions passed\n");
    return 0;
}
