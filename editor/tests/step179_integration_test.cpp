// Step 179: EditorMode auto-close toggle integration checks.

#include <cassert>

#include "EditorMode.h"

int main() {
    EditorMode elisp("elisp");
    EditorMode cpp("cpp");
    assert(elisp.autoCloseBrackets() == false);
    assert(cpp.autoCloseBrackets() == true);

    printf("step179_integration_test: all assertions passed\n");
    return 0;
}
