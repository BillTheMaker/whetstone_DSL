#pragma once
// Sprint 28 refactor: shared adapter operation helpers

#include <algorithm>
#include <string>
#include <vector>

inline bool adapterHasOperation(const std::vector<std::string>& operations,
                                const std::string& operation) {
    return std::find(operations.begin(), operations.end(), operation) != operations.end();
}
