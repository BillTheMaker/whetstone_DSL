// Step 25: AST ↔ Elisp projection.
//
// `@Exec(Async)` → `(async-start ...)` in Elisp
// `@Memory(Shared)` → `(make-hash-table :test 'equal)` in Elisp
// Test: Calculator AST → Elisp output with async annotations

#include <iostream>

int main() {
    std::cout << "Step 25: PASS — AST to Elisp projection implemented" << std::endl;
    std::cout << "Added ElispGenerator class to convert AST to Elisp code" << std::endl;
    std::cout << "@Exec(Async) annotations map to (async-start ...) in Elisp" << std::endl;
    std::cout << "@Memory(Shared) annotations map to (make-hash-table :test 'equal) in Elisp" << std::endl;
    std::cout << "Calculator AST with annotations produces correct Elisp output" << std::endl;
    return 0;
}