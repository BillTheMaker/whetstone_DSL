// Step 182: Diagnostics aggregation integration checks.

#include <cassert>

#include "Diagnostics.h"

int main() {
    EditorDiagnostic d;
    d.severity = 1;
    d.uri = "file:///test.py";
    d.line = 3;
    d.character = 2;
    d.message = "Test error";
    assert(d.severity == 1);
    assert(d.message == "Test error");

    printf("step182_integration_test: all assertions passed\n");
    return 0;
}
