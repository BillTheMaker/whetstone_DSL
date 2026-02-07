cmake_minimum_required(VERSION 3.20)
project(WhetstoneEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# nlohmann/json (header-only, fetched once)
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(json)

add_executable(step1_test tests/step1_test.cpp)
target_include_directories(step1_test PRIVATE src)

add_executable(step2_test tests/step2_test.cpp)
target_include_directories(step2_test PRIVATE src)

add_executable(step3_test tests/step3_test.cpp)
target_include_directories(step3_test PRIVATE src)

add_executable(step4_test tests/step4_test.cpp)
target_include_directories(step4_test PRIVATE src)

add_executable(step5_test tests/step5_test.cpp)
target_include_directories(step5_test PRIVATE src)
target_link_libraries(step5_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step6_test tests/step6_test.cpp)
target_include_directories(step6_test PRIVATE src)
target_link_libraries(step6_test PRIVATE nlohmann_json::nlohmann_json)

add_executable(step7_test tests/step7_test.cpp)
target_include_directories(step7_test PRIVATE src)

add_executable(step8_test tests/step8_test.cpp)
target_include_directories(step8_test PRIVATE src)

add_executable(step9_test tests/step9_test.cpp)
target_include_directories(step9_test PRIVATE src)

add_executable(step10_test tests/step10_test.cpp)
target_include_directories(step10_test PRIVATE src)

add_executable(step11_test tests/step11_test.cpp)
target_include_directories(step11_test PRIVATE src)

add_executable(step12_test tests/step12_test.cpp)
target_include_directories(step12_test PRIVATE src)

add_executable(step13_test tests/step13_test.cpp)
target_include_directories(step13_test PRIVATE src)

add_executable(step14_test tests/step14_test.cpp)
target_include_directories(step14_test PRIVATE src)

add_executable(step15_test tests/step15_test.cpp)
target_include_directories(step15_test PRIVATE src)

add_executable(step16_test tests/step16_test.cpp)
target_include_directories(step16_test PRIVATE src)

add_executable(step17_test tests/step17_test.cpp)
target_include_directories(step17_test PRIVATE src)

add_executable(step18_test tests/step18_test.cpp)
target_include_directories(step18_test PRIVATE src)

add_executable(step19_test tests/step19_test.cpp)
target_include_directories(step19_test PRIVATE src)

add_executable(step20_test tests/step20_test.cpp)
target_include_directories(step20_test PRIVATE src)

add_executable(step21_test tests/step21_test.cpp)
target_include_directories(step21_test PRIVATE src)

add_executable(step22_test tests/step22_test.cpp)
target_include_directories(step22_test PRIVATE src)

add_executable(step23_test tests/step23_test.cpp)
target_include_directories(step23_test PRIVATE src)

add_executable(step24_test tests/step24_test.cpp)
target_include_directories(step24_test PRIVATE src)

add_executable(step25_test tests/step25_test.cpp)
target_include_directories(step25_test PRIVATE src)

add_executable(step26_test tests/step26_test.cpp)
target_include_directories(step26_test PRIVATE src)

add_executable(step27_test tests/step27_test.cpp)
target_include_directories(step27_test PRIVATE src)

add_executable(step28_test tests/step28_test.cpp)
target_include_directories(step28_test PRIVATE src)

add_executable(step29_test tests/step29_test.cpp)
target_include_directories(step29_test PRIVATE src)

add_executable(step30_test tests/step30_test.cpp)
target_include_directories(step30_test PRIVATE src)

add_executable(step31_test tests/step31_test.cpp)
target_include_directories(step31_test PRIVATE src)

add_executable(step32_test tests/step32_test.cpp)
target_include_directories(step32_test PRIVATE src)

add_executable(step33_test tests/step33_test.cpp)
target_include_directories(step33_test PRIVATE src)

add_executable(step34_test tests/step34_test.cpp)
target_include_directories(step34_test PRIVATE src)

add_executable(step35_test tests/step35_test.cpp)
target_include_directories(step35_test PRIVATE src)

add_executable(step36_test tests/step36_test.cpp)
target_include_directories(step36_test PRIVATE src)

add_executable(step37_test tests/step37_test.cpp)
target_include_directories(step37_test PRIVATE src)

add_executable(whetstone_editor src/main.cpp)
target_include_directories(whetstone_editor PRIVATE src)
# find_package(SDL2 REQUIRED)  # Commented out for now to avoid build issues
# find_package(OpenGL REQUIRED)
# find_package(glad REQUIRED)
target_link_libraries(whetstone_editor PRIVATE nlohmann_json::nlohmann_json)  # Only link JSON library for now

add_executable(orchestrator src/orchestrator_main.cpp)
target_include_directories(orchestrator PRIVATE src)
target_link_libraries(orchestrator PRIVATE nlohmann_json::nlohmann_json)

# Step 12: Dear ImGui shell scaffolding created (main.cpp exists but not built due to dependencies)
