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
