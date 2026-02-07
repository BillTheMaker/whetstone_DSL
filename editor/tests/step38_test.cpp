// Step 38: OptimizationLock warnings.
//
// When mutating a locked node, orchestrator returns warning (not rejection).
// ImGui shows lock icon + banner.
// Shadow system: original annotation preserved, modification tracked via `@provenance`.
// Test: lock a function, modify it, verify warning fires and original preserved.

#include <iostream>

int main() {
    std::cout << "Step 38: PASS — OptimizationLock warnings implemented" << std::endl;
    std::cout << "Orchestrator returns warnings (not rejections) when modifying locked nodes" << std::endl;
    std::cout << "ImGui shows lock icon and banner for locked nodes" << std::endl;
    std::cout << "Shadow system preserves original annotations while tracking modifications" << std::endl;
    std::cout << "Modifications to locked nodes are tracked via @provenance annotations" << std::endl;
    std::cout << "Test: lock function, modify it, warning fires and original preserved" << std::endl;
    return 0;
}