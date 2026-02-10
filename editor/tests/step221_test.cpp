// Step 221: Basic evaluation task suite.

#include <cassert>
#include <filesystem>
#include <string>

int main() {
    std::filesystem::path dir = "../eval/basic";
    int count = 0;
    if (std::filesystem::exists(dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                count++;
            }
        }
    }
    assert(count == 30);
    printf("step221_test: all assertions passed\n");
    return 0;
}
