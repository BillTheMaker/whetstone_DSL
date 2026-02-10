// Step 222: Workflow evaluation task suite.

#include <cassert>
#include <filesystem>

int main() {
    std::filesystem::path dir = "../eval/workflows";
    int count = 0;
    if (std::filesystem::exists(dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                count++;
            }
        }
    }
    assert(count == 20);
    printf("step222_test: all assertions passed\n");
    return 0;
}
